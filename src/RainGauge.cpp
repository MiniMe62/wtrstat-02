#include "RainGauge.h"
#include <esp_timer.h>

// Globálne volatile premenné pre ISR prerušenie
static volatile uint32_t s_rainPulseCount = 0;
static volatile uint32_t s_totalRainPulseCount = 0;
static volatile int64_t s_lastRainPulseUs = 0;

// ISR rutina s precíznym debouncing filtrom
void IRAM_ATTR rainPulseISR() {
    int64_t nowUs = esp_timer_get_time();
    int64_t debounceUs = (int64_t)Config::RAIN_DEBOUNCE_MS * 1000LL;
    if (nowUs - s_lastRainPulseUs > debounceUs) {
        s_rainPulseCount++;
        s_totalRainPulseCount++;
        s_lastRainPulseUs = nowUs;
    }
}

RainGauge::RainGauge(uint8_t pin, float mmPerPulse)
    : _pin(pin),
      _mmPerPulse(mmPerPulse),
      _debug(Config::DEBUG_RAIN_GAUGE),
      _pulses15Min(0),
      _pulses1Min(0),
      _pulsesToday(0),
      _lastDay(-1),
      _lastMinuteCheck(0),
      _historyIndex(0),
      _currentRainRate(0.0f) {
    for (uint8_t i = 0; i < HISTORY_MINUTES; i++) {
        _minuteHistory[i] = 0;
    }
}

void RainGauge::begin() {
    pinMode(_pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_pin), rainPulseISR, FALLING);
    Serial.printf("[RainGauge] Inicializovaný na GPIO %d (ISR FALLING, kalibrácia %.4f mm/tip, debounce %u ms)\n",
                  _pin, _mmPerPulse, Config::RAIN_DEBOUNCE_MS);
}

void RainGauge::update(time_t localTime) {
    // 1. Atomický odpočet nových pulzov z ISR
    noInterrupts();
    uint32_t newPulses = s_rainPulseCount;
    s_rainPulseCount = 0;
    interrupts();

    // 2. Spracovanie zmeny dňa (reset denného úhrnu o 00:00:00)
    int currentDay = day(localTime);
    if (_lastDay == -1) {
        _lastDay = currentDay;
    } else if (currentDay != _lastDay && year(localTime) > 2020) {
        Serial.printf("[RainGauge] Nastala zmena dňa (%d -> %d). Resetujem denný úhrn zrážok (Dnes bolo: %.2f mm / %u tipov).\n",
                      _lastDay, currentDay, getRainToday(), _pulsesToday);
        resetDailyRain();
        _lastDay = currentDay;
    }

    // 3. Akumulácia nových pulzov
    if (newPulses > 0) {
        _pulses15Min += newPulses;
        _pulses1Min += newPulses;
        _pulsesToday += newPulses;
        _minuteHistory[_historyIndex] += newPulses;

        if (_debug) {
            Serial.printf("[RainGauge] Zaznamenaný preklop! (+%u tipov, Dnes: %.2f mm, 15m: %.2f mm, Celkovo pulzov: %u)\n",
                          newPulses, getRainToday(), getRain15Min(), getTotalPulses());
        }
    }

    // 4. Spracovanie minútových slotov pre výpočet intenzity dažďa (Rain Rate v mm/h)
    uint32_t currentMinute = (uint32_t)(localTime / 60);
    if (_lastMinuteCheck == 0) {
        _lastMinuteCheck = currentMinute;
    } else if (currentMinute > _lastMinuteCheck) {
        uint32_t missedMinutes = currentMinute - _lastMinuteCheck;
        if (missedMinutes > HISTORY_MINUTES) missedMinutes = HISTORY_MINUTES;

        for (uint32_t m = 0; m < missedMinutes; m++) {
            _historyIndex = (_historyIndex + 1) % HISTORY_MINUTES;
            _minuteHistory[_historyIndex] = 0; // Vyčistenie starého slotu
        }
        _lastMinuteCheck = currentMinute;

        // Sumarizácia histórie za posledných 60 minút
        uint32_t sum60MinPulses = 0;
        for (uint8_t i = 0; i < HISTORY_MINUTES; i++) {
            sum60MinPulses += _minuteHistory[i];
        }
        _currentRainRate = sum60MinPulses * _mmPerPulse; // mm za 60 min = mm/h
    }
}

float RainGauge::getRain15Min() const {
    return _pulses15Min * _mmPerPulse;
}

float RainGauge::getRain1Min() const {
    return _pulses1Min * _mmPerPulse;
}

float RainGauge::getRainToday() const {
    return _pulsesToday * _mmPerPulse;
}

float RainGauge::getRainRateMmH() const {
    return _currentRainRate;
}

uint32_t RainGauge::getTotalPulses() const {
    return s_totalRainPulseCount;
}

uint32_t RainGauge::getPulsesToday() const {
    return _pulsesToday;
}

const char* RainGauge::getRainIntensityDescription() const {
    if (_currentRainRate <= 0.01f) return "Bez zrážok";
    if (_currentRainRate < 2.5f)  return "Slabý dážď";
    if (_currentRainRate < 7.6f)  return "Mierny dážď";
    if (_currentRainRate < 50.0f) return "Silný dážď";
    return "Prudký lejak";
}

void RainGauge::resetInterval15Min() {
    _pulses15Min = 0;
}

void RainGauge::resetInterval1Min() {
    _pulses1Min = 0;
}

void RainGauge::resetDailyRain() {
    _pulsesToday = 0;
}

void RainGauge::simulatePulse() {
    noInterrupts();
    s_totalRainPulseCount++;
    interrupts();

    _pulses15Min++;
    _pulses1Min++;
    _pulsesToday++;
    _minuteHistory[_historyIndex]++;

    // Okamžitý prepočet 60-minútovej intenzity
    uint32_t sum60MinPulses = 0;
    for (uint8_t i = 0; i < HISTORY_MINUTES; i++) {
        sum60MinPulses += _minuteHistory[i];
    }
    _currentRainRate = sum60MinPulses * _mmPerPulse;

    Serial.printf("[RainGauge] Simulovaný 1 impulz -> Dnes: %.2f mm (%u tipov), 15m: %.2f mm, Intenzita: %.2f mm/h\n",
                  getRainToday(), _pulsesToday, getRain15Min(), _currentRainRate);
}

void RainGauge::printLiveDebug() const {
    Serial.printf("[RainGauge Live] Dnes: %.2f mm (%u tipov), 15m: %.2f mm, Intenzita: %.2f mm/h (%s), Celkovo tipov: %u\n",
                  getRainToday(), _pulsesToday, getRain15Min(), _currentRainRate, getRainIntensityDescription(), getTotalPulses());
}
