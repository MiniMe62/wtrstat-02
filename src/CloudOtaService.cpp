#include "CloudOtaService.h"
#include "Config.h"
#include "WindVane.h"
#include "LightSensor.h"
#include <WiFi.h>
#include <Preferences.h>

CloudOtaService::CloudOtaService() {
}

void CloudOtaService::begin(const WindVane* windVane, LightSensor* lightSensor) {
    _windVane = windVane;
    _lightSensor = lightSensor;

    // Detekcia prvého štartu po aktualizácii firmvéru
    Preferences prefs;
    if (prefs.begin("wtrstat", false)) {
        String prevVer = prefs.getString("fw_ver_ota", "");
        if (prevVer != Config::FIRMWARE_VERSION) {
            prefs.putString("fw_ver_ota", Config::FIRMWARE_VERSION);
            _justUpdated = true;
            Serial.printf("[CloudOTA] Detegovaný prvý štart novej verzie v%s (predchádzajúca: '%s')\n",
                          Config::FIRMWARE_VERSION, prevVer.c_str());
        }
        prefs.end();
    }
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

    Serial.printf("\n[CloudOTA] Kontrolujem verziu z URL: %s (Stanica: %s)\n", Config::GITHUB_VERSION_URL, Config::LOC_ID);
    Serial.printf("[CloudOTA] Voľná RAM pred kontrolou: %u bajtov (najväčší súvislý blok: %u B)\n",
                  ESP.getFreeHeap(), ESP.getMaxAllocHeap());

    bool isHttps = String(Config::GITHUB_VERSION_URL).startsWith("https://");
    WiFiClientSecure* secureClient = nullptr;
    WiFiClient plainClient;

    if (isHttps) {
        secureClient = new WiFiClientSecure();
        if (!secureClient) {
            result.error = "Nedostatok RAM pre TLS klienta";
            return result;
        }
        secureClient->setInsecure();
        secureClient->setHandshakeTimeout(25);
        secureClient->setTimeout(15000);
    } else {
        plainClient.setTimeout(15000);
    }

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setUserAgent("ESP32-WeatherStation-OTA");
    http.setTimeout(15000);

    bool begun = isHttps ? http.begin(*secureClient, Config::GITHUB_VERSION_URL)
                         : http.begin(plainClient, Config::GITHUB_VERSION_URL);

    if (begun) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK || httpCode == 200) {
            String payload = http.getString();
            parseVersionJson(payload, result);
        } else {
            result.error = String("HTTP GET zlyhal s kodom: ") + httpCode;
        }
        http.end();
    } else {
        result.error = "Nepodarilo sa vytvorit spojenie na GitHub";
    }

    if (secureClient) {
        delete secureClient;
    }

    return result;
}

