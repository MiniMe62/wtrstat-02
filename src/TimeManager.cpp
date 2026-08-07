#include "TimeManager.h"
#include <WiFi.h>

TimeManager::TimeManager()
    : _ntpUDP(),
      _timeClient(_ntpUDP, "pool.ntp.org", 0, 60000),
      _cest{"CEST", Last, Sun, Mar, 2, 120},
      _cet{"CET", Last, Sun, Oct, 3, 60},
      _tz(_cest, _cet) {
}

void TimeManager::begin() {
    _timeClient.begin();
}

bool TimeManager::syncNTP() {
    if (WiFi.status() != WL_CONNECTED) return false;

    Serial.println("[NTP] Pripájam k NTP serveru pool.ntp.org...");
    if (_timeClient.forceUpdate()) {
        time_t rawTime = _timeClient.getEpochTime();
        setTime(rawTime); // Nastavenie vnútornej TimeLib
        
        Serial.printf("[NTP] Sync OK! Lokálny čas: %s\n", getFormattedCustom().c_str());
        return true;
    }

    Serial.println("[NTP] Synchronizácia zlyhala!");
    return false;
}

time_t TimeManager::getUtcTime() const {
    return now();
}

time_t TimeManager::getLocalTime() const {
    return _tz.toLocal(now());
}

time_t TimeManager::getMarkTime(uint32_t intervalMin) const {
    time_t utc = now();
    uint32_t intervalSec = intervalMin * 60;
    time_t mark = (utc / intervalSec) * intervalSec;
    return mark;
}

String TimeManager::getFormattedCustom(time_t utcTime) const {
    if (utcTime == 0) utcTime = now();
    TimeChangeRule *tcr;
    time_t localT = _tz.toLocal(utcTime, &tcr);
    char buf[35];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d %s",
            year(localT), month(localT), day(localT),
            hour(localT), minute(localT),
            (tcr ? tcr->abbrev : "CET"));
    return String(buf);
}

String TimeManager::getFormattedISO(time_t utcTime) const {
    // ThingSpeak ISO 8601: YYYY-MM-DDTHH:MM:00Z
    char buf[30];
    sprintf(buf, "%04d-%02d-%02dT%02d:%02d:00Z",
            year(utcTime), month(utcTime), day(utcTime),
            hour(utcTime), minute(utcTime));
    return String(buf);
}

String TimeManager::getFormattedLocal(time_t utcTime) const {
    return getFormattedCustom(utcTime);
}
