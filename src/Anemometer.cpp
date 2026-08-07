#include "Anemometer.h"
#include <esp_timer.h>

// Globálne volatile premenné pre čítač v prerušení ISR
static volatile uint32_t s_windPulseCount = 0;
static volatile int64_t s_lastPulseTimeUs = 0;

// Hardvérová ISR rutina s 5ms debouncingom
void IRAM_ATTR windPulseISR() {
    int64_t nowUs = esp_timer_get_time();
    if (nowUs - s_lastPulseTimeUs > 5000) { // 5ms softvérový filter zákmity
        s_windPulseCount++;
        s_lastPulseTimeUs = nowUs;
    }
}

Anemometer::Anemometer(uint8_t pin)
    : _pin(pin),
      _lastReportTime(0),
      _currentWindSpeed(0.0f),
      _historyIndex(0),
      _historyCount(0) {
    for (uint8_t i = 0; i < WINDOW_SIZE; i++) {
        _pulseHistory[i] = 0;
        _timeHistoryMs[i] = 0;
    }
}

void Anemometer::begin() {
    pinMode(_pin, INPUT_PULLUP);
    _lastReportTime = millis();
    attachInterrupt(digitalPinToInterrupt(_pin), windPulseISR, FALLING);
    Serial.printf("[Anemometer] Inicializovaný na GPIO %d (ISR FALLING)\n", _pin);
}

void Anemometer::update() {
    uint32_t currentMillis = millis();

    if (currentMillis - _lastReportTime >= Config::HALL_REPORT_INTERVAL_MS) {
        uint32_t elapsedMs = currentMillis - _lastReportTime;
        _lastReportTime = currentMillis;

        // Atomický odpočet pulzov za aktuálny interval
        noInterrupts();
        uint32_t pulses = s_windPulseCount;
        s_windPulseCount = 0;
        interrupts();

        // Uloženie vzorky do kĺzavého okna
        _pulseHistory[_historyIndex] = pulses;
        _timeHistoryMs[_historyIndex] = elapsedMs;

        _historyIndex = (_historyIndex + 1) % WINDOW_SIZE;
        if (_historyCount < WINDOW_SIZE) {
            _historyCount++;
        }

        // Sumarizácia kĺzavého okna
        uint32_t totalPulses = 0;
        uint32_t totalTimeMs = 0;
        for (uint8_t i = 0; i < _historyCount; i++) {
            totalPulses += _pulseHistory[i];
            totalTimeMs += _timeHistoryMs[i];
        }

        float elapsedSecs = (float)totalTimeMs / 1000.0f;
        if (elapsedSecs < 0.1f) elapsedSecs = 0.1f;

        float hz = (float)totalPulses / elapsedSecs;

        // Výpočet rýchlosti vetra (m/s)
        // C = 2 * PI * r = 2 * PI * 0.195m = 1.2252m
        float circumference = 2.0f * M_PI * Config::ANEMOMETER_RADIUS_M;
        float rotationsPerSec = hz / (float)Config::ANEMOMETER_PULSES_PER_REV;
        _currentWindSpeed = rotationsPerSec * circumference * Config::ANEMOMETER_CALIBRATION_FACTOR;
    }
}

uint32_t Anemometer::getTotalPulseCount() const {
    noInterrupts();
    uint32_t pulses = s_windPulseCount;
    interrupts();
    return pulses;
}
