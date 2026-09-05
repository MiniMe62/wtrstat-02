#include "WindVane.h"
#include <cmath>
#include <algorithm>

// Kalibračná tabuľka nameraných mV z wtrStat-01 pre 16 smerov
/* 7.8.2024 - aktualizovaná kalibračná tabuľka po testovaní na wtrStat-02
    (všetky hodnoty sú v mV, uhol v stupňoch, názov smeru)
=== WindVane Kalibračné Štatistiky pri napajacom napati cca 3.28 V ===
Smer SZ   (tab  173 mV) -> Min:  171, Max:  177, Avg:  173 mV (Vzoriek: 13)
Smer JZ   (tab  350 mV) -> Min:  349, Max:  355, Avg:  352 mV (Vzoriek: 13)
Smer JV   (tab  616 mV) -> Min:  614, Max:  622, Avg:  618 mV (Vzoriek: 15)
Smer SV   (tab  826 mV) -> Min:  825, Max:  832, Avg:  828 mV (Vzoriek: 11)
Smer Z    (tab 1071 mV) -> Min: 1068, Max: 1076, Avg: 1072 mV (Vzoriek: 11)
Smer ZSZ  (tab 1140 mV) -> Min: 1139, Max: 1147, Avg: 1143 mV (Vzoriek: 15)
Smer ZJZ  (tab 1231 mV) -> Min: 1219, Max: 1235, Avg: 1230 mV (Vzoriek: 17)
Smer S    (tab 1389 mV) -> Min: 1351, Max: 1361, Avg: 1356 mV (Vzoriek: 13)
Smer SSZ  (tab 1406 mV) -> Min: 1402, Max: 1415, Avg: 1408 mV (Vzoriek: 13)
Smer SZ   (tab 0.114) -> Min: 0.114, Max: 0.114, Avg: 0.114 (Vzoriek: 13)
Smer JZ   (tab 0.220) -> Min: 0.220, Max: 0.220, Avg: 0.220 (Vzoriek: 13)
Smer JV   (tab 0.381) -> Min: 0.381, Max: 0.381, Avg: 0.381 (Vzoriek: 15)
Smer SV   (tab 0.508) -> Min: 0.508, Max: 0.508, Avg: 0.508 (Vzoriek: 11)
Smer Z    (tab 0.657) -> Min: 0.657, Max: 0.657, Avg: 0.657 (Vzoriek: 11)
Smer ZSZ  (tab 0.698) -> Min: 0.698, Max: 0.698, Avg: 0.698 (Vzoriek: 15)
Smer ZJZ  (tab 0.752) -> Min: 0.752, Max: 0.752, Avg: 0.752 (Vzoriek: 17)
Smer S    (tab 0.828) -> Min: 0.828, Max: 0.828, Avg: 0.828 (Vzoriek: 13)
Smer SSZ  (tab 0.862) -> Min: 0.862, Max: 0.862, Avg: 0.862 (Vzoriek: 13)
Smer SSV  (tab 1.022) -> Min: 1.022, Max: 1.022, Avg: 1.022 (Vzoriek: 18)
Smer J    (tab 1.121) -> Min: 1.121, Max: 1.121, Avg: 1.121 (Vzoriek: 15)
Smer JJZ  (tab 1.164) -> Min: 1.164, Max: 1.164, Avg: 1.164 (Vzoriek: 13)
Smer JJV  (tab 1.198) -> Min: 1.198, Max: 1.198, Avg: 1.198 (Vzoriek: 14)
Smer V    (tab 1.316) -> Min: 1.316, Max: 1.316, Avg: 1.316 (Vzoriek: 33)
Smer VJV  (tab 1.365) -> Min: 1.365, Max: 1.365, Avg: 1.365 (Vzoriek: 13)
Smer VSV  (tab 1.386) -> Min: 1.386, Max: 1.386, Avg: 1.386 (7.8.2026 19:35 Namerane na funkcnom WindVane
=== WindVane Kalibračné Štatistiky (Pomer) ===
Smer SZ   (tab 0.103) -> Min: 0.100, Max: 0.106, Avg: 0.103 (Vzoriek: 12)
Smer JZ   (tab 0.210) -> Min: 0.209, Max: 0.213, Avg: 0.211 (Vzoriek: 15)
Smer JV   (tab 0.370) -> Min: 0.368, Max: 0.372, Avg: 0.371 (Vzoriek: 13)
Smer SV   (tab 0.496) -> Min: 0.494, Max: 0.498, Avg: 0.496 (Vzoriek: 13)
Smer Z    (tab 0.644) -> Min: 0.641, Max: 0.646, Avg: 0.644 (Vzoriek: 14)
Smer ZSZ  (tab 0.685) -> Min: 0.679, Max: 0.689, Avg: 0.685 (Vzoriek: 20)
Smer ZJZ  (tab 0.739) -> Min: 0.725, Max: 0.741, Avg: 0.737 (Vzoriek: 11)
Smer S    (tab 0.812) -> Min: 0.810, Max: 0.816, Avg: 0.814 (Vzoriek: 16)
Smer SSZ  (tab 0.846) -> Min: 0.842, Max: 0.848, Avg: 0.846 (Vzoriek: 12)
Smer SSV  (tab 1.006) -> Min: 1.005, Max: 1.009, Avg: 1.007 (Vzoriek: 21)
Smer J    (tab 1.105) -> Min: 1.104, Max: 1.109, Avg: 1.106 (Vzoriek: 20)
Smer JJZ  (tab 1.147) -> Min: 1.145, Max: 1.149, Avg: 1.147 (Vzoriek: 12)
Smer JJV  (tab 1.185) -> Min: 1.184, Max: 1.188, Avg: 1.185 (Vzoriek: 11)
Smer V    (tab 1.300) -> Min: 1.299, Max: 1.303, Avg: 1.301 (Vzoriek: 13)
Smer VJV  (tab 1.349) -> Min: 1.346, Max: 1.353, Avg: 1.349 (Vzoriek: 13)
Smer VSV  (tab 1.373) -> Min: 1.372, Max: 1.376, Avg: 1.375 (Vzoriek: 17)

5.9.2026 - Reálna strešná kalibrácia TEST_VIDIEK (>12 800 meraní pri vetre z JZ sektora)
S   : 0.737 (0.725-0.755) n=74
SSV : 0.604 (0.601-0.639) n=2593
SV  : 1.017 (0.961-1.064) n=53
V   : 0.501 (0.499-0.503) n=20
VJV : 0.493 (0.489-0.496) n=18
JV  : 1.170 (1.076-1.279) n=238
JJV : 0.528 (0.525-0.530) n=42
J   : 0.579 (0.573-0.600) n=1925
JJZ : 0.560 (0.550-0.572) n=286
JZ  : 1.442 (1.314-1.459) n=5913
ZJZ : 0.791 (0.756-0.797) n=326
Z   : 0.879 (0.840-0.923) n=959
ZSZ : 0.817 (0.797-0.839) n=182
SZ  : 1.691 (1.626-1.705) n=193
SSZ : 0.688 (0.671-0.706) n=3
*/


