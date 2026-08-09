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
    float humidity = 0.0f;  // Dnu
    float pressure = 0.0f;  // Von
    float windSpeedAvg;
    float windSpeedMax;
    float windDirDeg;
    String windDirName;
    float rain = 0.0f;      // Zrážky
    float light = 0.0f;     // Jas (do JSON)
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
