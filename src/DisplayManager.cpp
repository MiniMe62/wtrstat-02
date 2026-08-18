#include "DisplayManager.h"
#include "driver/touch_sensor.h"

DisplayManager::DisplayManager()
    : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
      _isInitialized(false),
      _isScreenOn(false),
      _screenOnUntil(0),
      _lastTouchCheckTime(0),
      _lastTouchDetectedTime(0),
      _lastDrawTime(0) {
}

bool DisplayManager::begin() {
    if (!Config::ENABLE_OLED) {
        Serial.println("[OLED] Displej je zakázaný v konfigurácii (ENABLE_OLED = false).");
        return false;
    }

    // Inicializácia I2C zbernice na definovaných pinoch (GPIO 21 SDA, GPIO 22 SCL)
    Wire.begin(Pinout::I2C_SDA, Pinout::I2C_SCL);

    // Skúsime najčastejšiu I2C adresu 0x3C
    if (_display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        _isInitialized = true;
        Serial.println("[OLED] SSD1306 128x64 úspešne nájdený a inicializovaný na I2C adrese 0x3C");
    } 
    // Ak neuspeje, skúsime alternatívnu I2C adresu 0x3D
    else if (_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
        _isInitialized = true;
        Serial.println("[OLED] SSD1306 128x64 úspešne nájdený a inicializovaný na I2C adrese 0x3D");
    } else {
        Serial.println("[OLED] CHYBA: SSD1306 displej nebol nájdený na I2C zbernici (skontrolujte SDA=21, SCL=22, napájanie 3.3V/5V a pull-up)!");
        _isInitialized = false;
        return false;
    }

    // Inicializácia dotykového HW radiča cez ESP-IDF
    touch_pad_init();
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_0V);
    int8_t pad = digitalPinToTouchChannel(Pinout::OLED_TOUCH_PIN);
    if (pad >= 0) {
        touch_pad_config((touch_pad_t)pad, 0);
    }

    _isScreenOn = true;
    _screenOnUntil = millis() + Config::OLED_TIMEOUT_MS;

    // Úvodná obrazovka
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setCursor(18, 12);
    _display.println("wtrStat-02 START");
    _display.setCursor(10, 30);
    _display.println("ESP32 Weather Station");
    _display.setCursor(22, 48);
    _display.println("Initializing...");
    _display.display();
    delay(1000);

    Serial.printf("[OLED] Displej inicializovany, touch pad T%d (GPIO %d) aktivny (timeout: %u s).\n",
                  pad, Pinout::OLED_TOUCH_PIN, Config::OLED_TIMEOUT_MS / 1000);

    return true;
}

void DisplayManager::checkTouchAndTimeout() {
    if (!_isInitialized) return;

    uint32_t now = millis();

    // 1. Kontrola kapacitného dotykového pinu každých 100 ms
    if (now - _lastTouchCheckTime >= 100) {
        _lastTouchCheckTime = now;
        uint16_t touchVal = 0;
        int8_t pad = digitalPinToTouchChannel(Pinout::OLED_TOUCH_PIN);
        if (pad >= 0) {
            touch_pad_read((touch_pad_t)pad, &touchVal);
        }

        // Ak je nameraná hodnota pod prahom, zaznamenal sa dotyk (prah > touchVal > 0)
        if (touchVal > 0 && touchVal < Config::OLED_TOUCH_THRESHOLD) {
            if (now - _lastTouchDetectedTime >= Config::OLED_TOUCH_DEBOUNCE_MS) {
                _lastTouchDetectedTime = now;
                Serial.printf("[OLED] Dotyk detegovany na GPIO %d (T%d, hodnota %u < %u) -> Prebudzam displej.\n",
                              Pinout::OLED_TOUCH_PIN, pad, touchVal, Config::OLED_TOUCH_THRESHOLD);
                wake();
            }
        }
    }

    // 2. Kontrola vypršania časovača (ak nie je vypnutý: OLED_TIMEOUT_MS == 0 znamená trvalo zapnutý)
    if (_isScreenOn && Config::OLED_TIMEOUT_MS > 0) {
        if (now >= _screenOnUntil) {
            sleep();
        }
    }
}

void DisplayManager::wake() {
    _screenOnUntil = millis() + Config::OLED_TIMEOUT_MS;
    if (!_isScreenOn && _isInitialized) {
        _display.ssd1306_command(SSD1306_DISPLAYON);
        _isScreenOn = true;
        Serial.printf("[OLED] Dotyk detegovany -> Displej ZAPNUTY na %u s.\n", Config::OLED_TIMEOUT_MS / 1000);
    }
}

void DisplayManager::sleep() {
    if (_isScreenOn && _isInitialized) {
        _display.ssd1306_command(SSD1306_DISPLAYOFF);
        _isScreenOn = false;
        Serial.println("[OLED] Timeout vyprsal -> Displej VYPNUTY (setrenie OLED pixelov).");
    }
}

void DisplayManager::update(const TempSensorManager& tempMgr,
                            const Anemometer& anemometer,
                            const WindVane& windVane,
                            const WifiService& wifiService,
                            const TimeManager& timeMgr) {
    if (!_isInitialized || !_isScreenOn) return;

    _display.clearDisplay();
    _display.setTextWrap(false);
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);

    // === 1. Horná lišta (Status bar) ===
    _display.fillRect(0, 0, 128, 10, SSD1306_WHITE);
    _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    _display.setCursor(2, 1);
    
    // Formátovaný čas HH:MM:SS z TimeManager (vľavo)
    String timeStr = timeMgr.getFormattedTime();
    _display.print(timeStr);

    // WiFi status (zarovnaný doprava, napr. "-65dBm" alebo "No WiFi")
    char wifiBuf[16];
    if (wifiService.isConnected()) {
        snprintf(wifiBuf, sizeof(wifiBuf), "%ddBm", wifiService.getRSSI());
    } else {
        snprintf(wifiBuf, sizeof(wifiBuf), "No WiFi");
    }
    int16_t x1, y1;
    uint16_t w, h;
    _display.getTextBounds(wifiBuf, 0, 0, &x1, &y1, &w, &h);
    _display.setCursor(126 - w, 1);
    _display.print(wifiBuf);

    // === 2. Meteo hodnoty (Telo displeja) ===
    _display.setTextColor(SSD1306_WHITE);

    // Riadok 1: Teploty (Tin a Tout)
    _display.setCursor(0, 14);
    _display.printf("Tin:  %5.2f %cC", tempMgr.getTempIn(), (char)247);

    _display.setCursor(0, 26);
    _display.printf("Tout: %5.2f %cC", tempMgr.getTempOut(), (char)247);

    // Riadok 3: Vietor (Rýchlosť)
    _display.setCursor(0, 39);
    _display.printf("Wind: %4.1f m/s", anemometer.getWindSpeed());

    // Riadok 4: Smer vetra
    _display.setCursor(0, 52);
    _display.printf("Dir:  %-4s (%3.0f%c)", 
                    windVane.getInstantDirName(), 
                    windVane.getInstantAngle(), 
                    (char)247);

    _display.display();
}
