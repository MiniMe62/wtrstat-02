#pragma once

#include <Arduino.h>
#include <time.h>
#include "TempSensorManager.h"
#include "Anemometer.h"
#include "WindVane.h"

/**
 * @brief Sumárne namerané dáta pre 15-minútovú záverku
 */
struct WeatherSnapshot {
    time_t timestamp;
    float tempIn;
    float tempOut;
    float windSpeedAvg;
    float windSpeedMax;
    float windDirDeg;
    String windDirName;
    bool isValid;
};

/**
 * @brief Akumulátor 15-minútových meraní
 */
class DataAggregator {
public:
    DataAggregator();

    void begin();
    void sample(const TempSensorManager& tempMgr, const Anemometer& anemometer, WindVane& windVane);

    WeatherSnapshot finalizeSnapshot(time_t markTimestamp, const WindVane& windVane);
    void reset();

private:
    double _tempInSum;
    uint32_t _tempInCount;

    double _tempOutSum;
    uint32_t _tempOutCount;

    double _windSpeedSum;
    float _windSpeedMax;
    uint32_t _windSpeedCount;
};
