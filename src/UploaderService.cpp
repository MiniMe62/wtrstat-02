#include "UploaderService.h"
#include <WiFi.h>

static String urlEncode(const String& value) {
    String encoded = "";
    for (int i = 0; i < value.length(); i++) {
        char c = value.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            char buf[4];
            sprintf(buf, "%%%02X", c);
            encoded += buf;
        }
    }
    return encoded;
}

UploaderService::UploaderService() {
}

bool UploaderService::sendSnapshot(const WeatherSnapshot& snap, const TimeManager& timeMgr) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Uploader] WiFi nie je pripojené! Odosielanie zrušené.");
        return false;
    }

    Serial.println("==========================================");
    Serial.printf("[Uploader] Odosielam 15-minútovú záverku [%s]\n", timeMgr.getFormattedLocal(snap.timestamp).c_str());
    Serial.printf("  - TempIn:  %.2f °C\n", snap.tempIn);
    Serial.printf("  - TempOut: %.2f °C\n", snap.tempOut);
    Serial.printf("  - Vetor:   %.2f m/s (Max: %.2f m/s)\n", snap.windSpeedAvg, snap.windSpeedMax);
    Serial.printf("  - Smer:    %.1f° (%s)\n", snap.windDirDeg, snap.windDirName.c_str());
    Serial.println("==========================================");

    if (Config::DRY_RUN_UPLOAD) {
        Serial.println("[DRY RUN UPLOAD] Odosielanie je v REŽIME SIMULÁCIE (DRY_RUN_UPLOAD = true).");
        Serial.println("[DRY RUN UPLOAD] Ostrý zápis do Google Sheets a ThingSpeak bol vynechaný pre ochranu dát.");
        return true;
    }

    bool gsOk = sendToGoogleSheets(snap, timeMgr);
    bool tsOk = sendToThingSpeak(snap, timeMgr);
    bool aioOk = sendToAdafruitIO(snap, timeMgr);

    return (gsOk && tsOk && aioOk);
}

bool UploaderService::sendToGoogleSheets(const WeatherSnapshot& snap, const TimeManager& timeMgr) {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    // Formátovanie s garanciou dvoch desatinných miest (napr. 22.00, 22.25)
    char bufIn[16], bufOut[16], bufWind[16], bufWindMax[16], bufDeg[16];
    snprintf(bufIn, sizeof(bufIn), "%.2f", snap.tempIn);
    snprintf(bufOut, sizeof(bufOut), "%.2f", snap.tempOut);
    snprintf(bufWind, sizeof(bufWind), "%.1f", snap.windSpeedAvg);
    snprintf(bufWindMax, sizeof(bufWindMax), "%.1f", snap.windSpeedMax);
    snprintf(bufDeg, sizeof(bufDeg), "%.0f", snap.windDirDeg);

    StaticJsonDocument<384> doc;
    doc["timestamp"] = timeMgr.getFormattedLocal(snap.timestamp);
    doc["locid"] = Config::LOC_ID;
    doc["tempIn"] = bufIn;
    doc["tempOut"] = bufOut;
    doc["windSpeed"] = bufWind;
    doc["windSpeedMax"] = bufWindMax;
    doc["windDirDeg"] = bufDeg;
    doc["windDirName"] = snap.windDirName;

    String jsonString;
    serializeJson(doc, jsonString);

    Serial.println("[GoogleSheets] Odosielam POST na Google Apps Script (HTTPS)...");
    http.begin(client, Config::GOOGLE_SCRIPT_URL);
    http.addHeader("Content-Type", "application/json");

    const char* headerKeys[] = {"Location"};
    http.collectHeaders(headerKeys, 1);
    http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);

    int httpCode = http.POST(jsonString);

    if (httpCode == 302) {
        String newUrl = http.header("Location");
        http.end();

        if (newUrl.length() > 0) {
            http.begin(client, newUrl);
            http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
            int getCode = http.GET();
            if (getCode > 0) {
                Serial.printf("[GoogleSheets] Úspech GET Redirect (%d): %s\n", getCode, http.getString().c_str());
                http.end();
                return true;
            }
        }
    } else if (httpCode == 200) {
        Serial.printf("[GoogleSheets] Úspech POST Direct: %s\n", http.getString().c_str());
        http.end();
        return true;
    } else {
        Serial.printf("[GoogleSheets] Chyba HTTP kod: %d\n", httpCode);
    }

    http.end();
    return false;
}