bool CloudOtaService::performUpdate(const String& url) {
    if (WiFi.status() != WL_CONNECTED || url.isEmpty()) {
        Serial.println("[CloudOTA] Zlyhanie: WiFi nie je pripojene alebo prazdna URL");
        setAdafruitCommandStatus("OTA ERR: WiFi odpojene");
        return false;
    }

    Serial.printf("\n[CloudOTA] ==========================================\n");
    Serial.printf("[CloudOTA] Spúšťam OTA sťahovanie z:\n[CloudOTA] %s\n", url.c_str());
    Serial.printf("[CloudOTA] Voľná RAM pred sťahovaním: %u bajtov (najväčší blok: %u B)\n",
                  ESP.getFreeHeap(), ESP.getMaxAllocHeap());
    Serial.printf("[CloudOTA] ==========================================\n");

    bool isHttps = url.startsWith("https://");
    WiFiClientSecure* secureClient = nullptr;
    WiFiClient plainClient;

    if (isHttps) {
        secureClient = new WiFiClientSecure();
        if (!secureClient) {
            Serial.println("[CloudOTA] Chyba: Nedostatok RAM pre WiFiClientSecure!");
            setAdafruitCommandStatus("OTA ERR: Nedostatok RAM pre TLS");
            return false;
        }
        secureClient->setInsecure();
        secureClient->setHandshakeTimeout(30);
        secureClient->setTimeout(25000);
    } else {
        plainClient.setTimeout(25000);
    }

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setUserAgent("ESP32-WeatherStation-OTA");
    http.setTimeout(35000);

    bool begun = isHttps ? http.begin(*secureClient, url)
                         : http.begin(plainClient, url);

    if (!begun) {
        Serial.println("[CloudOTA] Nepodarilo sa inicializovať HTTPClient spojenie.");
        setAdafruitCommandStatus("OTA ERR: HTTP init zlyhal");
        if (secureClient) delete secureClient;
        return false;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK && httpCode != 200) {
        Serial.printf("[CloudOTA] HTTP GET zlyhal s kódom: %d (%s)\n", httpCode, http.errorToString(httpCode).c_str());
        setAdafruitCommandStatus(String("OTA ERR: HTTP kód ") + httpCode);
        http.end();
        if (secureClient) delete secureClient;
        return false;
    }

    int contentLength = http.getSize();
    Serial.printf("[CloudOTA] Server nahlásil veľkosť binárky: %d bajtov (%d KB)\n",
                  contentLength, contentLength / 1024);

    if (contentLength <= 0) {
        Serial.println("[CloudOTA] Chyba: Neplatná veľkosť súboru (Content-Length <= 0)!");
        setAdafruitCommandStatus("OTA ERR: Nulova velkost");
        http.end();
        if (secureClient) delete secureClient;
        return false;
    }

    if (!Update.begin(contentLength)) {
        Serial.printf("[CloudOTA] Update.begin zlyhal! Nedostatok miesta v OTA partícii. Kód chyby: %u\n", Update.getError());
        setAdafruitCommandStatus(String("OTA ERR: Partícia kód ") + Update.getError());
        http.end();
        if (secureClient) delete secureClient;
        return false;
    }

    Serial.println("[CloudOTA] Zapisujem streamované dáta priamo do flash pamäte po blokoch...");
    WiFiClient* stream = http.getStreamPtr();
    stream->setTimeout(20000);

    const size_t CHUNK_SIZE = 4096;
    uint8_t* buff = (uint8_t*)malloc(CHUNK_SIZE);
    if (!buff) {
        buff = (uint8_t*)malloc(2048);
    }
    if (!buff) {
        Serial.println("[CloudOTA] Chyba: Nedostatok RAM pre download buffer!");
        setAdafruitCommandStatus("OTA ERR: Nedostatok RAM pre buffer");
        Update.abort();
        http.end();
        if (secureClient) delete secureClient;
        return false;
    }

    size_t actualChunkSize = (ESP.getFreeHeap() > 30000) ? CHUNK_SIZE : 2048;
    size_t totalWritten = 0;
    unsigned long lastDataMs = millis();
    unsigned long lastLogMs = millis();
    bool writeOk = true;

    while (http.connected() && (totalWritten < (size_t)contentLength)) {
        size_t avail = stream->available();
        if (avail > 0) {
            size_t toRead = (avail > actualChunkSize) ? actualChunkSize : avail;
            if (totalWritten + toRead > (size_t)contentLength) {
                toRead = (size_t)contentLength - totalWritten;
            }
            int bytesRead = stream->read(buff, toRead);
            if (bytesRead > 0) {
                size_t bytesWritten = Update.write(buff, bytesRead);
                if (bytesWritten != (size_t)bytesRead) {
                    Serial.printf("[CloudOTA] Chyba zápisu flash! Napísané %u z %d B. Kód: %u\n",
                                  bytesWritten, bytesRead, Update.getError());
                    writeOk = false;
                    break;
                }
                totalWritten += bytesWritten;
                lastDataMs = millis();

                if (millis() - lastLogMs > 2500) {
                    lastLogMs = millis();
                    int pct = (int)((totalWritten * 100ULL) / contentLength);
                    Serial.printf("[CloudOTA] Priebeh: %d%% (%u / %d KB)\n",
                                  pct, totalWritten / 1024, contentLength / 1024);
                }
                yield();
            }
        } else {
            if (millis() - lastDataMs > 30000) {
                Serial.println("[CloudOTA] Timeout: Stream bez dát dlhšie ako 30s!");
                writeOk = false;
                break;
            }
            delay(10);
            yield();
        }
    }

    free(buff);

    if (!writeOk || totalWritten != (size_t)contentLength) {
        Serial.printf("[CloudOTA] Zlyhanie: Zapísané iba %u z %d bajtov! Prerušujem.\n",
                      totalWritten, contentLength);
        setAdafruitCommandStatus(String("OTA ERR: Zapisanych len ") + (totalWritten / 1024) + "/" + (contentLength / 1024) + " KB");
        Update.abort();
        http.end();
        if (secureClient) delete secureClient;
        return false;
    }

    if (!Update.end()) {
        Serial.printf("[CloudOTA] Overenie a finalizácia zápisu zlyhala! Kód chyby: %u\n", Update.getError());
        setAdafruitCommandStatus(String("OTA ERR: Update.end kód ") + Update.getError());
        http.end();
        if (secureClient) delete secureClient;
        return false;
    }

    if (!Update.isFinished()) {
        Serial.println("[CloudOTA] Chyba: Zápis OTA nie je kompletne dokončený!");
        setAdafruitCommandStatus("OTA ERR: Neukoncene");
        http.end();
        if (secureClient) delete secureClient;
        return false;
    }

    Serial.println("\n[CloudOTA] ==========================================");
    Serial.println("[CloudOTA] AKTUALIZÁCIA ÚSPEŠNE DOKONČENÁ!");
    Serial.println("[CloudOTA] ESP32 sa reštartuje do nového firmvéru...");
    Serial.println("[CloudOTA] ==========================================\n");

    setAdafruitCommandStatus("OTA OK -> REBOOT");
    http.end();
    if (secureClient) delete secureClient;
    delay(1500);
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
        DynamicJsonDocument doc(1280);
        doc["value"] = status;
        String payload;
        serializeJson(doc, payload);
        http.POST(payload);
        http.end();
        Serial.printf("[CloudOTA] Adafruit IO feed 'meteo-cmd' aktualizovaný:\n%s\n", status.c_str());
    }
}

