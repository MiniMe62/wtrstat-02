#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>
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

private:
    bool parseVersionJson(const String& json, OtaCheckResult& result);
    bool isNewerVersion(const String& newVer, const String& currVer);
};
