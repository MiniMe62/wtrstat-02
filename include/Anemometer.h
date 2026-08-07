#pragma once

#include <Arduino.h>
#include "Pinout.h"
#include "Config.h"

/**
 * @brief Trieda na meranie rýchlosti vetra pomocou Hallovho senzora (Anemometer)
 */
class Anemometer {
public:
    static constexpr uint8_t WINDOW_SIZE = 12; // 12 vzoriek po 5s = 60 sekúnd kĺzavé okno

    explicit Anemometer(uint8_t pin = Pinout::HALL_SENSOR);

    void begin();
    void update(); // Neblokujúci výpočet rýchlosti vetra

    float getWindSpeed() const { return _currentWindSpeed; }
    uint32_t getTotalPulseCount() const;

private:
    uint8_t _pin;
    uint32_t _lastReportTime;
    float _currentWindSpeed;

    uint32_t _pulseHistory[WINDOW_SIZE];
    uint32_t _timeHistoryMs[WINDOW_SIZE];
    uint8_t _historyIndex;
    uint8_t _historyCount;
};
