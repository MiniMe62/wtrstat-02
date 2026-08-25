#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Config.h"
#include "Pinout.h"
#include "TempSensorManager.h"
#include "Anemometer.h"
#include "WindVane.h"
#include "LightSensor.h"
#include "WifiService.h"
#include "TimeManager.h"

/**
 * @brief Správca I2C OLED displeja (SSD1306 128x64) pre zobrazenie živých meteo dát.
 */
class DisplayManager {
public:
    DisplayManager();

    bool begin();
    void update(const TempSensorManager& tempMgr,
                const Anemometer& anemometer,
                const WindVane& windVane,
                const LightSensor& lightSensor,
                const WifiService& wifiService,
                const TimeManager& timeMgr);

    void checkTouchAndTimeout();
    void wake();
    void sleep();

    bool isReady() const { return _isInitialized; }
    bool isScreenOn() const { return _isScreenOn; }

private:
    static constexpr uint8_t SCREEN_WIDTH = 128;
    static constexpr uint8_t SCREEN_HEIGHT = 64;
    static constexpr int8_t OLED_RESET = -1; // Zdieľaný reset pin s ESP32

    Adafruit_SSD1306 _display;
    bool _isInitialized;
    bool _isScreenOn;
    uint32_t _screenOnUntil;
    uint32_t _lastTouchCheckTime;
    uint32_t _lastTouchDetectedTime;
    uint32_t _lastDrawTime;
};
