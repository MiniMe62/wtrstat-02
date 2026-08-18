#pragma once

#include <Arduino.h>

/**
 * @brief Hardvérová definícia GPIO pinov pre ESP32 WeatherStation (wtrStat-02)
 * Zachováva pôvodné zapojenie z projektov wtrStat-01 a AntiWifiTemp22.
 * 
 * =========================================================================================
 * 📡 ODPORÚČANÉ ZAPOJENIE 8 ŽÍL FTP KÁBLA (Cat5e / Cat6) NA STRECHU:
 * =========================================================================================
 * Pár 1 (Oranžový / Bielo-Oranžový):
 *   - Oranžová:        5V VCC (Napájanie pre Hallov senzor anemometra - prevencia mrazov)
 *   - Bielo-Oranžová:  GND (Spoločná zem pre všetky senzory)
 * 
 * Pár 2 (Zelený / Bielo-Zelený):
 *   - Zelená:          Anemometer Hall Out -> GPIO 18 (Open-Collector, Pull-up na 3.3V)
 *   - Bielo-Zelená:    Zrážkomer Reed Out  -> GPIO 4  (Reed kontakt na GND, Pull-up 3.3V)
 * 
 * Pár 3 (Modrý / Bielo-Modrý):
 *   - Modrá:           WindVane ADC        -> GPIO 34 (Odporový delič ružice 0-3.3V)
 *   - Bielo-Modrá:     TEMT6000 Jas ADC    -> GPIO 35 (Senzor jasu/slnečného svitu 0-3.3V)
 * 
 * Pár 4 (Hnedý / Bielo-Hnedý):
 *   - Hnedá:           3.3V VCC (Napájanie pre WindVane, TEMT6000 a DS18B20 #2 Tout)
 *   - Bielo-Hnedá:     DS18B20 Tout Data   -> GPIO 14 (1-Wire dátový vodič zbernice)
 * 
 * Kovové tienenie FTP (Drain wire / Fólia):
 *   - Pripojiť na GND výhradne na strane ESP32 (ochrana pred elektromagnetickým rušením).
 * =========================================================================================
 */
namespace Pinout {
    // Dallas 1-Wire zbernica pre teplotné senzory DS18B20 (Tin a Tout)
    constexpr uint8_t TEMP_SENSOR = 14;

    // Hallov senzor (Anemometer) - GPIO podporujúci prerušenia (ISR)
    constexpr uint8_t HALL_SENSOR = 18;

    // Wind Vane (Veterná ružica s odporovým deličom) - Analógový pin ADC1
    constexpr uint8_t WIND_VANE_PIN = 34;

    // Sledovanie referenčného napätia 3.3V (Ratiometrické meranie pre WindVane) - ADC1
    constexpr uint8_t VCC_MONITOR_PIN = 32;

    // TEMT6000 Senzor slnečného svitu / jasu - Analógový pin ADC1
    constexpr uint8_t LIGHT_SENSOR_PIN = 35;

    // Zrážkomer (Tipping Bucket reed senzor) - GPIO s podporou prerušenia
    constexpr uint8_t RAIN_TIPPING_PIN = 4;

    // I2C zbernica (pre BME280 a 0.9" OLED displej)
    constexpr uint8_t I2C_SDA = 21;
    constexpr uint8_t I2C_SCL = 22;

    // Kapacitný dotykový pin pre prebudenie OLED displeja (ESP32 Touch8 = GPIO 33)
    constexpr uint8_t OLED_TOUCH_PIN = 33;
}
