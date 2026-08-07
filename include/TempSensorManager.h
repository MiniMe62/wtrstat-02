#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Pinout.h"
#include "Config.h"

/**
 * @brief Správca pre čítanie 2 kusov Dallas DS18B20 teplomerov na jednej 1-Wire zbernici.
 */
class TempSensorManager {
public:
    TempSensorManager();
    
    void begin();
    void update(); // Neblokujúce čítanie teplôt

    float getTempIn() const { return _tempIn; }
    float getTempOut() const { return _tempOut; }
    bool isReadValid() const { return _validRead; }

private:
    OneWire _oneWire;
    DallasTemperature _sensors;
    
    DeviceAddress _addrIn;
    DeviceAddress _addrOut;
    uint8_t _sensorCount;

    float _tempIn;
    float _tempOut;
    bool _validRead;

    uint32_t _lastRequestTime;
    bool _isConversionPending;
};
