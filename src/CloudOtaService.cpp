#include "CloudOtaService.h"
#include "Config.h"
#include <WiFi.h>

CloudOtaService::CloudOtaService() {
}

bool CloudOtaService::isNewerVersion(const String& newVer, const String& currVer) {
    if (newVer.isEmpty()) return false;
    if (newVer == currVer) return false;
    return true; // Ak sa líši od aktuálnej verzie, je k dispozícii aktualizácia
}

bool CloudOtaService::parseVersionJson(const String& json, OtaCheckResult& result) {
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        result.error = "Chyba parsovania JSON z GitHubu";
        return false;
    }

    const char* locId = Config::LOC_ID;
    if (!doc.containsKey(locId)) {
        result.error = String("Stanica '") + locId + "' nebola najdena v version.json";
        return false;
    }

    JsonObject stationObj = doc[locId];
    result.newVersion = stationObj["version"] | "";
    result.downloadUrl = stationObj["firmware_url"] | "";
    result.notes = stationObj["notes"] | "";
    result.currentVersion = Config::FIRMWARE_VERSION;

    if (result.newVersion.isEmpty() || result.downloadUrl.isEmpty()) {
        result.error = "Neplatny format verzie alebo URL v JSON";
        return false;
    }

    result.updateAvailable = isNewerVersion(result.newVersion, result.currentVersion);
    return true;
}

OtaCheckResult CloudOtaService::checkVersion() {
    OtaCheckResult result;
    result.updateAvailable = false;
    result.currentVersion = Config::FIRMWARE_VERSION;

    if (WiFi.status() != WL_CONNECTED) {
        result.error = "WiFi nie je pripojene";
        return result;
    }

    WiFiClientSecure client;
    client.setInsecure(); // Pre bezpečné stiahnutie z GitHub HTTPS bez správy Root CA certifikátov

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(10000);

    Serial.printf("[CloudOTA] Kontrolujem verziu z URL: %s (Stanica: %s)\n", Config::GITHUB_VERSION_URL, Config::LOC_ID);

    if (http.begin(client, Config::GITHUB_VERSION_URL)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            parseVersionJson(payload, result);
        } else {
            result.error = String("HTTP GET zlyhal s kodom: ") + httpCode;
        }
        http.end();
    } else {
        result.error = "Nepodarilo sa vytvorit HTTPS spojenie na GitHub";
    }

    return result;
}

bool CloudOtaService::performUpdate(const String& url) {
    if (WiFi.status() != WL_CONNECTED || url.isEmpty()) {
        Serial.println("[CloudOTA] Zlyhanie: WiFi nie je pripojene alebo prazdna URL");
        return false;
    }

    Serial.printf("[CloudOTA] Spustam stahovanie a flashovanie z: %s\n", url.c_str());

    WiFiClientSecure client;
    client.setInsecure();

    httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    httpUpdate.rebootOnUpdate(true);

    t_httpUpdate_return ret = httpUpdate.update(client, url);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("[CloudOTA] Aktualizacia zlyhala! Chyba (%d): %s\n",
                          httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            return false;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("[CloudOTA] Ziadna nova verzia");
            return false;

        case HTTP_UPDATE_OK:
            Serial.println("[CloudOTA] Aktualizacia uspesna! Restartujem...");
            return true;
    }

    return false;
}

void CloudOtaService::resetAdafruitCommandFeed() {
    if (WiFi.status() != WL_CONNECTED) return;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String url = String("https://io.adafruit.com/api/v2/") + Config::AIO_USERNAME + "/feeds/" + Config::AIO_CMD_FEED + "/data";
    if (http.begin(client, url)) {
        http.addHeader("X-AIO-Key", Config::getAioKey());
        http.addHeader("Content-Type", "application/json");
        http.POST("{\"value\":\"IDLE\"}");
        http.end();
        Serial.println("[CloudOTA] Adafruit IO feed 'meteo-cmd' bol resetovaný na stav 'IDLE'.");
    }
}

bool CloudOtaService::checkAdafruitCommand() {
    if (!Config::ENABLE_ADAFRUIT_IO_UPLOAD || WiFi.status() != WL_CONNECTED) {
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setTimeout(5000);
    String url = String("https://io.adafruit.com/api/v2/") + Config::AIO_USERNAME + "/feeds/" + Config::AIO_CMD_FEED + "/data/last";

    if (!http.begin(client, url)) {
        return false;
    }

    http.addHeader("X-AIO-Key", Config::getAioKey());
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK || httpCode == 200) {
        String payload = http.getString();
        http.end();

        StaticJsonDocument<512> doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (!err) {
            String val = doc["value"] | "";
            val.trim();
            if (val.equalsIgnoreCase("UPDATE")) {
                Serial.println("\n[CloudOTA] ==========================================");
                Serial.println("[CloudOTA] Prijatý príkaz UPDATE z Adafruit IO!");
                Serial.println("[CloudOTA] ==========================================");
                
                // 1. Ochrana pred zacyklením - resetujeme feed na IDLE
                resetAdafruitCommandFeed();

                // 2. Kontrola novej verzie
                OtaCheckResult res = checkVersion();
                if (res.updateAvailable && !res.downloadUrl.isEmpty()) {
                    Serial.printf("[CloudOTA] Na GitHube je dostupná nová verzia v%s (aktuálna v%s). Spúšťam inštaláciu...\n",
                                  res.newVersion.c_str(), res.currentVersion.c_str());
                    return performUpdate(res.downloadUrl);
                } else if (res.error.length() > 0) {
                    Serial.printf("[CloudOTA] Kontrola verzie zlyhala: %s\n", res.error.c_str());
                } else {
                    Serial.printf("[CloudOTA] Zariadenie už má najnovšiu verziu v%s. Inštalácia vynechaná.\n",
                                  res.currentVersion.c_str());
                }
            }
        }
        return false;
    }

    http.end();
    return false;
}