bool UploaderService::sendToThingSpeak(const WeatherSnapshot& snap, const TimeManager& timeMgr) {
    // Štandardný ľahký WiFiClient pre HTTP (bez SSL RSA nárokov na RAM)
    WiFiClient client;
    HTTPClient http;

    String isoTime = timeMgr.getFormattedISO(snap.timestamp);

    // Vytvorenie JSON statusu pre texty a vedľajšie senzory
    char dirCombo[16];
    snprintf(dirCombo, sizeof(dirCombo), "%s/%03d", snap.windDirName.c_str(), (int)snap.windDirDeg);

    StaticJsonDocument<128> statusDoc;
    statusDoc["time"] = timeMgr.getFormattedLocal(snap.timestamp);
    statusDoc["dir"] = dirCombo;
    statusDoc["light"] = snap.light;

    String statusJson;
    serializeJson(statusDoc, statusJson);
    String encodedStatus = urlEncode(statusJson);

    // Nové mapovanie presne 8 polí + status
    String url = String(Config::TS_SERVER) + "/update?api_key=" + String(Config::TS_API_KEY) +
                 "&field1=" + String(snap.tempIn, 2) +
                 "&field2=" + String(snap.tempOut, 2) +
                 "&field3=" + String(snap.humidity, 1) +
                 "&field4=" + String(snap.pressure, 1) +
                 "&field5=" + String(snap.windDirDeg, 0) +
                 "&field6=" + String(snap.windSpeedAvg, 1) +
                 "&field7=" + String(snap.windSpeedMax, 1) +
                 "&field8=" + String(snap.rain, 2) +
                 "&status=" + encodedStatus +
                 "&created_at=" + isoTime;

    Serial.println("[ThingSpeak] Odosielam HTTP GET request...");
    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode == 200) {
        Serial.printf("[ThingSpeak] Úspech (Záznam #%s)\n", http.getString().c_str());
        http.end();
        return true;
    }

    Serial.printf("[ThingSpeak] Chyba HTTP kod: %d\n", httpCode);
    http.end();
    return false;
}

bool UploaderService::sendToAdafruitIO(const WeatherSnapshot& snap, const TimeManager& timeMgr) {
    WiFiClient client;
    Adafruit_MQTT_Client mqtt(&client, Config::AIO_SERVER, Config::AIO_SERVERPORT, Config::AIO_USERNAME, Config::AIO_KEY);
    Adafruit_MQTT_Publish groupPub(&mqtt, Config::AIO_GROUP_TOPIC);

    Serial.println("[AdafruitIO] Pripajam sa k MQTT brokeru...");
    
    int8_t ret;
    uint8_t retries = 3;
    while ((ret = mqtt.connect()) != 0) {
        Serial.printf("[AdafruitIO] MQTT Chyba: %s\n", mqtt.connectErrorString(ret));
        Serial.println("[AdafruitIO] Opakujem pokus o 5 sekund...");
        mqtt.disconnect();
        delay(5000); 
        retries--;
        if (retries == 0) return false;
    }
    Serial.println("[AdafruitIO] MQTT Pripojene!");

    StaticJsonDocument<384> doc;
    // Tieto kľúče musia presne zodpovedať názvom feedov v Adafruit IO skupine (meteo)
    doc["tempin"] = round(snap.tempIn * 100.0) / 100.0;
    doc["tempout"] = round(snap.tempOut * 100.0) / 100.0;
    doc["humidity"] = round(snap.humidity * 10.0) / 10.0;
    doc["pressure"] = round(snap.pressure * 10.0) / 10.0;
    doc["wind-speed-avg"] = round(snap.windSpeedAvg * 10.0) / 10.0;
    doc["wind-speed-max"] = round(snap.windSpeedMax * 10.0) / 10.0;
    doc["wind-direction"] = round(snap.windDirDeg);
    doc["light"] = snap.light;
    doc["rain"] = snap.rain;

    String jsonString;
    serializeJson(doc, jsonString);

    Serial.printf("[AdafruitIO] Odosielam JSON: %s\n", jsonString.c_str());
    if (!groupPub.publish(jsonString.c_str())) {
        Serial.println("[AdafruitIO] Publikovanie zlyhalo.");
        mqtt.disconnect();
        return false;
    }
    
    Serial.println("[AdafruitIO] Publikovanie uspesne.");
    mqtt.disconnect();
    return true;
}

