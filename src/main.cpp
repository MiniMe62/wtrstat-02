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
DataAggregator aggregator;
UploaderService uploader;
BleService bleService;
WebServerManager webServerMgr;

Scheduler runner;

// Časové razítko poslednej odoslanej 15-minútovej záverky (ochrana pred opakovaním v tej istej sekunde)
static time_t s_lastUploadedMark = 0;

// ================= ULOHY TASKSCHEDULER =================

// 1. Čítanie senzorov každé 2 sekundy
void cbSensorRead() {
    tempMgr.update();
    anemometer.update();
    aggregator.sample(tempMgr, anemometer, windVane);
}

// 2. BLE Advertising aktualizácia každých 60 sekúnd
void cbBleUpdate() {
    bleService.updatePayload(tempMgr, anemometer, windVane, timeMgr);
}

// 3. Kontrola 15-minútového intervalu (hh:00:00, hh:15:00, hh:30:00, hh:45:00) každú sekundu
void cbCheckUploadMark() {
    time_t nowUtc = timeMgr.getUtcTime();
    
    // Zistíme lokálne minúty a sekundy
    time_t localT = timeMgr.getLocalTime();
    uint8_t m = minute(localT);
    uint8_t s = second(localT);

    // Ak je presne minute % 15 == 0 a second == 0
    if ((m % Config::MEASURE_INTERVAL_MIN == 0) && (s == 0)) {
        time_t currentMark = timeMgr.getMarkTime();

        if (currentMark != s_lastUploadedMark) {
            s_lastUploadedMark = currentMark;
            Serial.printf("\n[%s] [MARK] Zistený 15-minútový interval (%02d:00)! Spúšťam uploader...\n",
                          timeMgr.getFormattedCustom().c_str(), m);

            // Vytvorenie a odoslanie 15-minútovej záverky
            WeatherSnapshot snap = aggregator.finalizeSnapshot(currentMark, windVane);
            if (snap.isValid) {
                uploader.sendSnapshot(snap, timeMgr);
            } else {
                Serial.printf("[%s] [MARK] Dáta zo senzorov nie sú platné, vynechávam upload.\n",
                              timeMgr.getFormattedCustom().c_str());
            }

            // Reset akumulátorov pre ďalší 15-minútový cyklus
            aggregator.reset();
            windVane.resetAggregation();
        }
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
Task tCheckUpload(1000, TASK_FOREVER, &cbCheckUploadMark);
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
    aggregator.begin();

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
    runner.addTask(tCheckUpload);
    runner.addTask(tWindDebug); // Len pre vývoj, vypnúť v produkcii
    runner.addTask(tLiveWindDebug); // Len pre vývoj, vypnúť v produkcii

    tSensorRead.enable();
    tBleUpdate.enable();
    tCheckUpload.enable();
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
