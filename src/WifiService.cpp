#include "WifiService.h"

WifiService::WifiService() : _connectedSSID("") {
}

bool WifiService::connectBestNetwork() {
    Serial.println("[WiFi] Skenujem dostupné WiFi siete...");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(false); // Odpojí predošlé pripojenie bez vypnutia rádia
    delay(150);

    int n = WiFi.scanNetworks(false, false); // Synchrónny sken
    
    if (n <= 0) {
        Serial.printf("[WiFi] Skenovanie zlyhalo alebo sa nenašli žiadne siete (n=%d).\n", n);
        WiFi.scanDelete();
        return false;
    }

    Serial.printf("[WiFi] Skenovanie dokončené. Nájdených %d sietí:\n", n);
    for (int i = 0; i < n; ++i) {
        Serial.printf("   - %s (%d dBm, Ch:%d)\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));
    }

    int bestRSSI = -1000;
    int bestKnownIdx = -1;

    for (int i = 0; i < n; ++i) {
        String scannedSSID = WiFi.SSID(i);
        int rssi = WiFi.RSSI(i);

        for (uint8_t k = 0; k < Config::KNOWN_WIFI_COUNT; k++) {
            if (scannedSSID == String(Config::KNOWN_WIFI_NETWORKS[k].ssid)) {
                if (rssi > bestRSSI) {
                    bestRSSI = rssi;
                    bestKnownIdx = k;
                }
            }
        }
    }

    WiFi.scanDelete(); // Uvoľnenie pamäte po skene

    if (bestKnownIdx != -1) {
        const char* targetSSID = Config::KNOWN_WIFI_NETWORKS[bestKnownIdx].ssid;
        const char* targetPass = Config::KNOWN_WIFI_NETWORKS[bestKnownIdx].pass;

        Serial.printf("[WiFi] Pripájam k najsilnejšej známej sieti: %s (%d dBm)\n", targetSSID, bestRSSI);
        WiFi.begin(targetSSID, targetPass);

        uint8_t attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 25) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            _connectedSSID = targetSSID;
            Serial.printf("[WiFi] Úspešne pripojený! IP: %s, RSSI: %d dBm\n", getIPAddress().c_str(), WiFi.RSSI());
            return true;
        }
    }

    Serial.println("[WiFi] Pripojenie zlyhalo (žiadna známa sieť neodpovedá).");
    return false;
}

bool WifiService::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void WifiService::ensureConnected() {
    if (!isConnected()) {
        connectBestNetwork();
    }
}
