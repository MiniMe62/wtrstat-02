#include <Arduino.h>
#define _TASK_SLEEP_LINE
#include <TaskScheduler.h>

#include "Config.h"
#include "Pinout.h"
#include "TempSensorManager.h"
#include "Anemometer.h"
#include "WindVane.h"
#include "LightSensor.h"
#include "RainGauge.h"
#include "WifiService.h"
#include "TimeManager.h"
#include "DataAggregator.h"
#include "UploaderService.h"
#include "WebServerManager.h"
#include "DisplayManager.h"
#include "CloudOtaService.h"

// Objektové inštancie modulov
TempSensorManager tempMgr;
Anemometer anemometer(Pinout::HALL_SENSOR);
WindVane windVane(Pinout::WIND_VANE_PIN);
LightSensor lightSensor(Pinout::LIGHT_SENSOR_PIN, Config::LIGHT_LOAD_RESISTOR_OHMS);
RainGauge rainGauge(Pinout::RAIN_TIPPING_PIN, Config::RAIN_MM_PER_PULSE);

WifiService wifiService;
TimeManager timeMgr;
DataAggregator aggregator15Min;
DataAggregator aggregator1Min;
UploaderService uploader;
CloudOtaService cloudOta;
WebServerManager webServerMgr;
DisplayManager displayMgr;

Scheduler runner;

// Časové razítko poslednej odoslanej 15-minútovej a 1-minútovej záverky
static time_t s_lastUploadedMark15Min = 0;
static time_t s_lastUploadedMark1Min = 0;

// ================= ULOHY TASKSCHEDULER =================
void cbCloudOtaAutoCheck();

// 1. Čítanie senzorov každé 2 sekundy
void cbSensorRead() {
    tempMgr.update();
    anemometer.update();
    lightSensor.update();
    lightSensor.updateSunshineDuration(timeMgr.getLocalTime());
    rainGauge.update(timeMgr.getLocalTime());
    aggregator15Min.sample(tempMgr, anemometer, windVane, lightSensor, &rainGauge);
    aggregator1Min.sample(tempMgr, anemometer, windVane, lightSensor, &rainGauge);
}

// 2a. Kontrola 15-minútového intervalu každú sekundu
void cbCheckUploadMark15Min() {
    time_t currentMark = timeMgr.getMarkTime(Config::MEASURE_INTERVAL_MIN);

    if (currentMark != s_lastUploadedMark15Min && s_lastUploadedMark15Min != 0) {
        // Počkáme na sekundu :02, aby servery (Adafruit/ThingSpeak) mali svoj čas zaručene po čase značky
        if (second(timeMgr.getUtcTime()) < 2) {
            return;
        }
        s_lastUploadedMark15Min = currentMark;
        Serial.printf("\n[%s] [MARK] Zistený %d-minútový interval! Spúšťam GS/TS uploader...\n",
                      timeMgr.getFormattedCustom().c_str(), Config::MEASURE_INTERVAL_MIN);

        WeatherSnapshot snap = aggregator15Min.finalizeSnapshot(currentMark, windVane, &rainGauge, false);
            if (snap.isValid) {
                uploader.send15MinSnapshot(snap, timeMgr);
            } else {
                Serial.printf("[%s] [MARK] Dáta zo senzorov nie sú platné, vynechávam upload.\n",
                              timeMgr.getFormattedCustom().c_str());
            }

        aggregator15Min.reset();
        windVane.resetAggregation();
        rainGauge.resetInterval15Min();
    } else if (s_lastUploadedMark15Min == 0) {
        // Inicializácia pri štarte, aby to neodoslalo hneď v 0. sekunde behu
        s_lastUploadedMark15Min = currentMark;
    }
}

// 2b. Kontrola 1-minútového intervalu pre Adafruit IO (alebo 5s v kalibračnom režime)
static unsigned long s_lastCalibUploadMs = 0;
static unsigned long s_lastCmdCheckMs = 0;

