#include "LightSensor.h"

LightSensor::LightSensor(uint8_t pin, float loadResistorOhms)
    : _pin(pin),
      _loadResistor(loadResistorOhms),
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
    update();
    Serial.printf("[LightSensor] Inicializovany na ADC pin GPIO %d (R_load: %.1f Ohm)\n", _pin, _loadResistor);
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
    if (_rawMilliVolts > Config::LIGHT_ADC_ZERO_OFFSET_MV) {
        _lastMilliVolts = _rawMilliVolts - Config::LIGHT_ADC_ZERO_OFFSET_MV;
    } else {
        _lastMilliVolts = 0;
    }

    // Prepočet fotoprúdu I = U / R (v mikroampéroch uA)
    // TEMT6000 dáva cca 0.5 uA na 1 lux -> 1 uA = 2 luxy
    float current_uA = 0.0f;
    if (_loadResistor > 0.0f) {
        current_uA = ((float)_lastMilliVolts / _loadResistor) * 1000.0f;
    }
    _estimatedLux = current_uA * 2.0f;

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
        return "Oblacno / Svetly tien";
    } else if (_lastMilliVolts < Config::LIGHT_TH_SUNNY_MV) {
        return "Polooblacno";
    } else {
        return "Priame slnko";
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
    Serial.printf("[LightSensor] RAW: %4u mV | Ciste: %4u mV | Lux: %5.0f lx | Jas: %5.1f %% | %s | Svit: %s\n",
                  _rawMilliVolts, _lastMilliVolts, _estimatedLux, _brightnessPercent, getSkyCondition(), getSunshineFormatted().c_str());
}
