#pragma once

#include <Arduino.h>
#include "Pinout.h"
#include "Config.h"

/**
 * @brief Kalibračný prvok smeru vetra
 */
struct WindCalib {
    float ratio;         // Nameraná kalibračná hodnota (pomer)
    float angleDeg;      // Uhol v stupňoch (0° - 360°)
    const char* name;    // Textový názov smeru (napr. "S", "SSV")
};

/**
 * @brief Štatistika pre HW kalibráciu a ladenie veternej ružice
 */
struct DirectionStats {
    float minRatio = 999.0f;
    float maxRatio = 0.0f;
    double sumRatio = 0.0;
    uint32_t count = 0;
};

/**
 * @brief Trieda na určenie smeru vetra s goniometrickým/vektorovým priemerovaním
 */
class WindVane {
public:
    static constexpr uint8_t NUM_DIRECTIONS = 16;

    explicit WindVane(uint8_t pin = Pinout::WIND_VANE_PIN, uint8_t vccPin = Pinout::VCC_MONITOR_PIN);

    void begin();
    void update(); // Neblokujúce čítanie a vektorová akumulácia

    float getInstantAngle() const { return _instantAngle; }
    const char* getInstantDirName() const { return _instantDirName; }

    // Goniometrický priemer za akumulačnú periódu (napr. 15min)
    float getAveragedAngle() const;
    const char* getAveragedDirName() const;
    
    void resetAggregation();

    // Výpis debug štatistík pre kalibráciu
    void printDebugStats() const;
    
    // Živý výpis aktuálneho napätia pre manuálnu kalibráciu
    void printLiveDebug();

private:
    uint8_t _pin;
    uint8_t _vccPin;
    float _instantAngle;
    const char* _instantDirName;

    // Goniometrická akumulácia (vektorový priemer)
    double _sinSum;
    double _cosSum;
    uint32_t _sampleCount;

    // Štatistika pre debugovanie HW odporového deliča
    DirectionStats _stats[NUM_DIRECTIONS];

    float readRatioAveraged(uint8_t samples = 16);
    int findClosestDirectionIndex(float measuredRatio) const;

    static const WindCalib CALIBRATION_TABLE[NUM_DIRECTIONS];
    static const char* angleToDirName(float angleDeg);
};
