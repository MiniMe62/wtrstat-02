#include "WifiService.h"

WifiService::WifiService() : _connectedSSID(""), _lastReconnectAttempt(0) {
    for (uint8_t k = 0; k < Config::KNOWN_WIFI_COUNT; k++) {
        _wifiMulti.addAP(Config::KNOWN_WIFI_NETWORKS[k].ssid, Config::KNOWN_WIFI_NETWORKS[k].pass);
    }
}

bool WifiService::connectBestNetwork() {
    Serial.println("[WiFi] Inicializujem WiFi (WiFiMulti)...");
    WiFi.mode(WIFI_STA);

    Serial.print("[WiFi] Hľadám a pripájam najsilnejšiu známu sieť");

    // Skúsime sa pripojiť s max timeoutom 6 sekúnd (neprepadne do nekonečného visenia)
    uint32_t startMs = millis();
    while (_wifiMulti.run() != WL_CONNECTED && millis() - startMs < 6000) {
        delay(350);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        _connectedSSID = WiFi.SSID();
        Serial.printf("[WiFi] Úspešne pripojený k sieti: %s (IP: %s, RSSI: %d dBm, Ch: %d)\n",
                      _connectedSSID.c_str(), getIPAddress().c_str(), WiFi.RSSI(), WiFi.channel());
        return true;
    }

    Serial.println("[WiFi] ⚠️ Žiadna známa WiFi sieť sa nepripojila v limite 6s. Stanica pokračuje v offline režime.");
    return false;
}

bool WifiService::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void WifiService::ensureConnected() {
    if (!isConnected()) {
        uint32_t now = millis();
        // Skúšame reconnect max raz za 15 sekúnd, aby to neblokovalo slučku
        if (now - _lastReconnectAttempt >= 15000) {
            _lastReconnectAttempt = now;
            Serial.println("[WiFi] Pokus o opätovné pripojenie na pozadí...");
            if (_wifiMulti.run() == WL_CONNECTED) {
                _connectedSSID = WiFi.SSID();
                Serial.printf("[WiFi] Obnovené pripojenie k sieti: %s (IP: %s)\n",
                              _connectedSSID.c_str(), getIPAddress().c_str());
            }
        }
    }
}

