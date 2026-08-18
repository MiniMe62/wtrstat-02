#include "TempSensorManager.h"
#include <cmath>

static float roundToQuarterStep(float val) {
    if (val <= -55.0f || val >= 125.0f) return val;
    return std::round(val * 4.0f) / 4.0f;
}

TempSensorManager::TempSensorManager()
    : _oneWire(Pinout::TEMP_SENSOR),
      _sensors(&_oneWire),
      _sensorCount(0),
      _tempIn(-127.0f),
      _tempOut(-127.0f),
      _validRead(false),
      _lastRequestTime(0),
      _isConversionPending(false) {
}

void TempSensorManager::begin() {
#ifdef SIMULATE_TEMP_SENSORS
#if SIMULATE_TEMP_SENSORS
    _sensorCount = 2;
    _tempIn = 22.50f;
    _tempOut = 18.25f;
    _validRead = true;
    _lastRequestTime = millis();
    Serial.printf("[DS18B20] SIMULÁCIA AKTÍVNA (generujú sa virtuálne teploty pre 2 senzory: In=%.2f°C, Out=%.2f°C)\n", _tempIn, _tempOut);
    return;
#endif
#endif

    _sensors.begin();
    _sensors.setWaitForConversion(false); // Neblokujúci režim!
    
    _sensorCount = _sensors.getDeviceCount();
    Serial.printf("[DS18B20] Nájdených %d senzorov na pin %d\n", _sensorCount, Pinout::TEMP_SENSOR);

    if (_sensorCount >= 1) {
        _sensors.getAddress(_addrIn, 0);
        _sensors.setResolution(_addrIn, Config::TEMP_RESOLUTION_BITS);
    }
    if (_sensorCount >= 2) {
        _sensors.getAddress(_addrOut, 1);
        _sensors.setResolution(_addrOut, Config::TEMP_RESOLUTION_BITS);
    }

    // Spustíme prvé meranie
    _sensors.requestTemperatures();
    _lastRequestTime = millis();
    _isConversionPending = true;
}

void TempSensorManager::update() {
    uint32_t now = millis();

#ifdef SIMULATE_TEMP_SENSORS
#if SIMULATE_TEMP_SENSORS
    if (now - _lastRequestTime >= Config::SENSOR_READ_INTERVAL_MS) {
        _lastRequestTime = now;
        // Simulujeme mierne kolísanie teplôt s krokom 0.25 °C (10-bit rozlíšenie DS18B20)
        // Vnútorná teplota: okolo 22.5 °C (+- 0.5 °C)
        float deltaIn = (float)random(-1, 2) * 0.25f;
        _tempIn += deltaIn;
        if (_tempIn < 21.0f) _tempIn = 21.0f;
        if (_tempIn > 24.0f) _tempIn = 24.0f;
        _tempIn = roundToQuarterStep(_tempIn);

        // Vonkajšia teplota: okolo 18.5 °C (+- 0.75 °C)
        float deltaOut = (float)random(-2, 3) * 0.25f;
        _tempOut += deltaOut;
        if (_tempOut < 14.0f) _tempOut = 14.0f;
        if (_tempOut > 26.0f) _tempOut = 26.0f;
        _tempOut = roundToQuarterStep(_tempOut);

        _validRead = true;
    }
    return;
#endif
#endif

    // 10-bit rozlíšenie vyžaduje cca 188 ms na konverziu
    if (_isConversionPending && (now - _lastRequestTime >= 200)) {
        if (_sensorCount >= 1) {
            float t1 = _sensors.getTempC(_addrIn);
            // Plauzibilný rozsah: -40.0°C až +65.0°C, ignorovať -127.0 (odpojené) aj +85.0 (power-on reset glitch)
            if (t1 >= -40.0f && t1 <= 65.0f && std::abs(t1 - 85.0f) > 0.1f) {
                _tempIn = roundToQuarterStep(t1);
            }
        }

        if (_sensorCount >= 2) {
            float t2 = _sensors.getTempC(_addrOut);
            if (t2 >= -40.0f && t2 <= 65.0f && std::abs(t2 - 85.0f) > 0.1f) {
                _tempOut = roundToQuarterStep(t2);
            }
        } else {
            // Ak je zapojený zatiaľ len 1 senzor na kontaktnom poli
            _tempOut = _tempIn;
        }

        _validRead = (_tempIn > -40.0f && _tempIn <= 65.0f);
        _isConversionPending = false;
    }

    // Pravidelné vyžiadanie novej konverzie každé 2 sekundy
    if (!_isConversionPending && (now - _lastRequestTime >= Config::SENSOR_READ_INTERVAL_MS)) {
        _sensors.requestTemperatures();
        _lastRequestTime = now;
        _isConversionPending = true;
    }
}

