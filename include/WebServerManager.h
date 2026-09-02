#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "TempSensorManager.h"
#include "Anemometer.h"
#include "WindVane.h"
#include "LightSensor.h"
#include "RainGauge.h"
#include "WifiService.h"
#include "TimeManager.h"
#include "CloudOtaService.h"

/**
 * @brief Lokálny Web Server poskytujúci responsive meteo Dashboard (HTML/CSS/JS), JSON API, Web OTA a GitHub Cloud OTA
 */
class WebServerManager {
public:
    WebServerManager();

    void begin(const TempSensorManager* tempMgr, const Anemometer* anemometer, const WindVane* windVane,
               const WifiService* wifiService, const TimeManager* timeMgr, CloudOtaService* cloudOta = nullptr,
               const LightSensor* lightSensor = nullptr, RainGauge* rainGauge = nullptr);
    void handleClient();

private:
    WebServer _server;
    const TempSensorManager* _tempMgr;
    const Anemometer* _anemometer;
    const WindVane* _windVane;
    const LightSensor* _lightSensor;
    RainGauge* _rainGauge;
    const WifiService* _wifiService;
    const TimeManager* _timeMgr;
    CloudOtaService* _cloudOta;

    void handleRoot();
    void handleApiLive();
    void handleApiTestRainTip();
    void handleNotFound();

    // Web & Cloud OTA Update
    void handleUpdatePage();
    void handleUpdateUpload();
    void handleUpdateDone();
    void handleApiOtaCheck();
    void handleApiOtaCloudUpdate();
};


