#pragma once

#include <Arduino.h>
#include "Secrets.h"

/**
 * @brief Globálna konfigurácia pre wtrStat-02
 */

// Profily staníc pre ochranu dát a nezávislé OTA aktualizácie
#define SITE_TEST_VIDIEK 1  // Testovacia stanica na vidieku (reálne senzory, testovací ThingSpeak)
#define SITE_TEST_MESTO  2  // Testovacia stanica v meste na stole (simulované senzory)
#define SITE_GO85        3  // Ostrá produkcia vidiek
#define SITE_RU48        4  // Ostrá produkcia mesto

// Vyberte aktívny profil pre kompiláciu:
#define CURRENT_SITE SITE_TEST_VIDIEK

#define WTRSTAT_FIRMWARE_VERSION "2.2.0"

namespace Config {
    // Verzia firmvéru a vzdialené aktualizácie
    constexpr const char* FIRMWARE_VERSION = WTRSTAT_FIRMWARE_VERSION;
    constexpr const char* GITHUB_VERSION_URL = "https://raw.githubusercontent.com/MiniMe62/wtrStat-02/main/version.json";
    constexpr bool AUTO_UPDATE_FROM_GITHUB = true;                     // Plne automatický background update bez nutnosti klikania
    constexpr uint8_t AUTO_UPDATE_HOUR = 0;                            // Nočný čas dennej kontroly (00:10)
    constexpr uint8_t AUTO_UPDATE_MINUTE = 10;

    // Sériová komunikácia
    constexpr uint32_t SERIAL_BAUD = 115200;

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

    // Dynamic Cloud Configuration & Prepínače podľa profilu CURRENT_SITE
#if CURRENT_SITE == SITE_TEST_VIDIEK
    constexpr const char* LOC_ID = "TEST_VIDIEK";
    constexpr const char* TS_SERVER = "http://api.thingspeak.com";
    constexpr const char* TS_CHAN_ID = "3205571";
    constexpr const char* TS_API_KEY = "SXG8MK33NKFEA0UX";
    constexpr bool DRY_RUN_UPLOAD = false;                // Ostrý zápis: povolený pre testovací ThingSpeak a Adafruit IO
    constexpr bool ENABLE_THINGSPEAK_UPLOAD = true;       // Zápis do testovacieho ThingSpeak kanálu
    constexpr bool ENABLE_GOOGLE_SHEETS_UPLOAD = false;   // NEZAPISUJE do produkčného Google Sheets
    constexpr bool ENABLE_ADAFRUIT_IO_UPLOAD = true;      // Adafruit IO 1-minútové odosielanie
    #define SIMULATE_WIND_SENSORS false                   // Čítať z reálnych senzorov na vidieku (resp. HW pinov)
    #define SIMULATE_TEMP_SENSORS false                   // Čítať z reálnych teplomerov na vidieku

#elif CURRENT_SITE == SITE_TEST_MESTO
    constexpr const char* LOC_ID = "TEST_MESTO";
    constexpr const char* TS_SERVER = "http://api.thingspeak.com";
    constexpr const char* TS_CHAN_ID = "3205571";
    constexpr const char* TS_API_KEY = "SXG8MK33NKFEA0UX";
    constexpr bool DRY_RUN_UPLOAD = true;                 // Bezpečná simulácia na stole: žiadne reálne odosielanie do cloudu
    constexpr bool ENABLE_THINGSPEAK_UPLOAD = false;      // Vypnuté v meste, aby nekolidovalo s testom z vidieka
    constexpr bool ENABLE_GOOGLE_SHEETS_UPLOAD = false;
    constexpr bool ENABLE_ADAFRUIT_IO_UPLOAD = false;
    #define SIMULATE_WIND_SENSORS false                   // Čítať z reálnych senzorov na stole
    #define SIMULATE_TEMP_SENSORS false                   // Čítať z reálnych teplomerov na stole

#elif CURRENT_SITE == SITE_GO85
    constexpr const char* LOC_ID = "GO85";
    constexpr const char* TS_SERVER = "http://api.thingspeak.com";
    constexpr const char* TS_CHAN_ID = "1554841";
    constexpr const char* TS_API_KEY = "M4SJ7BSW2LR4WQVD";
    constexpr bool DRY_RUN_UPLOAD = false;
    constexpr bool ENABLE_THINGSPEAK_UPLOAD = true;
    constexpr bool ENABLE_GOOGLE_SHEETS_UPLOAD = true;
    constexpr bool ENABLE_ADAFRUIT_IO_UPLOAD = true;
    #define SIMULATE_WIND_SENSORS false
    #define SIMULATE_TEMP_SENSORS false

#elif CURRENT_SITE == SITE_RU48
    constexpr const char* LOC_ID = "RU48";
    constexpr const char* TS_SERVER = "http://api.thingspeak.com";
    constexpr const char* TS_CHAN_ID = "287161";
    constexpr const char* TS_API_KEY = "WEO05BAL45Y3E52D";
    constexpr bool DRY_RUN_UPLOAD = false;
    constexpr bool ENABLE_THINGSPEAK_UPLOAD = true;
    constexpr bool ENABLE_GOOGLE_SHEETS_UPLOAD = true;
    constexpr bool ENABLE_ADAFRUIT_IO_UPLOAD = true;
    #define SIMULATE_WIND_SENSORS false
    #define SIMULATE_TEMP_SENSORS false
#endif

