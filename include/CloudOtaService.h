#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>
#include <ArduinoJson.h>

struct OtaCheckResult {
    bool updateAvailable;
    String currentVersion;
    String newVersion;
    String downloadUrl;
    String notes;
    String error;
};

class WindVane;
class LightSensor;

class CloudOtaService {
public:
    CloudOtaService();

    void begin(const WindVane* windVane = nullptr, LightSensor* lightSensor = nullptr);

    OtaCheckResult checkVersion();
    bool performUpdate(const String& url);
    bool checkAdafruitCommand();
    void handlePendingOta();
    bool isOtaPending() const { return _otaPending; }

    bool isCalibMode() const { return _calibMode; }
    void setCalibMode(bool active);
    uint32_t getCalibRemainingSec() const;
    void updateCalibTimeout();
    void sendStatsToAdafruit();
    void setAdafruitCommandStatus(const String& status);

private:
    bool parseVersionJson(const String& json, OtaCheckResult& result);
    bool isNewerVersion(const String& newVer, const String& currVer);
    void resetAdafruitCommandFeed();

    const WindVane* _windVane = nullptr;
    LightSensor* _lightSensor = nullptr;
    bool _calibMode = false;
    unsigned long _calibStartTime = 0;
    bool _justUpdated = false;

    // Asynchrónna odložená OTA pre uvoľnenie RAM
    bool _otaPending = false;
    unsigned long _otaPendingTime = 0;
    String _pendingOtaUrl;
};

