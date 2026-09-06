#include "LightSensor.h"

LightSensor::LightSensor(uint8_t pin, float loadResistorOhms, int8_t rangePin, float loadHighOhms, float loadLowOhms, bool dynamicRangeEnabled)
    : _pin(pin),
      _loadResistor(loadResistorOhms),
      _rangePin(rangePin),
      _loadResistorHigh(loadHighOhms),
      _loadResistorLow(loadLowOhms),
      _dynamicRangeEnabled(dynamicRangeEnabled),
      _isHighSensitivity(false),
      _debug(false),
      _rawMilliVolts(0),
      _lastMilliVolts(0),
      _estimatedLux(0.0f),
      _brightnessPercent(0.0f),
      _sunshineMinutesToday(0),
      _lastSunshineDay(-1),
      _lastSunshineCheckMinute(0xFFFFFFFF) {
}

void LightSensor::begin() {
    analogReadResolution(12);
    if (_dynamicRangeEnabled && _rangePin >= 0) {
        // Inicializujeme v LOW citlivosti (spodok odporu 2k pripojený na GND cez OUTPUT LOW)
        pinMode(_rangePin, OUTPUT);
        digitalWrite(_rangePin, LOW);
        _isHighSensitivity = false;
        Serial.printf("[LightSensor] Dynamic Ranging AKTÍVNY na pin GPIO %d (Riadiaci pin: %d, R_high: %.0fR, R_low: %.0fR)\n",
                      _pin, _rangePin, _loadResistorHigh, _loadResistorLow);
    } else {
        Serial.printf("[LightSensor] Inicializovany na ADC pin GPIO %d (Staticky R_load: %.1f Ohm)\n", _pin, _loadResistor);
    }
    update();
}

void LightSensor::setDynamicRange(bool enabled) {
    _dynamicRangeEnabled = enabled;
    if (_rangePin >= 0) {
        if (_dynamicRangeEnabled) {
            pinMode(_rangePin, OUTPUT);
            digitalWrite(_rangePin, LOW);
            _isHighSensitivity = false;
            Serial.printf("[LightSensor] Dynamic Ranging zapnuty (Riadiaci pin: %d)\n", _rangePin);
        } else {
            // Bezpečnostný režim: stiahnuť na GND pre štandardný paralelný odpor
            pinMode(_rangePin, OUTPUT);
            digitalWrite(_rangePin, LOW);
            _isHighSensitivity = false;
            Serial.println("[LightSensor] Dynamic Ranging vypnuty (zostava staticky na LOW rozsahu)");
        }
    }
}

uint32_t LightSensor::readMilliVoltsAveraged(uint8_t samples) const {
    uint32_t sum = 0;
    // 20 vzoriek po 1ms = presne 20ms (1 plná perióda 50 Hz siete/blikania svetiel)
    for (uint8_t i = 0; i < 20; i++) {
        sum += analogReadMilliVolts(_pin);
        delay(1);
    }
    return (sum / 20);
}