    // Adafruit IO
    constexpr const char* AIO_SERVER = "io.adafruit.com";
    constexpr uint16_t AIO_SERVERPORT = 1883;
    constexpr const char* AIO_USERNAME = "Minime62";
    
    // Automatické zostavenie kompletného Adafruit IO kľúča (ochrana pred scanermi na GitHube)
    inline String getAioKey() {
        String k = SECRET_AIO_KEY;
        if (!k.startsWith("aio_")) {
            return String("aio_") + k;
        }
        return k;
    }

    constexpr const char* AIO_GROUP_TOPIC = "Minime62/groups/meteo/json";
    constexpr const char* AIO_CMD_FEED = "meteo-cmd";                  // Feed pre On-Demand príkazy (napr. UPDATE)

    // Časovanie a intervaly
    constexpr uint32_t MEASURE_INTERVAL_MIN = 15;      // 15-minútový cyklus odosielania (GS, TS)
    constexpr uint32_t MEASURE_INTERVAL_FAST_MIN = 1;  // 1-minútový cyklus odosielania (Adafruit IO)
    constexpr uint32_t SENSOR_READ_INTERVAL_MS = 2000;   // Čítanie senzorov každé 2s
    constexpr uint32_t HALL_REPORT_INTERVAL_MS = 5000;   // Anemometer kĺzavé okno

    // Kalibrácia Anemometra
    constexpr float ANEMOMETER_RADIUS_M = 0.195f;        // Polomer misiek (19.5 cm)
    constexpr float ANEMOMETER_CALIBRATION_FACTOR = 4.0f; // Kalibračný koeficient (odporúčané 3-4+)
    constexpr uint8_t ANEMOMETER_PULSES_PER_REV = 1;

    // Rozlíšenie DS18B20 (10 = 0.25°C)
    constexpr uint8_t TEMP_RESOLUTION_BITS = 10;

    // Kalibrácia a nastavenie TEMT6000 senzora jasu
    constexpr float LIGHT_LOAD_RESISTOR_OHMS = 2000.0f;   // Paralelný záťažový odpor (2 kOhm)
    constexpr uint32_t LIGHT_ADC_ZERO_OFFSET_MV = 142;    // Hardvérový posun ESP32 ADC pri nulovom napätí (142 mV)
    constexpr bool DEBUG_LIGHT_SENSOR = true;              // Zapína periodický výpis hodnôt jasu do Serial monitora

    // Prahové hodnoty osvetlenia (deliace hranice v čistých mV, kde 2800 mV = 100% jas)
    constexpr uint32_t LIGHT_TH_NIGHT_MV = 80;             // Hranica: Noc / Tma (< 80 mV)
    constexpr uint32_t LIGHT_TH_OVERCAST_MV = 300;         // Hranica: Husto zamračené / Dážď (80 - 299 mV)
    constexpr uint32_t LIGHT_TH_CLOUDY_MV = 650;           // Hranica: Zamračené / Sivá obloha (300 - 649 mV)
    constexpr uint32_t LIGHT_TH_SUNNY_MV = 1200;          // Hranica: Polooblačno (650 - 1199 mV) -> Jasno / Priame slnko (>= 1200 mV)

    // Kalibrácia a nastavenie zrážkomera (Tipping Bucket)
    constexpr float RAIN_MM_PER_PULSE = 0.2794f;          // Kalibračný objem misky: 0.2794 mm (0.01 palca) na 1 preklop
    constexpr uint32_t RAIN_DEBOUNCE_MS = 100;           // Hardvérový/softvérový debounce filter pre jazýčkový kontakt (100 ms)
    constexpr bool DEBUG_RAIN_GAUGE = true;               // Debug výpis pri zaznamenaní preklopu

    // Debug prepínače
    constexpr bool DEBUG_ENABLE = true;
    constexpr bool ENABLE_OLED = true;
    constexpr uint32_t OLED_TIMEOUT_MS = 60000;           // Doba svietenia OLED po štarte / dotyku v ms (60000 = 1 min, 0 = stále zapnuté)
    constexpr uint16_t OLED_TOUCH_THRESHOLD = 150;        // Prah citlivosti kapacitného dotyku (hodnota klesá pri dotyku nastavene na 150 - bez dotyku je cca 760)
    constexpr uint32_t OLED_TOUCH_DEBOUNCE_MS = 300;      // Debounce filter proti viacnásobnému dotyku v ms
}