void CloudOtaService::sendStatsToAdafruit() {
    if (_windVane) {
        String stats = _windVane->getFormattedStats();
        setAdafruitCommandStatus(stats);
    } else {
        Serial.println("[CloudOTA] Nemám referenciu na WindVane pre odoslanie štatistiky.");
    }
}

void CloudOtaService::setCalibMode(bool active) {
    bool wasActive = _calibMode;
    _calibMode = active;
    if (active) {
        _calibStartTime = millis();
        Serial.println("\n[CalibMode] >>> STREŠNÝ KALIBRAČNÝ REŽIM ZAPNUTÝ na 15 minút (interval 6s) <<<");
    } else {
        _calibStartTime = 0;
        Serial.println("\n[CalibMode] >>> STREŠNÝ KALIBRAČNÝ REŽIM VYPNUTÝ (návrat k 1min/15min) <<<");
        if (wasActive) {
            sendStatsToAdafruit();
        }
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

    // Automatické odoslanie potvrdenia do Adafruit IO pri prvom štarte po úspešnom OTA
    if (_justUpdated) {
        _justUpdated = false;
        String bootMsg = String("BOOT OK: v") + Config::FIRMWARE_VERSION;
        setAdafruitCommandStatus(bootMsg);
        Serial.printf("[CloudOTA] Odoslané potvrdenie '%s' do Adafruit IO feedu '%s'\n",
                      bootMsg.c_str(), Config::AIO_CMD_FEED);
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
                Serial.println("[CloudOTA] Plánujem odloženú OTA aktualizáciu pre čistú RAM...");
                Serial.println("[CloudOTA] ==========================================");
                _otaPending = true;
                _otaPendingTime = millis();
                _pendingOtaUrl = "";
                setAdafruitCommandStatus("OTA: Inic. za 3s...");
            } else if (val.startsWith("OTA_URL:") || val.startsWith("OTA:")) {
                String customUrl = val.substring(val.indexOf(':') + 1);
                customUrl.trim();
                Serial.printf("\n[CloudOTA] Prijatý príkaz vlastnej OTA URL: %s\n", customUrl.c_str());
                _otaPending = true;
                _otaPendingTime = millis();
                _pendingOtaUrl = customUrl;
                setAdafruitCommandStatus("OTA: Custom URL za 3s...");
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
            } else if (val.equalsIgnoreCase("STATS") || val.equalsIgnoreCase("STAT") || val.equalsIgnoreCase("STATISTIKA")) {
                Serial.println("\n[CloudOTA] ==========================================");
                Serial.println("[CloudOTA] Prijatý príkaz STATS z Adafruit IO! Odosielam tabuľku...");
                Serial.println("[CloudOTA] ==========================================");
                sendStatsToAdafruit();
            } else if (val.equalsIgnoreCase("DR_ON") || val.equalsIgnoreCase("LIGHT_DR_ON")) {
                Serial.println("\n[CloudOTA] ==========================================");
                Serial.println("[CloudOTA] Prijatý príkaz DR_ON z Adafruit IO! Aktivujem Dynamic Ranging...");
                Serial.println("[CloudOTA] ==========================================");
                if (_lightSensor) {
                    _lightSensor->setDynamicRange(true);
                    setAdafruitCommandStatus("DR: ON");
                }
            } else if (val.equalsIgnoreCase("DR_OFF") || val.equalsIgnoreCase("LIGHT_DR_OFF")) {
                Serial.println("\n[CloudOTA] ==========================================");
                Serial.println("[CloudOTA] Prijatý príkaz DR_OFF z Adafruit IO! Deaktivujem Dynamic Ranging...");
                Serial.println("[CloudOTA] ==========================================");
                if (_lightSensor) {
                    _lightSensor->setDynamicRange(false);
                    setAdafruitCommandStatus("DR: OFF");
                }
            } else if (val.equalsIgnoreCase("VER") || val.equalsIgnoreCase("INFO") || val.equalsIgnoreCase("VERSION")) {
                Serial.println("\n[CloudOTA] ==========================================");
                Serial.println("[CloudOTA] Prijatý príkaz VER z Adafruit IO! Odosielam info o verzii...");
                Serial.println("[CloudOTA] ==========================================");
                uint32_t uptimeSec = millis() / 1000;
                uint32_t hrs = uptimeSec / 3600;
                uint32_t mins = (uptimeSec % 3600) / 60;
                char buf[120];
                snprintf(buf, sizeof(buf), "v%s (%s) | Heap:%uKB | RSSI:%ddBm | Up:%uh%02um",
                         Config::FIRMWARE_VERSION, Config::LOC_ID, ESP.getFreeHeap() / 1024, WiFi.RSSI(), hrs, mins);
                setAdafruitCommandStatus(String(buf));
            }
        }
        return false;
    }

    http.end();
    return false;
}