// Kalibračná tabuľka nameraných pomerov (pomer = mV_smerovka / mV_vcc)
// Aktualizované 5.9.2026 podľa reálnej strešnej kalibrácie SITE_TEST_VIDIEK
const WindCalib WindVane::CALIBRATION_TABLE[WindVane::NUM_DIRECTIONS] = {
    {0.737f,   0.0f, "S"},
    {0.604f,  22.5f, "SSV"},
    {1.017f,  45.0f, "SV"},
    {0.476f,  67.5f, "VSV"},
    {0.501f,  90.0f, "V"},
    {0.493f, 112.5f, "VJV"},
    {1.170f, 135.0f, "JV"},
    {0.528f, 157.5f, "JJV"},
    {0.579f, 180.0f, "J"},
    {0.560f, 202.5f, "JJZ"},
    {1.442f, 225.0f, "JZ"},
    {0.791f, 247.5f, "ZJZ"},
    {0.879f, 270.0f, "Z"},
    {0.817f, 292.5f, "ZSZ"},
    {1.691f, 315.0f, "SZ"},
    {0.688f, 337.5f, "SSZ"}
};

WindVane::WindVane(uint8_t pin, uint8_t vccPin)
    : _pin(pin),
      _vccPin(vccPin),
      _instantAngle(0.0f),
      _instantDirName("N/A"),
      _lastRatio(0.0f),
      _sinSum(0.0),
      _cosSum(0.0),
      _sampleCount(0) {
}

