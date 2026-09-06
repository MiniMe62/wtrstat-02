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
    explicit LightSensor(uint8_t pin = Pinout::LIGHT_SENSOR_PIN,
                         float loadResistorOhms = 2000.0f,
                         int8_t rangePin = -1,
                         float loadHighOhms = 10000.0f,
                         float loadLowOhms = 1667.0f,
                         bool dynamicRangeEnabled = false);

    void begin();
    void update(); // Zmeria aktuálne napätie a prepočíta jas

    uint32_t getMilliVolts() const { return _lastMilliVolts; }
    uint32_t getRawMilliVolts() const { return _rawMilliVolts; }
    float getEstimatedLux() const { return _estimatedLux; }
    float getBrightnessPercent() const { return _brightnessPercent; }
    const char* getSkyCondition() const;
    bool isDirectSun() const;
    bool isDynamicRangeEnabled() const { return _dynamicRangeEnabled; }
    bool isHighSensitivity() const { return _isHighSensitivity; }

    void setDynamicRange(bool enabled);

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
    int8_t _rangePin;
    float _loadResistorHigh;
    float _loadResistorLow;
    bool _dynamicRangeEnabled;
    bool _isHighSensitivity;
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
