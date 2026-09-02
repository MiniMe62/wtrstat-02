#pragma once

#include <Arduino.h>
#include <TimeLib.h>
#include "Pinout.h"
#include "Config.h"

/**
 * @brief Trieda na meranie zrážok pomocou zrážkomera s preklápacou miskou (Tipping Bucket)
 * Využíva hardvérové prerušenie s debouncingom a sleduje intervalové aj denné sumy.
 */
class RainGauge {
public:
    static constexpr uint8_t HISTORY_MINUTES = 60; // 60 minút histórie pre výpočet intenzity zrážok (Rain Rate mm/h)

    explicit RainGauge(uint8_t pin = Pinout::RAIN_TIPPING_PIN, float mmPerPulse = Config::RAIN_MM_PER_PULSE);

    void begin();
    void update(time_t localTime); // Kontrola zmeny dňa (reset o polnoci) a výpočet intenzity

    // Prístup k nameraným hodnotám
    float getRain15Min() const;        // Zrážky za aktuálny 15-minútový interval (mm)
    float getRain1Min() const;         // Zrážky za poslednú 1 minútu (mm)
    float getRainToday() const;        // Kumulatívne denné zrážky od polnoci (mm)
    float getRainRateMmH() const;      // Okamžitá intenzita dažďa za posledných 60 min (mm/h)
    uint32_t getTotalPulses() const;   // Celkový počet preklopení od štartu
    uint32_t getPulsesToday() const;   // Počet preklopení dnes
    const char* getRainIntensityDescription() const;

    // Resetovacie metódy po odoslaní záverky
    void resetInterval15Min();
    void resetInterval1Min();
    void resetDailyRain();

    // Simulácia a manuálne testovanie
    void simulatePulse();

    void setDebug(bool debug) { _debug = debug; }
    void printLiveDebug() const;

private:
    uint8_t _pin;
    float _mmPerPulse;
    bool _debug;

    // Pulzy v intervaloch
    uint32_t _pulses15Min;
    uint32_t _pulses1Min;
    uint32_t _pulsesToday;

    // Sledovanie zmeny dňa
    int _lastDay;
    uint32_t _lastMinuteCheck;

    // História minútových pulzov pre výpočet okamžitej intenzity (mm/h)
    uint16_t _minuteHistory[HISTORY_MINUTES];
    uint8_t _historyIndex;
    float _currentRainRate;
};
