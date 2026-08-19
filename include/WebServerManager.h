#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "TempSensorManager.h"
#include "Anemometer.h"
#include "WindVane.h"
#include "WifiService.h"
#include "TimeManager.h"

/**
 * @brief Lokálny Web Server poskytujúci responsive meteo Dashboard (HTML/CSS/JS), JSON API a Web OTA Update
 */
class WebServerManager {
public:
    WebServerManager();

    void begin(const TempSensorManager* tempMgr, const Anemometer* anemometer, const WindVane* windVane, const WifiService* wifiService, const TimeManager* timeMgr);
    void handleClient();

private:
    WebServer _server;
    const TempSensorManager* _tempMgr;
    const Anemometer* _anemometer;
    const WindVane* _windVane;
    const WifiService* _wifiService;
    const TimeManager* _timeMgr;

    void handleRoot();
    void handleApiLive();
    void handleNotFound();

    // Web OTA Update
    void handleUpdatePage();
    void handleUpdateUpload();
    void handleUpdateDone();
};

