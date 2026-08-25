#pragma once

#include <Arduino.h>
#include <TimeLib.h>
#include "Pinout.h"
#include "Config.h"

/**
 * @brief Trieda na meranie intenzity osvetlenia a detekciu slnečného svitu (TEMT6000)
 */
class LightSensor {
public:
    explicit LightSensor(uint8_t pin = Pinout::LIGHT_SENSOR_PIN, float loadResistorOhms = 220.0f);

    void begin();
    void update(); // Zmeria aktuálne napätie a prepočíta jas

    uint32_t getMilliVolts() const { return _lastMilliVolts; }
    float getEstimatedLux() const { return _estimatedLux; }
    float getBrightnessPercent() const { return _brightnessPercent; }
    const char* getSkyCondition() const;
    bool isDirectSun() const;

    // Sledovanie slnečného svitu (Sunshine Duration)
    void updateSunshineDuration(time_t localTime);
    uint32_t getSunshineMinutesToday() const { return _sunshineMinutesToday; }
    float getSunshineHoursToday() const { return _sunshineMinutesToday / 60.0f; }
    String getSunshineFormatted() const;
    void resetDailySunshine();

    void setDebug(bool debug) { _debug = debug; }
    void printLiveDebug() const;

private:
    uint8_t _pin;
    float _loadResistor;
    bool _debug;

    uint32_t _rawMilliVolts;
    uint32_t _lastMilliVolts;
    float _estimatedLux;
    float _brightnessPercent;

    // Denné počítadlo slnečného svitu
    uint32_t _sunshineMinutesToday;
    int _lastSunshineDay;
    uint32_t _lastSunshineCheckMinute;

    uint32_t readMilliVoltsAveraged(uint8_t samples = 20) const;
};
