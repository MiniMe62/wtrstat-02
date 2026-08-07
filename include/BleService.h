#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "TempSensorManager.h"
#include "Anemometer.h"
#include "WindVane.h"
#include "TimeManager.h"

// Custom UUIDs pre wtrStat-02 Meteo Službu
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

/**
 * @brief Bluetooth Low Energy (BLE) vysielanie pre mobilnú aplikáciu (nRF Connect)
 */
class BleService {
public:
    BleService();

    void begin();
    void updatePayload(const TempSensorManager& tempMgr, const Anemometer& anemometer, const WindVane& windVane, const TimeManager& timeMgr);

private:
    BLEServer* _pServer;
    BLECharacteristic* _pCharacteristic;
    bool _deviceConnected;
};
