#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "Config.h"

/**
 * @brief Správca pre WiFi pripojenie s auto-vyhľadaním najsilnejšej známej siete cez WiFiMulti
 */
class WifiService {
public:
    WifiService();

    bool connectBestNetwork();
    bool isConnected() const;
    void ensureConnected();

    String getConnectedSSID() const { return _connectedSSID; }
    int getRSSI() const { return WiFi.RSSI(); }
    String getIPAddress() const { return WiFi.localIP().toString(); }

private:
    WiFiMulti _wifiMulti;
    String _connectedSSID;
    uint32_t _lastReconnectAttempt;
};

