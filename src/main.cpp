#include <Arduino.h>
#define _TASK_SLEEP_LINE
#include <TaskScheduler.h>

#include "Config.h"
#include "Pinout.h"
#include "TempSensorManager.h"
#include "Anemometer.h"
#include "WindVane.h"
#include "WifiService.h"
#include "TimeManager.h"
#include "DataAggregator.h"
#include "UploaderService.h"
#include "BleService.h"
#include "WebServerManager.h"

// Objektové inštancie modulov
TempSensorManager tempMgr;
Anemometer anemometer(Pinout::HALL_SENSOR);
WindVane windVane(Pinout::WIND_VANE_PIN);

WifiService wifiService;
TimeManager timeMgr;
DataAggregator aggregator15Min;
DataAggregator aggregator1Min;
UploaderService uploader;
BleService bleService;
WebServerManager webServerMgr;

Scheduler runner;

// Časové razítko poslednej odoslanej 15-minútovej a 1-minútovej záverky
static time_t s_lastUploadedMark15Min = 0;
static time_t s_lastUploadedMark1Min = 0;

// ================= ULOHY TASKSCHEDULER =================

// 1. Čítanie senzorov každé 2 sekundy
void cbSensorRead() {
    tempMgr.update();
    anemometer.update();
    aggregator15Min.sample(tempMgr, anemometer, windVane);
    aggregator1Min.sample(tempMgr, anemometer, windVane);
}

// 2. BLE Advertising aktualizácia každých 60 sekúnd
void cbBleUpdate() {
    bleService.updatePayload(tempMgr, anemometer, windVane, timeMgr);
}

// 3a. Kontrola 15-minútového intervalu každú sekundu
void cbCheckUploadMark15Min() {
    time_t currentMark = timeMgr.getMarkTime(Config::MEASURE_INTERVAL_MIN);

    if (currentMark != s_lastUploadedMark15Min && s_lastUploadedMark15Min != 0) {
        s_lastUploadedMark15Min = currentMark;
        Serial.printf("\n[%s] [MARK] Zistený %d-minútový interval! Spúšťam GS/TS uploader...\n",
                      timeMgr.getFormattedCustom().c_str(), Config::MEASURE_INTERVAL_MIN);

        WeatherSnapshot snap = aggregator15Min.finalizeSnapshot(currentMark, windVane);
            if (snap.isValid) {
                uploader.send15MinSnapshot(snap, timeMgr);
            } else {
                Serial.printf("[%s] [MARK] Dáta zo senzorov nie sú platné, vynechávam upload.\n",
                              timeMgr.getFormattedCustom().c_str());
            }

        aggregator15Min.reset();
        windVane.resetAggregation();
    } else if (s_lastUploadedMark15Min == 0) {
        // Inicializácia pri štarte, aby to neodoslalo hneď v 0. sekunde behu
        s_lastUploadedMark15Min = currentMark;
    }
}

// 3b. Kontrola 1-minútového intervalu pre Adafruit IO
void cbCheckUploadMark1Min() {
    time_t currentMark = timeMgr.getMarkTime(Config::MEASURE_INTERVAL_FAST_MIN);

    if (currentMark != s_lastUploadedMark1Min && s_lastUploadedMark1Min != 0) {
        s_lastUploadedMark1Min = currentMark;
        Serial.printf("\n[%s] [MARK] Zistený %d-minútový interval! Spúšťam AdafruitIO uploader...\n",
                      timeMgr.getFormattedCustom().c_str(), Config::MEASURE_INTERVAL_FAST_MIN);

        WeatherSnapshot snap = aggregator1Min.finalizeSnapshot(currentMark, windVane);
            if (snap.isValid) {
                uploader.send1MinSnapshot(snap, timeMgr);
            }
            
        aggregator1Min.reset();
    } else if (s_lastUploadedMark1Min == 0) {
        s_lastUploadedMark1Min = currentMark;
    }
}

void cbPrintWindDebug() {
    windVane.printDebugStats();
}

void cbPrintLiveWindDebug() {
    windVane.printLiveDebug();
}

// Definovanie úloh TaskScheduler
Task tSensorRead(Config::SENSOR_READ_INTERVAL_MS, TASK_FOREVER, &cbSensorRead);
Task tBleUpdate(Config::BLE_UPDATE_INTERVAL_MS, TASK_FOREVER, &cbBleUpdate);
Task tCheckUpload15Min(1000, TASK_FOREVER, &cbCheckUploadMark15Min);
Task tCheckUpload1Min(1000, TASK_FOREVER, &cbCheckUploadMark1Min);
Task tWindDebug(2000, TASK_FOREVER, &cbPrintWindDebug); // Debug WindVane každých 10 sekúnd (len pre vývoj, vypnúť v produkcii) 
Task tLiveWindDebug(2000, TASK_FOREVER, &cbPrintLiveWindDebug);

void setup() {
    Serial.begin(Config::SERIAL_BAUD);
    delay(1000);
    Serial.println("\n==========================================");
    Serial.println("   ESP32 WeatherStation wtrStat-02 START  ");
    Serial.println("==========================================");

    // Inicializácia senzorických modulov
    tempMgr.begin();
    anemometer.setDebug(true); // Aktivuje debug výpisy každých 5s
    anemometer.begin();
    windVane.begin();
    aggregator15Min.begin();
    aggregator1Min.begin();

    // WiFi a Čas
    bool wifiOk = wifiService.connectBestNetwork();
    timeMgr.begin();

    if (wifiOk) {
        timeMgr.syncNTP();
        webServerMgr.begin(&tempMgr, &anemometer, &windVane, &wifiService, &timeMgr);
    }

    // Inicializácia BLE
    bleService.begin();

    // Pridanie úloh do plánovača TaskScheduler
    runner.init();
    runner.addTask(tSensorRead);
    runner.addTask(tBleUpdate);
    runner.addTask(tCheckUpload15Min);
    runner.addTask(tCheckUpload1Min);
    runner.addTask(tWindDebug); // Len pre vývoj, vypnúť v produkcii
    runner.addTask(tLiveWindDebug); // Len pre vývoj, vypnúť v produkcii

    tSensorRead.enable();
    tBleUpdate.enable();
    tCheckUpload15Min.enable();
    tCheckUpload1Min.enable();
    tWindDebug.enable(); // Len pre vývoj, vypnúť v produkcii
    tLiveWindDebug.enable(); // Len pre vývoj, vypnúť v produkcii

    Serial.printf("[%s] [Setup] Všetky sub-systémy úspešne inicializované!\n\n",
                  timeMgr.getFormattedCustom().c_str());
}

void loop() {
    runner.execute();
    
    // Obsluha HTTP Web Servera (Live Dashboard)
    if (wifiService.isConnected()) {
        webServerMgr.handleClient();
    }
}