void WindVane::begin() {
    analogReadResolution(12);
    resetAggregation();
    Serial.printf("[WindVane] Inicializovaný na ADC1 (Vane: %d, VccRef: %d)\n", _pin, _vccPin);
}

float WindVane::readRatioAveraged(uint8_t samples) {
#ifdef SIMULATE_WIND_SENSORS
#if SIMULATE_WIND_SENSORS
    // Vráti pomer pre nejaký náhodný smer (simulujeme striedanie smerov)
    int simIdx = random(0, NUM_DIRECTIONS);
    return CALIBRATION_TABLE[simIdx].ratio;
#endif
#endif

    if (samples < 4) samples = 4;
    if (samples > 32) samples = 32;

    uint32_t vaneBuf[32];
    uint32_t vccBuf[32];

    for (uint8_t i = 0; i < samples; i++) {
        vaneBuf[i] = analogReadMilliVolts(_pin);
        vccBuf[i] = analogReadMilliVolts(_vccPin);
        delayMicroseconds(50);
    }
    
    // Orezaný priemer (Trimmed Mean): zoradíme a zahodíme 2 najvyššie a 2 najnižšie vzorky.
    // Tým sa dokonale odfiltrujú prechodové zákmity pri pohybe magnetu medzi čipmi.
    std::sort(vaneBuf, vaneBuf + samples);
    std::sort(vccBuf, vccBuf + samples);

    uint8_t trim = (samples >= 8) ? 2 : 1;
    uint32_t vaneSum = 0;
    uint32_t vccSum = 0;
    for (uint8_t i = trim; i < samples - trim; i++) {
        vaneSum += vaneBuf[i];
        vccSum += vccBuf[i];
    }

    if (vccSum == 0) return 0.0f; // Ochrana pred delením nulou
    return (float)vaneSum / (float)vccSum;
}

int WindVane::findClosestDirectionIndex(float measuredRatio) const {
    if (measuredRatio > 1.85f) { // Mŕtva zóna / rozpojený obvod (Pull-up vytiahne na plných ~3.3V / pomer ~2.0)
        return -1;
    }

    float minDiff = 999.0f;
    int bestIdx = 0;

    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        float diff = std::abs(measuredRatio - CALIBRATION_TABLE[i].ratio);
        if (diff < minDiff) {
            minDiff = diff;
            bestIdx = i;
        }
    }
    return bestIdx;
}

void WindVane::update() {
    float ratio = readRatioAveraged(16);
    _lastRatio = ratio;
    int idx = findClosestDirectionIndex(ratio);

    if (idx >= 0) {
        _instantAngle = CALIBRATION_TABLE[idx].angleDeg;
        _instantDirName = CALIBRATION_TABLE[idx].name;

        // Goniometrická akumulácia (vektorový súčet sin a cos uhla)
        double rad = _instantAngle * (M_PI / 180.0);
        _sinSum += std::sin(rad);
        _cosSum += std::cos(rad);
        _sampleCount++;

        // Aktualizácia štatistík pre HW kalibráciu
        DirectionStats& st = _stats[idx];
        if (ratio < st.minRatio) st.minRatio = ratio;
        if (ratio > st.maxRatio) st.maxRatio = ratio;
        st.sumRatio += ratio;
        st.count++;
    }
}

