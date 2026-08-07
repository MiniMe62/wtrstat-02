#include "BleService.h"

class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        Serial.println("[BLE] Klient pripojený!");
    }
    void onDisconnect(BLEServer* pServer) override {
        Serial.println("[BLE] Klient odpojený, spúšťam znova Advertising...");
        pServer->getAdvertising()->start();
    }
};

BleService::BleService()
    : _pServer(nullptr),
      _pCharacteristic(nullptr),
      _deviceConnected(false) {
}

void BleService::begin() {
    BLEDevice::init("wtrStat-02");

    _pServer = BLEDevice::createServer();
    _pServer->setCallbacks(new ServerCallbacks());

    BLEService* pService = _pServer->createService(SERVICE_UUID);

    _pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );

    _pCharacteristic->addDescriptor(new BLE2902());
    _pCharacteristic->setValue("wtrStat-02 Ready");

    pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.println("[BLE] Služba 'wtrStat-02' spustená (Advertising active)");
}

void BleService::updatePayload(const TempSensorManager& tempMgr, const Anemometer& anemometer, const WindVane& windVane, const TimeManager& timeMgr) {
    if (_pCharacteristic == nullptr) return;

    String ts = timeMgr.getFormattedCustom();

    // Formát: "2026-08-06 03:05 CEST | Tin:30.50C,Tout:30.25C,Wind:0.0m/s,Dir:SZ(315°)"
    char payload[140];
    snprintf(payload, sizeof(payload), "%s | Tin:%.2fC,Tout:%.2fC,Wind:%.1fm/s,Dir:%s(%.0f°)",
             ts.c_str(),
             tempMgr.getTempIn(),
             tempMgr.getTempOut(),
             anemometer.getWindSpeed(),
             windVane.getInstantDirName(),
             windVane.getInstantAngle());

    _pCharacteristic->setValue(payload);
    _pCharacteristic->notify();

    Serial.printf("[BLE Broadcast] Sent: %s\n", payload);
}
