#pragma once

#include <Arduino.h>
#include <time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "Config.h"

/**
 * @brief Správca presného času s NTP synchronizáciou a formátovaním 15-minútových timestampov (:00)
 */
class TimeManager {
public:
    TimeManager();

    void begin();
    bool syncNTP();

    time_t getUtcTime() const;
    time_t getLocalTime() const;

    // Vráti najbližšiu predchádzajúcu 15-minútovú záverku (hh:00:00, hh:15:00, hh:30:00...)
    time_t getMarkTime(uint32_t intervalMin = Config::MEASURE_INTERVAL_MIN) const;

    // Formátuje lokálny čas vo formáte YYYY-MM-DD HH:mm <TZ> (napr. 2026-08-06 03:05 CEST)
    String getFormattedCustom(time_t utcTime = 0) const;
    String getFormattedTime(time_t utcTime = 0) const; // HH:MM:SS
    String getFormattedISO(time_t utcTime) const;
    String getFormattedLocal(time_t utcTime) const;

private:
    WiFiUDP _ntpUDP;
    NTPClient _timeClient;

    TimeChangeRule _cest;
    TimeChangeRule _cet;
    mutable Timezone _tz;
};