void LightSensor::update() {
    _rawMilliVolts = readMilliVoltsAveraged(20);

    // Odpočítanie hardvérového offsetu ESP32 (cca 142 mV pri 0V na pine)
    uint32_t cleanMilliVolts = 0;
    if (_rawMilliVolts > Config::LIGHT_ADC_ZERO_OFFSET_MV) {
        cleanMilliVolts = _rawMilliVolts - Config::LIGHT_ADC_ZERO_OFFSET_MV;
    }

    // Auto-ranging prepínanie rozsahov s hysteréziou
    if (_dynamicRangeEnabled && _rangePin >= 0) {
        if (!_isHighSensitivity && cleanMilliVolts < Config::LIGHT_RANGE_SWITCH_LOW_MV) {
            // Sme v LOW citlivosti (1.67k), ale svetla je primálo -> odopneme 2k odpor do INPUT (HIGH citlivosť 10k)
            pinMode(_rangePin, INPUT);
            _isHighSensitivity = true;
            delay(5); // Ustálenie náboja na pine
            _rawMilliVolts = readMilliVoltsAveraged(20);
            cleanMilliVolts = (_rawMilliVolts > Config::LIGHT_ADC_ZERO_OFFSET_MV) 
                              ? (_rawMilliVolts - Config::LIGHT_ADC_ZERO_OFFSET_MV) : 0;
        } else if (_isHighSensitivity && cleanMilliVolts > Config::LIGHT_RANGE_SWITCH_HIGH_MV) {
            // Sme v HIGH citlivosti (10k), ale napätie sa blíži k stropu -> pripojíme 2k odpor na GND (LOW citlivosť 1.67k)
            pinMode(_rangePin, OUTPUT);
            digitalWrite(_rangePin, LOW);
            _isHighSensitivity = false;
            delay(5); // Ustálenie náboja
            _rawMilliVolts = readMilliVoltsAveraged(20);
            cleanMilliVolts = (_rawMilliVolts > Config::LIGHT_ADC_ZERO_OFFSET_MV) 
                              ? (_rawMilliVolts - Config::LIGHT_ADC_ZERO_OFFSET_MV) : 0;
        }
    }

    // Skutočný fotoprúd I = U / R (v mikroampéroch uA)
    float activeResistor = _loadResistor;
    if (_dynamicRangeEnabled && _rangePin >= 0) {
        activeResistor = _isHighSensitivity ? _loadResistorHigh : _loadResistorLow;
    }

    float current_uA = 0.0f;
    if (activeResistor > 0.0f) {
        current_uA = ((float)cleanMilliVolts / activeResistor) * 1000.0f;
    }
    _estimatedLux = current_uA * 35.0f;

    // Normalizácia na virtuálne milivolty (ekvivalent pôvodného 2k rozsahu, kde 2800 mV = 100% jas)
    // Zabezpečuje stálu kompatibilitu s prahmi stavu oblohy, heliografom a cloudom.
    if (_dynamicRangeEnabled && _rangePin >= 0) {
        _lastMilliVolts = (uint32_t)((current_uA * _loadResistor) / 1000.0f);
    } else {
        _lastMilliVolts = cleanMilliVolts;
    }

    // Relatívne percento jasu (2800 mV čistého svetla = 100%)
    _brightnessPercent = ((float)_lastMilliVolts / 2800.0f) * 100.0f;
    if (_brightnessPercent > 100.0f) {
        _brightnessPercent = 100.0f;
    }
}

const char* LightSensor::getSkyCondition() const {
    if (_lastMilliVolts < Config::LIGHT_TH_NIGHT_MV) {
        return "Noc / Tma";
    } else if (_lastMilliVolts < Config::LIGHT_TH_OVERCAST_MV) {
        return "Husto zamracene / Dazd";
    } else if (_lastMilliVolts < Config::LIGHT_TH_CLOUDY_MV) {
        return "Zamracene / Oblacno";
    } else if (_lastMilliVolts < Config::LIGHT_TH_SUNNY_MV) {
        return "Polooblacno";
    } else {
        return "Jasno / Slnko";
    }
}

bool LightSensor::isDirectSun() const {
    return (_lastMilliVolts >= Config::LIGHT_TH_SUNNY_MV);
}

void LightSensor::updateSunshineDuration(time_t localTime) {
    if (localTime <= 100000) return; // Neplatný čas pred NTP synchronizáciou

    int currentDay = day(localTime);
    uint32_t currentMinuteOfDay = hour(localTime) * 60 + minute(localTime);

    // 1. Denný reset o polnoci
    if (_lastSunshineDay != -1 && currentDay != _lastSunshineDay) {
        Serial.printf("[LightSensor] Polnoc - denny reset slnecneho svitu (Vcerajsi svit: %s)\n",
                      getSunshineFormatted().c_str());
        resetDailySunshine();
    }
    _lastSunshineDay = currentDay;

    // 2. Kontrola každej novej minúty (Campbell-Stokes heliograf princíp)
    if (currentMinuteOfDay != _lastSunshineCheckMinute) {
        _lastSunshineCheckMinute = currentMinuteOfDay;

        // Ak v tejto minúte intenzita presahuje prah priameho slnka, pripočítame 1 minútu svitu
        if (isDirectSun()) {
            _sunshineMinutesToday++;
        }
    }
}

String LightSensor::getSunshineFormatted() const {
    uint32_t hrs = _sunshineMinutesToday / 60;
    uint32_t mins = _sunshineMinutesToday % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%uh %02um", hrs, mins);
    return String(buf);
}

void LightSensor::resetDailySunshine() {
    _sunshineMinutesToday = 0;
}

void LightSensor::printLiveDebug() const {
    const char* rangeStr = "STAT";
    if (_dynamicRangeEnabled && _rangePin >= 0) {
        rangeStr = _isHighSensitivity ? "R_HIGH(10k)" : "R_LOW(1.67k)";
    }
    Serial.printf("[LightSensor] RAW: %4u mV | Ciste: %4u mV | Lux: %5.0f lx | Jas: %5.1f %% | [%s] | %s | Svit: %s\n",
                  _rawMilliVolts, _lastMilliVolts, _estimatedLux, _brightnessPercent, rangeStr, getSkyCondition(), getSunshineFormatted().c_str());
}
