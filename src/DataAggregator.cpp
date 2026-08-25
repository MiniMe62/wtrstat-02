#include "DataAggregator.h"
#include <cmath>

static float roundToQuarterStep(float val) {
    if (val <= -55.0f || val >= 125.0f) return val;
    return std::round(val * 4.0f) / 4.0f;
}

DataAggregator::DataAggregator() {
    reset();
}

void DataAggregator::begin() {
    reset();
}

void DataAggregator::reset() {
    _tempInSum = 0.0;
    _tempInCount = 0;

    _tempOutSum = 0.0;
    _tempOutCount = 0;

    _windSpeedSum = 0.0;
    _windSpeedMax = 0.0f;
    _windSpeedCount = 0;

    _lightSum = 0.0;
    _lightCount = 0;
}

void DataAggregator::sample(const TempSensorManager& tempMgr, const Anemometer& anemometer, WindVane& windVane, const LightSensor& lightSensor) {
    if (tempMgr.isReadValid()) {
        _tempInSum += tempMgr.getTempIn();
        _tempInCount++;

        _tempOutSum += tempMgr.getTempOut();
        _tempOutCount++;
    }

    float speed = anemometer.getWindSpeed();
    _windSpeedSum += speed;
    if (speed > _windSpeedMax) {
        _windSpeedMax = speed;
    }
    _windSpeedCount++;

    // Veternú ružicu akumuluje priamo trieda WindVane (goniometricky)
    windVane.update();

    // Jas / intenzita slnečného svitu (v percentách 0 - 100%)
    _lightSum += lightSensor.getBrightnessPercent();
    _lightCount++;
}

WeatherSnapshot DataAggregator::finalizeSnapshot(time_t markTimestamp, const WindVane& windVane) {
    WeatherSnapshot snap;
    snap.timestamp = markTimestamp;

    float rawAvgIn = (_tempInCount > 0) ? (float)(_tempInSum / _tempInCount) : -127.0f;
    float rawAvgOut = (_tempOutCount > 0) ? (float)(_tempOutSum / _tempOutCount) : -127.0f;

    snap.tempIn = roundToQuarterStep(rawAvgIn);
    snap.tempOut = roundToQuarterStep(rawAvgOut);

    snap.windSpeedAvg = (_windSpeedCount > 0) ? (float)(_windSpeedSum / _windSpeedCount) : 0.0f;
    snap.windSpeedMax = _windSpeedMax;

    // Goniometrický vektorový priemer z WindVane
    snap.windDirDeg = windVane.getAveragedAngle();
    snap.windDirName = windVane.getAveragedDirName();

    // Priemerný jas za danú periódu
    snap.light = (_lightCount > 0) ? (float)(_lightSum / _lightCount) : 0.0f;

    snap.isValid = (_tempInCount > 0 || _windSpeedCount > 0 || _lightCount > 0);

    return snap;
}
