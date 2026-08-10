#pragma once

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DataAggregator.h"
#include "TimeManager.h"
#include "Config.h"

/**
 * @brief Služba na bezpečné odosielanie nameraných dát na Google Sheets a ThingSpeak
 */
class UploaderService {
public:
    UploaderService();

    bool send15MinSnapshot(const WeatherSnapshot& snap, const TimeManager& timeMgr);
    bool send1MinSnapshot(const WeatherSnapshot& snap, const TimeManager& timeMgr);

private:
    bool sendToGoogleSheets(const WeatherSnapshot& snap, const TimeManager& timeMgr);
    bool sendToThingSpeak(const WeatherSnapshot& snap, const TimeManager& timeMgr);
    bool sendToAdafruitIO(const WeatherSnapshot& snap, const TimeManager& timeMgr);
};
