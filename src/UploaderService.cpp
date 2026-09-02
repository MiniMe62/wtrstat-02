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

bool UploaderService::send15MinSnapshot(const WeatherSnapshot& snap, const TimeManager& timeMgr) {
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
    Serial.printf("  - Zrážky:  %.2f mm (Dnes: %.2f mm, Intenzita: %.2f mm/h)\n", snap.rain, snap.rainDaily, snap.rainRate);
    Serial.println("==========================================");

    if (Config::DRY_RUN_UPLOAD) {
        Serial.println("[DRY RUN UPLOAD] Odosielanie je v REŽIME SIMULÁCIE (DRY_RUN_UPLOAD = true).");
        Serial.println("[DRY RUN UPLOAD] Ostrý zápis do Google Sheets a ThingSpeak bol vynechaný pre ochranu dát.");
        return true;
    }

    bool gsOk = true;
    if (Config::ENABLE_GOOGLE_SHEETS_UPLOAD) {
        gsOk = sendToGoogleSheets(snap, timeMgr);
    } else {
        Serial.println("[GoogleSheets] Odosielanie je vypnuté v konfigurácii.");
    }

    bool tsOk = true;
    if (Config::ENABLE_THINGSPEAK_UPLOAD) {
        tsOk = sendToThingSpeak(snap, timeMgr);
    } else {
        Serial.println("[ThingSpeak] Odosielanie je vypnuté v konfigurácii (ochrana limitov).");
    }

    return (gsOk && tsOk);
}

bool UploaderService::send1MinSnapshot(const WeatherSnapshot& snap, const TimeManager& timeMgr) {
    if (WiFi.status() != WL_CONNECTED) return false;
    
    if (Config::DRY_RUN_UPLOAD) return true;

    if (Config::ENABLE_ADAFRUIT_IO_UPLOAD) {
        return sendToAdafruitIO(snap, timeMgr);
    }
    return true;
}

bool UploaderService::sendToGoogleSheets(const WeatherSnapshot& snap, const TimeManager& timeMgr) {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    // Formátovanie s garanciou dvoch desatinných miest (napr. 22.00, 22.25)
    char bufIn[16], bufOut[16], bufWind[16], bufWindMax[16], bufDeg[16], bufRain[16], bufRainDaily[16];
    snprintf(bufIn, sizeof(bufIn), "%.2f", snap.tempIn);
    snprintf(bufOut, sizeof(bufOut), "%.2f", snap.tempOut);
    snprintf(bufWind, sizeof(bufWind), "%.1f", snap.windSpeedAvg);
    snprintf(bufWindMax, sizeof(bufWindMax), "%.1f", snap.windSpeedMax);
    snprintf(bufDeg, sizeof(bufDeg), "%.0f", snap.windDirDeg);
    snprintf(bufRain, sizeof(bufRain), "%.2f", snap.rain);
    snprintf(bufRainDaily, sizeof(bufRainDaily), "%.2f", snap.rainDaily);

    StaticJsonDocument<384> doc;
    doc["timestamp"] = timeMgr.getFormattedLocal(snap.timestamp);
    doc["locid"] = Config::LOC_ID;
    doc["tempIn"] = bufIn;
    doc["tempOut"] = bufOut;
    doc["windSpeed"] = bufWind;
    doc["windSpeedMax"] = bufWindMax;
    doc["windDirDeg"] = bufDeg;
    doc["windDirName"] = snap.windDirName;
    doc["rain"] = bufRain;
    doc["rainDaily"] = bufRainDaily;

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

    StaticJsonDocument<256> statusDoc;
    statusDoc["time"] = timeMgr.getFormattedLocal(snap.timestamp);
    statusDoc["dir"] = dirCombo;
    statusDoc["light"] = snap.light;
    statusDoc["rainToday"] = snap.rainDaily;
    statusDoc["rainRate"] = snap.rainRate;

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
    if (WiFi.status() != WL_CONNECTED) return false;

    WiFiClientSecure client;
    client.setInsecure(); // Pre HTTPS spojenie bez certifikátu

    HTTPClient http;
    // Adafruit IO API v2 endpoint pre skupinové dáta
    String url = "https://io.adafruit.com/api/v2/" + String(Config::AIO_USERNAME) + "/groups/meteo/data";

    if (!http.begin(client, url)) {
        Serial.println("[AdafruitIO] Chyba inicializácie HTTP klienta");
        return false;
    }

    http.addHeader("X-AIO-Key", Config::getAioKey());
    http.addHeader("Content-Type", "application/json");

    // Vytvorenie JSON dokumentu pre Adafruit IO s presným časom (:00)
    StaticJsonDocument<1024> doc;
    doc["created_at"] = timeMgr.getFormattedISO(snap.timestamp);
    
    // Zoznam hodnôt pre jednotlivé feedy v skupine
    JsonArray feeds = doc.createNestedArray("feeds");

    auto addFeed = [&](const char* key, float val, int decimals) {
        JsonObject f = feeds.createNestedObject();
        f["key"] = key;
        if (decimals == 0) {
            f["value"] = String((int)round(val));
        } else {
            f["value"] = String(val, decimals);
        }
    };

    // Tieto kľúče musia presne zodpovedať názvom feedov v Adafruit IO skupine (meteo)
    addFeed("tempin", snap.tempIn, 2);
    addFeed("tempout", snap.tempOut, 2);
    // humidity a pressure vynechané pre dodržanie limitu max 10 feedov na bezplatnom účte Adafruit IO
    addFeed("wind-speed-avg", snap.windSpeedAvg, 1); // Rýchlosť na 0.1
    addFeed("wind-speed-max", snap.windSpeedMax, 1);
    addFeed("wind-direction", snap.windDirDeg, 0); // Stupne na celé číslo
    addFeed("light", snap.light, 0);
    addFeed("rain", snap.rain, 2);
    addFeed("rain-today", snap.rainDaily, 2);

    String jsonString;
    serializeJson(doc, jsonString);

    Serial.println("[AdafruitIO] Odosielam cez HTTPS s presným časom (:00)...");
    int httpCode = http.POST(jsonString);

    bool success = false;
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == 200 || httpCode == 201) {
            Serial.printf("[AdafruitIO] Úspešne odoslané! (Kód: %d)\n", httpCode);
            success = true;
        } else {
            Serial.printf("[AdafruitIO] Chyba: Server vrátil kód %d\n", httpCode);
            String payload = http.getString();
            Serial.println(payload);
        }
    } else {
        Serial.printf("[AdafruitIO] Chyba spojenia: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    return success;
}