void cbCheckUploadMark1Min() {
    // 1. Pravidelná kontrola príkazov z Adafruit IO každých 10 sekúnd a update timeoutu
    cloudOta.updateCalibTimeout();
    unsigned long nowMs = millis();
    if (nowMs - s_lastCmdCheckMs >= Config::CALIB_CMD_CHECK_INTERVAL_MS) {
        s_lastCmdCheckMs = nowMs;
        cloudOta.checkAdafruitCommand();
    }

    // 2. Ak beží STREŠNÝ KALIBRAČNÝ REŽIM, odosielame každých 5 sekúnd s okamžitými hodnotami
    if (cloudOta.isCalibMode()) {
        if (nowMs - s_lastCalibUploadMs >= Config::CALIB_UPLOAD_INTERVAL_MS) {
            s_lastCalibUploadMs = nowMs;

            WeatherSnapshot snap;
            snap.timestamp = timeMgr.getUtcTime();
            snap.isValid = true;
            snap.tempIn = tempMgr.isReadValid() ? tempMgr.getTempIn() : 0.0f;
            snap.tempOut = tempMgr.isReadValid() ? tempMgr.getTempOut() : 0.0f;
            snap.windSpeedAvg = windVane.getLastRatio(); // Do rýchlosti zapíšeme pomer pre istotu na obe karty
            snap.windSpeedMax = windVane.getLastRatio(); // Uložíme surový pomer
            snap.windDirDeg = windVane.getInstantAngle();
            snap.windDirName = windVane.getInstantDirName();
            snap.light = lightSensor.getBrightnessPercent();
            snap.rain = rainGauge.getRain1Min();
            snap.rainDaily = rainGauge.getRainToday();

            Serial.printf("[CalibMode] Odosielam na Adafruit IO: Smer %s (%.1f°), Pomer: %.3f (x1000: %d) [Zostáva: %u s]\n",
                          snap.windDirName.c_str(), snap.windDirDeg, snap.windSpeedMax, 
                          (int)round(snap.windSpeedMax * 1000.0f), cloudOta.getCalibRemainingSec());

            uploader.send1MinSnapshot(snap, timeMgr, true);
        }
        return; // V kalibračnom režime preskočíme štandardný 1-minútový snapshot
    }

    // 3. ŠTANDARDNÝ REŽIM (Normal Mode - 1 minúta)
    time_t currentMark = timeMgr.getMarkTime(Config::MEASURE_INTERVAL_FAST_MIN);

    if (currentMark != s_lastUploadedMark1Min && s_lastUploadedMark1Min != 0) {
        // Počkáme na sekundu :02, aby server Adafruit IO mal svoj čas zaručene po čase značky (:00)
        if (second(timeMgr.getUtcTime()) < 2) {
            return;
        }
        s_lastUploadedMark1Min = currentMark;
        Serial.printf("\n[%s] [MARK] Zistený %d-minútový interval! Spúšťam AdafruitIO uploader...\n",
                      timeMgr.getFormattedCustom().c_str(), Config::MEASURE_INTERVAL_FAST_MIN);

        WeatherSnapshot snap = aggregator1Min.finalizeSnapshot(currentMark, windVane, &rainGauge, true);
        if (snap.isValid) {
            uploader.send1MinSnapshot(snap, timeMgr, false);
        }
        
        aggregator1Min.reset();
        rainGauge.resetInterval1Min();

        // 2d. Denná nočná kontrola v presnom čase (00:10)
        static int s_lastDailyOtaDay = -1;
        time_t localT = timeMgr.getLocalTime();
        if (Config::AUTO_UPDATE_FROM_GITHUB &&
            hour(localT) == Config::AUTO_UPDATE_HOUR &&
            minute(localT) == Config::AUTO_UPDATE_MINUTE &&
            s_lastDailyOtaDay != day(localT)) {
            s_lastDailyOtaDay = day(localT);
            Serial.printf("\n[%s] [CloudOTA] Nastal presný čas nočnej kontroly (%02d:%02d)!\n",
                          timeMgr.getFormattedCustom().c_str(), Config::AUTO_UPDATE_HOUR, Config::AUTO_UPDATE_MINUTE);
            cbCloudOtaAutoCheck();
        }
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

void cbPrintLiveLightDebug() {
    if (Config::DEBUG_LIGHT_SENSOR) {
        lightSensor.printLiveDebug();
    }
}

void cbDisplayUpdate() {
    if (Config::ENABLE_OLED) {
        displayMgr.update(tempMgr, anemometer, windVane, lightSensor, wifiService, timeMgr);
    }
}

// 3. Periodická NTP synchronizácia (každú 1 hodinu mimo 15-min záveriek)
void cbNtpPeriodicSync() {
    if (wifiService.isConnected()) {
        Serial.println("\n[NTP] Spúšťam plánovanú hodinovú resynchronizáciu času...");
        timeMgr.syncNTP();
    }
}

// 4. Automatická kontrola aktualizácie z GitHubu
void cbCloudOtaAutoCheck() {
    if (!Config::AUTO_UPDATE_FROM_GITHUB || !wifiService.isConnected()) {
        return;
    }
    Serial.println("\n[CloudOTA] Spúšťam plánovanú kontrolu verzie z GitHubu...");
    OtaCheckResult res = cloudOta.checkVersion();
    if (res.updateAvailable && !res.downloadUrl.isEmpty()) {
        Serial.printf("[CloudOTA] Zistená nová verzia pre stanicu %s: v%s! Spúšťam automatickú inštaláciu...\n",
                      Config::LOC_ID, res.newVersion.c_str());
        cloudOta.performUpdate(res.downloadUrl);
    } else if (res.error.length() > 0) {
        Serial.printf("[CloudOTA] Kontrola verzie: %s\n", res.error.c_str());
    } else {
        Serial.printf("[CloudOTA] Stanica %s je aktuálna (v%s).\n", Config::LOC_ID, res.currentVersion.c_str());
    }
}

// Definovanie úloh TaskScheduler
Task tSensorRead(Config::SENSOR_READ_INTERVAL_MS, TASK_FOREVER, &cbSensorRead);
Task tDisplayUpdate(1000, TASK_FOREVER, &cbDisplayUpdate);
Task tCheckUpload15Min(1000, TASK_FOREVER, &cbCheckUploadMark15Min);
Task tCheckUpload1Min(1000, TASK_FOREVER, &cbCheckUploadMark1Min);
Task tNtpSync(3600000, TASK_FOREVER, &cbNtpPeriodicSync); // Každú 1 hodinu
Task tCloudOtaBootCheck(120000, TASK_ONCE, &cbCloudOtaAutoCheck); // Jednorazová kontrola 2 minúty po štarte
Task tWindDebug(10000, TASK_FOREVER, &cbPrintWindDebug); // Štatistická tabuľka každých 10 sekúnd
Task tLiveWindDebug(2000, TASK_FOREVER, &cbPrintLiveWindDebug); // Živý výpis každé 2 sekundy
Task tLiveLightDebug(2000, TASK_FOREVER, &cbPrintLiveLightDebug);

void setup() {
    Serial.begin(Config::SERIAL_BAUD);
    delay(1000);
    Serial.println("\n==========================================");
    Serial.println("   ESP32 WeatherStation wtrStat-02 START  ");
    Serial.println("==========================================");

    // Inicializácia displeja (ak je povolený v Config.h)
    if (Config::ENABLE_OLED) {
        displayMgr.begin();
    }

    // Inicializácia senzorických modulov
    tempMgr.begin();
    anemometer.setDebug(false); // Dočasne vypnuté pre čistý Serial monitor
    anemometer.begin();
    windVane.begin();
    lightSensor.begin();
    rainGauge.begin();
    aggregator15Min.begin();
    aggregator1Min.begin();

    // WiFi a Čas
    bool wifiOk = wifiService.connectBestNetwork();
    timeMgr.begin();

    if (wifiOk) {
        timeMgr.syncNTP();
    }
    webServerMgr.begin(&tempMgr, &anemometer, &windVane, &wifiService, &timeMgr, &cloudOta, &lightSensor, &rainGauge);

    // Pridanie úloh do plánovača TaskScheduler
    runner.init();
    runner.addTask(tSensorRead);
    runner.addTask(tDisplayUpdate);
    runner.addTask(tCheckUpload15Min);
    runner.addTask(tCheckUpload1Min);
    runner.addTask(tNtpSync);
    runner.addTask(tCloudOtaBootCheck);
    runner.addTask(tWindDebug); // Len pre vývoj, vypnúť v produkcii
    runner.addTask(tLiveWindDebug); // Len pre vývoj, vypnúť v produkcii
    runner.addTask(tLiveLightDebug);

    tSensorRead.enable();
    if (Config::ENABLE_OLED) {
        tDisplayUpdate.enable();
    }
    tCheckUpload15Min.enable();
    tCheckUpload1Min.enable();
    // Spustíme NTP synchronizáciu s posunom 7 minút po štarte (mimo 15-minútových záveriek)
    tNtpSync.enableDelayed(7 * 60 * 1000);
    // Spustíme jednorazovú boot kontrolu GitHubu 2 minúty po štarte (nočná beží o 00:10, on-demand cez Adafruit IO)
    if (Config::AUTO_UPDATE_FROM_GITHUB) {
        tCloudOtaBootCheck.enableDelayed(2 * 60 * 1000);
    }
    tWindDebug.enable();     // Štatistická tabuľka (min/max/avg/počet vzoriek) každých 10s
    tLiveWindDebug.enable(); // Živý výpis napätia a pomeru každé 2s
    if (Config::DEBUG_LIGHT_SENSOR) {
        tLiveLightDebug.enable();
    }

    Serial.printf("[%s] [Setup] Všetky sub-systémy úspešne inicializované!\n\n",
                  timeMgr.getFormattedCustom().c_str());
}

void loop() {
    runner.execute();
    
    // Automatické udržiavanie / obnovenie WiFi na pozadí
    wifiService.ensureConnected();

    // Kontrola dotyku a šetriča OLED displeja
    if (Config::ENABLE_OLED) {
        displayMgr.checkTouchAndTimeout();
    }

    // Obsluha HTTP Web Servera (Live Dashboard)
    if (wifiService.isConnected()) {
        webServerMgr.handleClient();
    }
}
