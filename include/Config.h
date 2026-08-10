#pragma once

#include <Arduino.h>
#include "Secrets.h"

/**
 * @brief Globálna konfigurácia pre wtrStat-02
 */

// Profily staníc pre ochranu historických dát
#define SITE_GO85 1
#define SITE_RU48 2
#define SITE_TEST 3

// Vyberte aktívny profil (SITE_TEST pre bezpečné ladenie)
#define CURRENT_SITE SITE_TEST

namespace Config {
    // Sériová komunikácia
    constexpr uint32_t SERIAL_BAUD = 115200;

    // Bezpečnostný prepínač pre simuláciu odosielania (Dry Run)
    // Ak true: Dáta sa iba vypíšu do sériového monitora, no NEODOSLÚ sa do cloudu.
    constexpr bool DRY_RUN_UPLOAD = false; // Nastavte na false pre odosielanie na testovací kanál

    // Štruktúra pre WiFi prístupový bod
    struct WifiAp {
        const char* ssid;
        const char* pass;
    };

    // Zoznam známych WiFi sietí na skenovanie
    const WifiAp KNOWN_WIFI_NETWORKS[] = {
        {"Go85-2", "Gombos85"},
        {"MiniMe", "Gombos85"},
        {"QNet5", "JankaQietok123"},
        {"QNet2", "JankaQietok123"}
    };
    const uint8_t KNOWN_WIFI_COUNT = sizeof(KNOWN_WIFI_NETWORKS) / sizeof(KNOWN_WIFI_NETWORKS[0]);

    // Google Sheets WebApp URL
    constexpr const char* GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbziQxQvON5aQwpzPSWuCbRWRGeynLqxo1ZiTa5VqQJZV6P6O6icVlHbVcFvC-XzuEDH/exec";

    // Dynamic Cloud Configuration podľa profilu CURRENT_SITE
    // ThingSpeak používa HTTP (http://api.thingspeak.com) pre šetrenie RAM (bez SSL handshaku)
#if CURRENT_SITE == SITE_TEST
    constexpr const char* LOC_ID = "TEST";
    constexpr const char* TS_SERVER = "http://api.thingspeak.com";
    constexpr const char* TS_CHAN_ID = "3205571";
    constexpr const char* TS_API_KEY = "SXG8MK33NKFEA0UX";
#elif CURRENT_SITE == SITE_RU48
    constexpr const char* LOC_ID = "RU48";
    constexpr const char* TS_SERVER = "http://api.thingspeak.com";
    constexpr const char* TS_CHAN_ID = "287161";
    constexpr const char* TS_API_KEY = "WEO05BAL45Y3E52D";
#else // SITE_GO85
    constexpr const char* LOC_ID = "GO85";
    constexpr const char* TS_SERVER = "http://api.thingspeak.com";
    constexpr const char* TS_CHAN_ID = "1554841";
    constexpr const char* TS_API_KEY = "M4SJ7BSW2LR4WQVD";
#endif

    // Adafruit IO
    constexpr const char* AIO_SERVER = "io.adafruit.com";
    constexpr uint16_t AIO_SERVERPORT = 1883;
    constexpr const char* AIO_USERNAME = "Minime62";
    constexpr const char* AIO_KEY = SECRET_AIO_KEY;
    constexpr const char* AIO_GROUP_TOPIC = "Minime62/groups/meteo/json";

    // Časovanie a intervaly
    constexpr uint32_t MEASURE_INTERVAL_MIN = 15;      // 15-minútový cyklus odosielania (GS, TS)
    constexpr uint32_t MEASURE_INTERVAL_FAST_MIN = 1;  // 1-minútový cyklus odosielania (Adafruit IO)
    constexpr uint32_t SENSOR_READ_INTERVAL_MS = 2000;   // Čítanie senzorov každé 2s
    constexpr uint32_t BLE_UPDATE_INTERVAL_MS = 60000;   // BLE broadcast každých 60s
    constexpr uint32_t HALL_REPORT_INTERVAL_MS = 5000;   // Anemometer kĺzavé okno

    // Kalibrácia Anemometra
    constexpr float ANEMOMETER_RADIUS_M = 0.195f;        // Polomer misiek (19.5 cm)
    constexpr float ANEMOMETER_CALIBRATION_FACTOR = 4.0f; // Kalibračný koeficient (odporúčané 3-4+)
    constexpr uint8_t ANEMOMETER_PULSES_PER_REV = 1;

    // Rozlíšenie DS18B20 (10 = 0.25°C)
    constexpr uint8_t TEMP_RESOLUTION_BITS = 10;

    // Debug prepínače
    constexpr bool DEBUG_ENABLE = true;
    constexpr bool ENABLE_OLED = false;
    
    // Simulácia vstupov pre vývoj (bez pripojených senzorov)
    #define SIMULATE_WIND_SENSORS false
}
