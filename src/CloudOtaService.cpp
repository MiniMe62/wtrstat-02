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

    Serial.printf("[CloudOTA] Spúšťam priame HTTPS OTA sťahovanie z: %s\n", url.c_str());

    WiFiClientSecure client;
    client.setInsecure(); // GitHub HTTPS certifikáty bez potreby správy Root CA

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(35000); // 35s timeout pre stabilné stiahnutie 1.2 MB

    if (!http.begin(client, url)) {
        Serial.println("[CloudOTA] Nepodarilo sa inicializovať HTTPClient spojenie.");
        return false;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK && httpCode != 200) {
        Serial.printf("[CloudOTA] HTTP GET zlyhal s kódom: %d (%s)\n", httpCode, http.errorToString(httpCode).c_str());
        http.end();
        return false;
    }

    int contentLength = http.getSize();
    Serial.printf("[CloudOTA] Server nahlásil veľkosť binárky: %d bajtov\n", contentLength);

    if (contentLength <= 0) {
        Serial.println("[CloudOTA] Chyba: Neplatná veľkosť súboru (Content-Length <= 0)!");
        http.end();
        return false;
    }

    if (!Update.begin(contentLength)) {
        Serial.printf("[CloudOTA] Update.begin zlyhal! Nedostatok miesta v OTA partícii. Kód chyby: %u\n", Update.getError());
        http.end();
        return false;
    }

    Serial.println("[CloudOTA] Zapisujem streamované dáta priamo do flash pamäte...");
    WiFiClient* stream = http.getStreamPtr();
    size_t written = Update.writeStream(*stream);

    if (written != (size_t)contentLength) {
        Serial.printf("[CloudOTA] Zlyhanie: Zapísané iba %u z %d bajtov! Prerušujem.\n", written, contentLength);
        Update.abort();
        http.end();
        return false;
    }

    if (!Update.end()) {
        Serial.printf("[CloudOTA] Overenie a finalizácia zápisu zlyhala! Kód chyby: %u\n", Update.getError());
        http.end();
        return false;
    }

    if (!Update.isFinished()) {
        Serial.println("[CloudOTA] Chyba: Zápis OTA nie je kompletne dokončený!");
        http.end();
        return false;
    }

    Serial.println("\n[CloudOTA] ==========================================");
    Serial.println("[CloudOTA] AKTUALIZÁCIA ÚSPEŠNE DOKONČENÁ!");
    Serial.println("[CloudOTA] ESP32 sa reštartuje do nového firmvéru...");
    Serial.println("[CloudOTA] ==========================================\n");

    http.end();
    delay(1000);
    ESP.restart();
    return true;
}

void CloudOtaService::resetAdafruitCommandFeed() {
    setAdafruitCommandStatus("IDLE");
}

void CloudOtaService::setAdafruitCommandStatus(const String& status) {
    if (WiFi.status() != WL_CONNECTED) return;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String url = String("https://io.adafruit.com/api/v2/") + Config::AIO_USERNAME + "/feeds/" + Config::AIO_CMD_FEED + "/data";
    if (http.begin(client, url)) {
        http.addHeader("X-AIO-Key", Config::getAioKey());
        http.addHeader("Content-Type", "application/json");
        StaticJsonDocument<128> doc;
        doc["value"] = status;
        String payload;
        serializeJson(doc, payload);
        http.POST(payload);
        http.end();
        Serial.printf("[CloudOTA] Adafruit IO feed 'meteo-cmd' nastavený na: '%s'\n", status.c_str());
    }
}

void CloudOtaService::setCalibMode(bool active) {
    _calibMode = active;
    if (active) {
        _calibStartTime = millis();
        Serial.println("\n[CalibMode] >>> STREŠNÝ KALIBRAČNÝ REŽIM ZAPNUTÝ na 15 minút (interval 5s) <<<");
    } else {
        _calibStartTime = 0;
        Serial.println("\n[CalibMode] >>> STREŠNÝ KALIBRAČNÝ REŽIM VYPNUTÝ (návrat k 1min/15min) <<<");
    }
}

uint32_t CloudOtaService::getCalibRemainingSec() const {
    if (!_calibMode) return 0;
    unsigned long elapsedMs = millis() - _calibStartTime;
    unsigned long timeoutMs = Config::CALIB_TIMEOUT_SEC * 1000UL;
    if (elapsedMs >= timeoutMs) return 0;
    return (timeoutMs - elapsedMs) / 1000UL;
}

void CloudOtaService::updateCalibTimeout() {
    if (_calibMode) {
        if (getCalibRemainingSec() == 0) {
            Serial.println("[CalibMode] Timeout 15 minút vypršal. Automatické ukončenie kalibrácie.");
            setCalibMode(false);
            setAdafruitCommandStatus("IDLE");
        }
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
            } else if (val.equalsIgnoreCase("CALIB") || val.equalsIgnoreCase("CALIB_START") || val.equalsIgnoreCase("CALIBRATION")) {
                if (!_calibMode) {
                    Serial.println("\n[CloudOTA] ==========================================");
                    Serial.println("[CloudOTA] Prijatý príkaz CALIB z Adafruit IO!");
                    Serial.println("[CloudOTA] ==========================================");
                    setCalibMode(true);
                }
            } else if (val.equalsIgnoreCase("STOP") || val.equalsIgnoreCase("CALIB_STOP") || val.equalsIgnoreCase("CALIB_OFF") || val.equalsIgnoreCase("IDLE")) {
                if (_calibMode) {
                    Serial.println("\n[CloudOTA] ==========================================");
                    Serial.println("[CloudOTA] Prijatý príkaz IDLE/STOP - vypínam kalibráciu!");
                    Serial.println("[CloudOTA] ==========================================");
                    setCalibMode(false);
                }
            }
        }
        return false;
    }

    http.end();
    return false;
}
