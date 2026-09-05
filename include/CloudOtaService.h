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

class CloudOtaService {
public:
    CloudOtaService();

    OtaCheckResult checkVersion();
    bool performUpdate(const String& url);
    bool checkAdafruitCommand();

    bool isCalibMode() const { return _calibMode; }
    void setCalibMode(bool active);
    uint32_t getCalibRemainingSec() const;
    void updateCalibTimeout();

private:
    bool parseVersionJson(const String& json, OtaCheckResult& result);
    bool isNewerVersion(const String& newVer, const String& currVer);
    void resetAdafruitCommandFeed();
    void setAdafruitCommandStatus(const String& status);

    bool _calibMode = false;
    unsigned long _calibStartTime = 0;
};