void CloudOtaService::handlePendingOta() {
    if (!_otaPending) return;

    // Počkáme aspoň 3 sekundy od prijatia príkazu, aby sa všetky sieťové volania dokončili a uvoľnila sa RAM
    if (millis() - _otaPendingTime < 3000) {
        return;
    }

    _otaPending = false;
    Serial.println("\n[CloudOTA] >>> SPUŠŤAM ODLOŽENÚ OTA AKTUALIZÁCIU V ČISTOM PROSTREDÍ <<<");
    Serial.printf("[CloudOTA] Voľná RAM: %u bajtov (najväčší súvislý blok: %u B)\n",
                  ESP.getFreeHeap(), ESP.getMaxAllocHeap());

    String targetUrl = _pendingOtaUrl;
    if (targetUrl.isEmpty()) {
        OtaCheckResult res = checkVersion();
        if (res.updateAvailable && !res.downloadUrl.isEmpty()) {
            Serial.printf("[CloudOTA] Nájdená nová verzia v%s (aktuálna v%s). Spúšťam inštaláciu...\n",
                          res.newVersion.c_str(), res.currentVersion.c_str());
            performUpdate(res.downloadUrl);
        } else if (res.error.length() > 0) {
            Serial.printf("[CloudOTA] Kontrola verzie zlyhala: %s\n", res.error.c_str());
            setAdafruitCommandStatus(String("OTA ERR: ") + res.error);
        } else {
            Serial.printf("[CloudOTA] Zariadenie už má najnovšiu verziu v%s. Inštalácia vynechaná.\n",
                          res.currentVersion.c_str());
            setAdafruitCommandStatus(String("OTA: v") + Config::FIRMWARE_VERSION + " je aktualna");
        }
    } else {
        Serial.printf("[CloudOTA] Spúšťam priamy update zo zadanej URL: %s\n", targetUrl.c_str());
        performUpdate(targetUrl);
    }
}

