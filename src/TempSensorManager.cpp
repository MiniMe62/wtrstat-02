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

    // 10-bit rozlíšenie vyžaduje cca 188 ms na konverziu
    if (_isConversionPending && (now - _lastRequestTime >= 200)) {
        if (_sensorCount >= 1) {
            float t1 = _sensors.getTempC(_addrIn);
            if (t1 > -55.0f && t1 < 125.0f) {
                _tempIn = roundToQuarterStep(t1);
            }
        }

        if (_sensorCount >= 2) {
            float t2 = _sensors.getTempC(_addrOut);
            if (t2 > -55.0f && t2 < 125.0f) {
                _tempOut = roundToQuarterStep(t2);
            }
        } else {
            // Ak je zapojený zatiaľ len 1 senzor na kontaktnom poli
            _tempOut = _tempIn;
        }

        _validRead = (_tempIn > -55.0f);
        _isConversionPending = false;
    }

    // Pravidelné vyžiadanie novej konverzie každé 2 sekundy
    if (!_isConversionPending && (now - _lastRequestTime >= Config::SENSOR_READ_INTERVAL_MS)) {
        _sensors.requestTemperatures();
        _lastRequestTime = now;
        _isConversionPending = true;
    }
}