float WindVane::getAveragedAngle() const {
    if (_sampleCount == 0) return _instantAngle;

    double avgSin = _sinSum / _sampleCount;
    double avgCos = _cosSum / _sampleCount;

    double avgRad = std::atan2(avgSin, avgCos);
    double avgDeg = avgRad * (180.0 / M_PI);

    if (avgDeg < 0) {
        avgDeg += 360.0;
    }
    return (float)avgDeg;
}

const char* WindVane::getAveragedDirName() const {
    return angleToDirName(getAveragedAngle());
}

const char* WindVane::angleToDirName(float angleDeg) {
    // 16 smerových sektorov po 22.5°
    float normalized = fmod(angleDeg, 360.0f);
    if (normalized < 0) normalized += 360.0f;

    int sector = (int)floor((normalized + 11.25f) / 22.5f) % 16;
    switch (sector) {
        case 0:  return "S";
        case 1:  return "SSV";
        case 2:  return "SV";
        case 3:  return "VSV";
        case 4:  return "V";
        case 5:  return "VJV";
        case 6:  return "JV";
        case 7:  return "JJV";
        case 8:  return "J";
        case 9:  return "JJZ";
        case 10: return "JZ";
        case 11: return "ZJZ";
        case 12: return "Z";
        case 13: return "ZSZ";
        case 14: return "SZ";
        case 15: return "SSZ";
        default: return "S";
    }
}

void WindVane::resetAggregation() {
    _sinSum = 0.0;
    _cosSum = 0.0;
    _sampleCount = 0;
}

void WindVane::printDebugStats() const {
    Serial.println("=== WindVane Kalibračné Štatistiky (Pomer) ===");
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        const DirectionStats& st = _stats[i];
        if (st.count > 0) {
            float avg = (float)(st.sumRatio / st.count);
            Serial.printf("Smer %-4s (tab %.3f) -> Min: %.3f, Max: %.3f, Avg: %.3f (Vzoriek: %u)\n",
                          CALIBRATION_TABLE[i].name, CALIBRATION_TABLE[i].ratio,
                          st.minRatio, st.maxRatio, avg, st.count);
        }
    }
}

String WindVane::getFormattedStats() const {
    String out = "=== Statistika Pomerov ===\n";
    uint8_t active = 0;
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        const DirectionStats& st = _stats[i];
        if (st.count > 0) {
            active++;
            float avg = (float)(st.sumRatio / st.count);
            char buf[64];
            snprintf(buf, sizeof(buf), "%-4s: %.3f (%.3f-%.3f) n=%u\n",
                     CALIBRATION_TABLE[i].name, avg, st.minRatio, st.maxRatio, st.count);
            out += buf;
        }
    }
    if (active == 0) {
        out += "Zatial 0 vzoriek.\n";
    }
    return out;
}

void WindVane::printLiveDebug() {
    // Odmeriame priemerne hodnoty samostatne pre debug výpis
    uint32_t vaneSum = 0;
    uint32_t vccSum = 0;
    for (uint8_t i = 0; i < 16; i++) {
        vaneSum += analogReadMilliVolts(_pin);
        vccSum += analogReadMilliVolts(_vccPin);
        delayMicroseconds(50);
    }
    
    float ratio = 0.0f;
    if (vccSum > 0) {
        ratio = (float)vaneSum / (float)vccSum;
    }
    
    int idx = findClosestDirectionIndex(ratio);
    
    if (idx >= 0) {
        Serial.printf("[LiveDebug] VANE: %4d mV | VCC_REF: %4d mV | Pomer: %.3f | Smer: %-4s\n", 
            (vaneSum/16), (vccSum/16), ratio, CALIBRATION_TABLE[idx].name);
    } else {
        Serial.printf("[LiveDebug] VANE: %4d mV | VCC_REF: %4d mV | Pomer: %.3f | Mimo rozsah\n", 
            (vaneSum/16), (vccSum/16), ratio);
    }
}
