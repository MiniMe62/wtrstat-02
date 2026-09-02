#pragma once

#include <Arduino.h>
#include <time.h>
#include "TempSensorManager.h"
#include "Anemometer.h"
#include "WindVane.h"
#include "LightSensor.h"
#include "RainGauge.h"

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
    float rain = 0.0f;      // Zrážky za interval (mm)
    float rainDaily = 0.0f; // Kumulatívne denné zrážky (mm)
    float rainRate = 0.0f;  // Intenzita zrážok (mm/h)
    float light = 0.0f;     // Jas (relatívne percento 0-100% alebo mV)
    bool isValid;
};

/**
 * @brief Akumulátor 15-minútových meraní
 */
class DataAggregator {
public:
    DataAggregator();

    void begin();
    void sample(const TempSensorManager& tempMgr, const Anemometer& anemometer, WindVane& windVane,
                const LightSensor& lightSensor, const RainGauge* rainGauge = nullptr);

    WeatherSnapshot finalizeSnapshot(time_t markTimestamp, const WindVane& windVane, const RainGauge* rainGauge = nullptr, bool is1Min = false);
    void reset();

private:
    double _tempInSum;
    uint32_t _tempInCount;

    double _tempOutSum;
    uint32_t _tempOutCount;

    double _windSpeedSum;
    float _windSpeedMax;
    uint32_t _windSpeedCount;

    double _lightSum;
    uint32_t _lightCount;
};
