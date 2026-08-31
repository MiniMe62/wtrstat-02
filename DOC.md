# wtrStat-02: Projektová dokumentácia

Tento dokument sumarizuje kľúčové vylepšenia a architektonické rozhodnutia, ktoré boli implementované do softvéru meteostanice so zameraním na spoľahlivosť merania veternej ružice (WindVane).

## 1. Problém s kolísaním napätia a WiFi
Pôvodný kód čítal z veternej ružice (ktorá funguje ako odporový delič) absolútne hodnoty v milivoltoch. 
**Problém:** ESP32 čip pri zapnutí WiFi generuje veľké prúdové špičky, ktoré spôsobujú krátkodobý pokles napätia na 3,3V vetve. Analógový prevodník (ADC) v ESP32 má vlastnú veľmi stabilnú referenciu. Ak napájacie napätie klesne (napr. na 3,1V), výstupné napätie zo smerovky klesne tiež. Prevodník tak nameria nižšiu hodnotu v milivoltoch, hoci sa fyzický smer vetra nezmenil. To spôsobovalo masívne výpadky niektorých smerov a "halucinovanie" (naskakovanie nesprávnych smerov) počas prenosu dát.

## 2. Ratiometrické meranie (Riešenie)
Aby sme sa zbavili vplyvu kolísania napätia, bola implementovaná **Ratiometrická (pomerná) metóda merania**:
* **Referenčný pin (GPIO 32):** Bol pridaný nový hardvérový delič napätia (2x 10k rezistory) priamo z 3,3V vetvy. Z tohto deliča čítame aktuálny stav napájacej vetvy (vydelený na polovicu, aby sme neprekročili rozsah ADC).
* **Pomerný výpočet:** Namiesto ukladania milivoltov sa teraz napätie zo smerovky (GPIO 34) vydelí napätím z referenčného pinu (GPIO 32). 
  `Pomer (Ratio) = mV_Smerovka / mV_Referencia`
* **Výsledok:** Aj keď celé napájacie napätie dosky padne, padne úmerne aj referenčné napätie, aj napätie na smerovke. Ich vzájomný pomer ostane vždy identický. Zabezpečila sa tým dokonalá imunita voči zaťaženiu z WiFi.

## 3. Riešenie mŕtvych zón (Hardvérový problém)
Smerovka obsahuje jazýčkové (Reed) kontakty. Pri rotácii magnetu dochádza na zlomok sekundy (alebo v prípade opotrebovaného kontaktu aj na dlhšie) k strate elektrického spojenia.
Pin 34 zostane "visieť vo vzduchu", chytá šum a napätie padá k nule. V kóde je na to poistka:
Ak nameraný pomer klesne pod `0.05` (5%), kód vyhodnotí stav ako "mŕtvu zónu" a hodnotu pre daný moment ignoruje, čím sa predchádza zapisovaniu nezmyselných smerov do štatistiky.

## 4. Pravidlo používania ADC na ESP32
Na ESP32 je kritické obmedzenie: **ADC2 (GPIO 4, 15, 25, 26, 27, atď.) nie je možné použiť, ak je zapnutá WiFi.**
Preto sú všetky analógové senzory (WindVane, TEMT6000 Jas a VCC Monitor) správne zapojené výhradne na **ADC1** (GPIO 32, 34, 35). Tieto piny fungujú nepretržite aj počas odosielania dát do cloudu.

## 5. Užitočné debuggovacie funkcie
V triede `WindVane` sú dostupné metódy na diagnostiku:
* `printDebugStats()`: Vypíše štatistiku (min, max, priemer) pre každý detegovaný smer za posledný 15-minútový cyklus.
* `printLiveDebug()`: Vypisuje surové hodnoty (`vaneSum`, `vccSum`), výsledný vypočítaný pomer a interpretovaný smer. Tento nástroj slúži na rýchle odhalenie hardvérových chýb (napríklad zlepiaceho sa Reed kontaktu).

## 6. Architektúra ThingSpeak a Cloud odosielania
Vzhľadom na pevný limit 8 numerických polí (Fields) v jednom ThingSpeak kanáli sme zvolili hybridný prístup pre optimalizáciu dát:

### Priradenie polí (Fields 1-8):
Tieto polia sú vyhradené **výhradne pre číselné hodnoty**, z ktorých ThingSpeak dokáže automaticky kresliť čiarové grafy:
1. `Temperature In`
2. `Temperature Out`
3. `Humidity`
4. `Pressure`
5. `Wind Direction` (v stupňoch 0-360)
6. `Wind Speed Avg`
7. `Wind Speed Max`
8. `Rain`

### Pole Status (JSON Payload):
Menej dôležité dáta a najmä textové údaje sa odosielajú zabalené do štruktúrovaného JSON objektu vloženého priamo do poľa `status`. ThingSpeak limituje toto pole na 255 znakov.
Príklad: `status={"dir":"SV/045","light":0.0}`

## 7. Návod: MATLAB Vizualizácia smeru vetra (Štýl SHMÚ)
Keďže do `Field 5` ukladáme smer vetra v stupňoch, môžeme v ThingSpeaku vytvoriť vlastný bodový graf, ktorý stupne prepíše na textové kategórie (S, SV, V...).

**Postup nastavenia v ThingSpeak:**
1. Hore v menu vyberte **Apps** -> **MATLAB Visualizations**.
2. Kliknite na **New** -> **Custom (no starter code)** -> **Create**.
3. Do okna vložte nasledujúci kód (nezabudnite upraviť `channelID` a prípadne `readAPIKey` v záložke API Keys):

```matlab
% 1. Nastavenia kanálu
channelID = 12345678; 
readAPIKey = 'TVOJ_READ_API_KEY'; 

% 2. Načítaj dáta za posledný 1 deň
[data, time] = thingSpeakRead(channelID, 'Field', 5, 'NumDays', 1, 'ReadKey', readAPIKey);

% 2b. Preveď čas z UTC na stredoeurópsky čas (automaticky rieši letný/zimný čas)
time.TimeZone = 'UTC';
time.TimeZone = 'Europe/Bratislava';

% 3. Nakresli bodový graf (červené guličky)
scatter(time, data, 'filled', 'MarkerFaceColor', 'r');

% 4. Y-os: Prepiš čísla na textové skratky svetových strán
yticks([0 45 90 135 180 225 270 315 360]);
yticklabels({'S', 'SV', 'V', 'JV', 'J', 'JZ', 'Z', 'SZ', 'S'});
ylim([0 360]);

% 5. X-os: Značky ukotvené presne na polnoc s krokom 6 hodín (00:00, 06:00, 12:00, 18:00)
ax = gca;
startOfGraph = dateshift(time(1), 'start', 'day');
endOfGraph = dateshift(time(end), 'start', 'day') + days(1);
ax.XAxis.TickValues = startOfGraph : hours(6) : endOfGraph;
xtickformat('dd.MM. HH:mm');
xtickangle(45);

% 6. Kozmetika
title('História smeru vetra');
ylabel('Smer');
grid on;
```
4. Kliknite na **Save and Run**.
5. V sekcii *Display Settings* (úplne dole) zaškrtnite **Add/Edit this visualization to a channel** a vyberte svoj meteo kanál. Graf sa tým pridá na vašu verejnú nástenku.

## 8. Simulácia senzorov pre vývoj (Vane, Anemometer, Teplomery)
Pre umožnenie testovania logiky stanice, spojenia na cloud či webového dashboardu bez nutnosti mať fyzicky pripojené veterné alebo teplotné senzory na stole, sú k dispozícii simulačné režimy:

**Ako to funguje:**
* **Anemometer (`SIMULATE_WIND_SENSORS`):** Každých 5 sekúnd vygeneruje náhodný počet impulzov (náhodný vetrík).
* **WindVane (`SIMULATE_WIND_SENSORS`):** Pri každom meraní vráti náhodný pomer reprezentujúci jeden zo 16 nakalibrovaných smerov, takže sa smer vetra plynule mení.
* **Dallas DS18B20 (`SIMULATE_TEMP_SENSORS`):** Simuluje 2 senzory (vnútorná teplota In ~22 °C a vonkajšia teplota Out ~18 °C) s realistickým náhodným kolísaním v krokoch po 0.25 °C (rozlíšenie 10-bit).

**Ako zapnúť/vypnúť simuláciu:**
V súbore `include/Config.h` na konci súboru upravte makrá:
```cpp
// Zapne generovanie náhodných dát pre senzory
#define SIMULATE_WIND_SENSORS true 
#define SIMULATE_TEMP_SENSORS true

// Vypne simuláciu, kód začne čítať dáta z reálneho hardvéru (POUŽIŤ V PRODUKCII)
// #define SIMULATE_WIND_SENSORS false 
// #define SIMULATE_TEMP_SENSORS false
```
*Pozor: Nezabudnite prepnúť na `false`, keď stanicu umiestnite do exteriéru s reálnymi senzormi!*

## 9. Hardvérové odrušenie Anemometra (Hall Senzor)
Ak sa na anemometri prejavujú anomálie (napr. nereálne rýchlosti vetra 100+ m/s) pri napájaní zo spínaných zdrojov (napr. USB nabíjačky) alebo cez dlhší kábel, ide s najväčšou pravdepodobnosťou o elektromagnetické rušenie (EMI). Keďže interný pull-up rezistor na ESP32 je príliš slabý (~45 kΩ), neudrží signál stabilný a na pin preniká napr. 50 Hz šum.

**Odporúčané hardvérové úpravy (pre Dátový pin anemometra):**
1. **Externý Pull-Up rezistor (Priorita):** Zapojiť rezistor **4.7 kΩ** (alebo v krajnom prípade aj 1 kΩ až 10 kΩ) medzi Dátový pin a napätie **3.3V**. Tento rezistor zabezpečí tvrdú logickú úroveň (HIGH) a eliminuje väčšinu vonkajšieho šumu.
2. **Filtračný kondenzátor (Ideálne doplnenie):** Zapojiť malý keramický kondenzátor (napr. **100 nF**) priamo medzi Dátový pin a GND pre pohltenie rýchlych napäťových špičiek.

## 10. Šetrič OLED displeja a dotykové prebúdzanie (Touch Wake-up)
SSD1306 OLED displej trpí vypaľovaním pixelov (burn-in) pri 24/7 prevádzke. Preto je implementovaný automatický šetrič s kapacitným prebúdzaním:
* **Dotykový pin (Touch8 = GPIO 33):** Na tento pin stačí pripojiť kúsok vodiča, skrutku na krabičke alebo plôšku z medi/hliníka.
* **Časovač zhasnutia (`OLED_TIMEOUT_MS` v `Config.h`):** Po štarte alebo dotyku svieti displej 60 sekúnd (nastaviteľné), potom sa automaticky vypne cez hardvérový príkaz `SSD1306_DISPLAYOFF`.
* **Minimálna spotreba & I2C úspora:** Po zhasnutí displeja sa zastaví aj zbytočné prekresľovanie cez I2C zbernicu. Dotyk okamžite obnoví zobrazenie (`SSD1306_DISPLAYON`).

## 11. Časovanie a bezpečná NTP synchronizácia (1× za hodinu s offsetom)
Dlhodobý nepretržitý beh bez reštartu vyžaduje korekciu prirodzeného driftu interného kryštálu ESP32 (~2 až 5 sekúnd za pár dní).
* **Hodinová synchronizácia (`tNtpSync`):** Úloha v `TaskScheduler` beží s periodicitou **1 hodina (3 600 000 ms)**.
* **Časové odsadenie (+7 minút po štarte):** Synchronizácia je posunutá o 7 minút (`enableDelayed(7 * 60 * 1000)`), takže prebieha napr. v minútach `XX:07`. Tým sa úplne predchádza sieťovej kolízii s 15-minútovými záverkami (`:00, :15, :30, :45`).
* **Zaručenie presného času `:00` na cloudoch (Adafruit IO, ThingSpeak, Google Sheets):**
  * Uploader sa spúšťa v sekunde **`:02`** novej periódy a odosiela záverku s presným timestampom `:00` sekúnd.
  * Týmto 2-sekundovým posunom sa garantuje, že cloudové servery (najmä Adafruit IO) nikdy neodmietnu balíček chybou *"data created_at may not be in the future"*, a zároveň v databázach nezostávajú náhodné sekundy spôsobené sieťovým oneskorením.

## 12. Robustné neblokujúce pripojenie k WiFi (`WiFiMulti`)
* Pôvodné synchrónne volanie `WiFi.scanNetworks()` v `setup()` mohlo pri rušení zablokovať štart celého systému a TaskScheduleru.
* Prechodom na **`WiFiMulti`** si ESP32 automaticky a neblokujúco manažuje pripojenie k najsilnejšej známej sieti (zo zoznamu `KNOWN_WIFI_NETWORKS`).
* Stanica má nastavený tvrdý limit (6s timeout) – ak WiFi nie je dostupné, stanica plynule pokračuje v offline režime (lokálny zber, OLED, senzory) a pokusy o pripojenie opakuje na pozadí v `loop()`.

## 13. Filtrovanie plauzibilných hodnôt (Firmvér & Web)
Viacúrovňová ochrana pred zápisom chybných / rušivých dát:
* **Teploty (DS18B20):** Akceptovaný rozsah **`-40.0 °C` až `+60.0 °C`**. Automaticky sa zahadzujú hodnoty `-127.0 °C` (odpojený senzor) a `+85.0 °C` (neinicializovaný register Dallas pred prvou konverziou).
* **Rýchlosť vetra (Anemometer):** Priemerná rýchlosť je obmedzená na max **`40.0 m/s` (~144 km/h)**, náraz na max **`45.0 m/s`**, čo spoľahlivo eliminuje nočné špičky z elektromagnetického rušenia.

## 14. Webový Dashboard & GitHub Pages
Integrovaný webový dashboard (dostupný lokálne na IP adrese stanice a verejne cez GitHub Pages v priečinku `docs/`):
* **Vycentrovaná Veterná Ružica (Polar Area Chart):** Sever (S) je striktne zarovnaný vertikálne na 12:00 (`startAngle: -11.25`) s percentuálnym vyjadrením početnosti v 16 smeroch.
* **Graf Rýchlosti a Nárazov Vetra (ALADIN / SHMÚ štýl):**
  - **Kombinovaný graf (Line + Bar):** Čiara zobrazuje plynulý priebeh priemernej rýchlosti vetra (`Field 6`), zatiaľ čo vertikálne stĺpce znázorňujú nárazy vetra (`Field 7 - GUST`).
  - **Inteligentné filtrovanie nárazov:** Stĺpce nárazov nevystupujú pri každom malom zakolísaní, ale len pri výraznejších nárazoch (predvolene náraz $> 1.3\times$ priemeru a aspoň $+1.0\text{ m/s}$).
  - **Režimy filtrovania nárazov:** Prepínanie jedným klikom: *Výrazné (>1.3×)*, *Silné (>1.5×)*, *Všetky nárazy*, *Skryť nárazy*.
  - **Časové horizonty:** Možnosť voľby *⚡ Živé*, *📅 24h*, *📆 3d*, *🗓️ 7d* s priamym načítaním z ThingSpeak API.
  - **Štatistické karty:** Prehľad aktuálnej rýchlosti, priemeru za vybrané obdobie, maxima nárazu (v $\text{m/s}$ aj $\text{km/h}$) a celkového počtu zaznamenaných nárazov.
* **Časový Vývoj Smeru Vetra (Timeline):** Bodový priebeh stáčania smeru vetra v čase s farebným škálovaním podľa sily vetra (modrá = vánok, zelená = mierny, žltá = čerstvý, červená = silný vietor).
* **Interaktívny Teplotný Graf:** Priebeh $T_{in}$ a $T_{out}$ s kubickým Bézierovým vyhladzovaním, prepínaním pohľadov (`Obe`, `Tin`, `Tout`) a automatickým prispôsobením citlivosti Y-osi.
* **Prepínač bubliniek (`💬 Bubliny ZAP/VYP`):** Špeciálne tlačidlo v záhlaví každého grafu optimalizované pre smartfóny umožňujúce jedným ťuknutím skryť/zobraziť tooltip bublinu, aby na mobile neprekrývala krivky.

## 15. Pamäťová architektúra, Partície a Web OTA Aktualizácie

Táto sekcia podrobne popisuje rozdelenie flash pamäte ESP32, optimalizácie pre zmenšenie firmvéru, princíp bezpečného Dual-Bank OTA a kompletný návod na aktualizáciu firmvéru v teréne (z mobilu cez USB OTG aj bezdrôtovo cez web).

---

### 15.1 Rozdelenie Flash pamäte ESP32 (4 MB) a Partičné schémy

Čip ESP32 (ESP32 Dev Module) má integrovanú flash pamäť s celkovou veľkosťou **4 MB (4 194 304 B)**. Pamäť nie je jednoliaty blok, ale je riadená tzv. **partičnou tabuľkou (Partition Table)**:

```
+-----------------------------------------------------------------------------------+
| Bootloader | Partition Table | NVS (Nastavenia) | OTA Data |   App0   |   App1    |
|   (32 KB)  |     (4 KB)      |     (20 KB)      |  (8 KB)  | (1.92MB) | (1.92MB)  |
+-----------------------------------------------------------------------------------+
```

#### Porovnanie partičných schém v `platformio.ini`:

| Parameter | Schéma `huge_app.csv` (Pôvodná) | Schéma `min_spiffs.csv` (Aktuálna / OTA) |
| :--- | :--- | :--- |
| **Počet App slotov** | **1 slot** (iba `app0`) | **2 nezávislé sloty** (`app0` a `app1`) |
| **Kapacita pre kód** | **3.14 MB (3 276 800 B)** | **2× 1.92 MB (1 966 080 B)** |
| **Podpora bezdrôtového OTA** | ❌ **Nie** (nie je kam stiahnuť nový kód) | ✅ **Áno** (plne bezpečný duálny update) |
| **SPIFFS súborový systém** | Žiadny | Minimálny (192 KB) |
| **NVS (Ukladanie dát)** | 20 KB | 20 KB |

---

### 15.2 Pamäťová optimalizácia (Odstránenie BLE)

* **Problém s veľkosťou:** Pôvodný firmvér s integrovanou knižnicou `ESP32 BLE Arduino` (oficiálny Bluedroid stack) zaberal až **1.75 MB (1 747 741 B)**. V schéme `min_spiffs.csv` s 1.92 MB slotom to predstavovalo vyťaženie až 89 % a nechávalo rezervu len ~170 KB.
* **Riešenie:** Keďže stanica funguje primárne cez WiFi (lokálny WebServer, ThingSpeak, Adafruit IO, Google Sheets) a konfiguráciu parametrov je jednoduchšie realizovať cez webové rozhranie než cez špeciálnu BLE aplikáciu, BLE služba bola z hlavnej vetvy odstránená.
* **Výsledok:**
  - Veľkosť firmvéru klesla na **966 KB (966 421 B)**.
  - Využitie 1.92 MB slotu je **iba 49.2 %**.
  - **Rezerva:** K dispozícii zostáva **takmer 1 MB voľného miesta** (vyše 50 % kapacity slotu) pre akékoľvek budúce rozširovanie, nové grafy, výpočty či senzory.
* **Bezpečná záloha pôvodného BLE kódu:** Pôvodný kód so všetkými BLE funkciami je archivovaný v samostatnej git vetve:
  `git checkout backup/develop-with-ble`

---

### 15.3 Ako funguje bezpečný Dual-Bank OTA mechanizmus

ESP32 využíva **Dual-Bank (dvoj-slotovú) architektúru**, ktorá vylučuje znefunkčnenie ("bricknutie") zariadenia:

1. **Beh z aktívneho slotu:** Ak stanica momentálne beží napr. zo slotu `app0`, webový server prijíma nový súbor `firmware.bin` a zapisuje ho výhradne do neaktívneho slotu `app1`.
2. **Integrita a kontrolný súčet:** Počas nahrávania ESP32 priebežne overuje CRC/MD5 hash dát.
3. **Prepnutie ukazovateľa (`otadata`):** Až po úspešnom prijatí 100 % súboru a validácii hlavičky zapíše zavádzač do partície `otadata` informáciu, že pri najbližšom štarte má bootloader spustiť slot `app1`.
4. **Ochrana pri zlyhaní spojenia:** Ak počas nahrávania vypadne WiFi, vybije sa telefón alebo je súbor poškodený, zápis do `otadata` sa **nevykoná**. Stanica sa reštartuje a bezpečne naštartuje pôvodný, stále plne funkčný firmvér zo slotu `app0`.

---

### 15.4 Návod na aktualizáciu v teréne (z mobilu)

Pre aktualizáciu stanice na vidieku nepotrebujete notebook s vývojovým prostredím.

#### Scenár A: Prvotná inštalácia z mobilu cez USB OTG kábel
*(Nutné vykonať len raz pri prechode z `huge_app` na `min_spiffs` partície, alebo pri obnove po havárii)*

1. **Príprava súboru:** Na PC skompilujte projekt (`pio run`). Vytvorený binárny súbor nájdete v:
   `.pio/build/esp32dev/firmware.bin`
   Tento súbor si pošlite do mobilu (Google Drive, e-mail, lokálne stiahnutie).
2. **Prepojenie:** Prepojte ESP32 s Android telefónom pomocou USB-C / micro-USB OTG kábla/redukcie.
3. **Flashovacia aplikácia v mobile:**
   * **Možnosť 1 (Android Appka):** Nainštalujte z Google Play napríklad **ESP32 Loader** alebo **DroidProg**. V aplikácii povoľte USB prístup, vyberte súbor `firmware.bin`, zvoľte prenosovú rýchlosť `115200` alebo `921600` a kliknite na **Flash**.
   * **Možnosť 2 (Web Serial v prehliadači Chrome pre Android):** Otvorte webový nástroj [ESP Web Tools](https://esphome.github.io/esp-web-tools/) priamo v prehliadači Chrome, kliknite na *Connect*, vyberte pripojený USB port a nahrajte `firmware.bin`.

---

#### Scenár B: Pravidelné bezdrôtové aktualizácie cez Web OTA (Bez káblov)
*(Využiteľné pre všetky ďalšie updaty po nahraní OTA firmvéru)*

1. **Príprava:** Súbor `firmware.bin` máte stiahnutý v mobile.
2. **Pripojenie na sieť:** Pripojte mobil k rovnakej WiFi sieti (alebo mobilnému hotspotu), na ktorú je pripojená meteostanica.
3. **Otvorenie OTA rozhrania:**
   * V prehliadači na mobile otvorte adresu:
     `http://<IP_ADRESA_ESP32>/update`
     *(Alebo kliknite na odkaz `⚙️ OTA Update` v pätičke hlavného dashboardu).*
4. **Nahratie firmvéru:**
   * Kliknite na rámček **"📁 Kliknite pre výber súboru .bin"** a vyberte súbor `firmware.bin` z úložiska mobilu.
   * Kliknite na tlačidlo **"🚀 Spustiť aktualizáciu"**.
   * Na obrazovke sa zobrazí modrý animovaný ukazovateľ priebehu s percentami (prenos trvá približne 5 až 10 sekúnd).
5. **Automatický reštart:**
   * Po dokončení sa zobrazí zelené potvrdenie *"Úspešne nahraté!"*.
   * ESP32 sa automaticky reštartuje s novým firmvérom a prehliadač vás po 12 sekundách automaticky presmeruje späť na hlavný dashboard `/`.

---

### 15.5 Ako sa kedykoľvek vrátiť k jednej veľkej partícii (`huge_app.csv`)

Ak by ste v budúcnosti potrebovali celú pamäť (~3.14 MB) pre jednu obrovskú aplikáciu bez podpory OTA:
1. V súbore `platformio.ini` upravte konfiguráciu:
   ```ini
   ; Prepnite komentár na huge_app.csv:
   ; board_build.partitions = min_spiffs.csv
   board_build.partitions = huge_app.csv
   ```
2. Pripojte ESP32 cez USB kábel a nahrajte firmvér štandardným príkazom `pio run --target upload`.

---

### 15.6 Vzdialené aktualizácie cez GitHub a Selektovanie staníc (`version.json`)

Pre plne vzdialenú správu staníc bez nutnosti byť fyzicky na mieste:

#### 1. Selektovanie staníc cez `version.json`:
V koreňovom priečinku repozitára (alebo na GitHub Pages / Releases) sa nachádza konfiguračný súbor `version.json`. Každá stanica (`GO85` pre vidiek, `RU48` pre mesto, `TEST` pre laboratórium) má v tomto súbore vlastnú sekciu s číslom verzie a URL adresou binárky:

```json
{
  "GO85": {
    "version": "2.0.1",
    "firmware_url": "https://raw.githubusercontent.com/MiniMe62/wtrStat-02/main/bin/firmware_GO85.bin",
    "notes": "Oprava kalibrácie veternej ružice na vidieku"
  },
  "RU48": {
    "version": "2.0.0",
    "firmware_url": "https://raw.githubusercontent.com/MiniMe62/wtrStat-02/main/bin/firmware_RU48.bin",
    "notes": "Pôvodná stabilná verzia pre mesto"
  },
  "TEST": {
    "version": "2.0.1",
    "firmware_url": "https://raw.githubusercontent.com/MiniMe62/wtrStat-02/main/bin/firmware_TEST.bin",
    "notes": "Testovacie zostavenie"
  }
}
```

#### 2. Ako funguje selektívny update:
* Každé ESP32 pri overovaní porovnáva svoju internú verziu `Config::FIRMWARE_VERSION` s verziou uvedenou **výhradne pre svoje `Config::LOC_ID`** v súbore `version.json`.
* Ak zmeníte verziu na `2.0.1` len v sekcii `"GO85"`, stanica na vidieku (`GO85`) okamžite rozpozná dostupný update, zatiaľ čo stanica v meste (`RU48`) zostane nedotknutá.

#### 3. Postup spustenia z webového rozhrania (Na vyžiadanie):
1. Otvorte stránku `http://<IP_STANICE>/update`.
2. Kliknite na **"🔎 Skontrolovať novú verziu na GitHube"**.
3. ESP32 načíta `version.json` priamo z GitHubu a zobrazí poznámky k vydaniu.
4. Kliknutím na **"🚀 Inštalovať z GitHubu"** si ESP32 samo stiahne `.bin` súbor, zapíše ho do záložnej OTA partície a reštartuje sa.

#### 4. Plne autonómna aktualizácia na pozadí (Bez klikania):
* **Konfigurácia (`Config.h`):** `AUTO_UPDATE_FROM_GITHUB = true`.
* **Časovanie:** Úloha v `TaskScheduler` sa prvýkrát spustí 2 minúty po štarte stanice (jednorazovo) a následne **každú noc presne o 00:10** zosynchronizovane cez NTP čas.
* **Priebeh:** ESP32 sa v tichosti spojí s GitHubom. Ak zistí novú verziu pre svoje `LOC_ID`, samo si stiahne firmvér, preflashuje sa a reštartuje. Vy iba nahráte nový súbor na GitHub a zmeníte verziu v `version.json`.

---

## 16. Profily staníc a Oddelenie testovacích/ostrých dát

V `include/Config.h` je zavedený systém profilov cez `#define CURRENT_SITE`:

| Profil | `LOC_ID` | Účel | ThingSpeak Kanál | Google Sheets | Adafruit IO | Senzory |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `SITE_TEST_VIDIEK` | `TEST_VIDIEK` | Test na vidieku | Testovací (`3205571`) | ❌ Vypnuté | ✅ Zapnuté | Reálne senzory |
| `SITE_TEST_MESTO` | `TEST_MESTO` | Vývoj na stole | Testovací (`3205571`) | ❌ Vypnuté | ❌ Vypnuté | Simulované |
| `SITE_GO85` | `GO85` | **Ostrá produkcia vidiek** | Ostrý (`1554841`) | ✅ Zapnuté | ✅ Zapnuté | Reálne senzory |
| `SITE_RU48` | `RU48` | **Ostrá produkcia mesto** | Ostrý (`287161`) | ✅ Zapnuté | ✅ Zapnuté | Reálne senzory |

### Bezpečnostná poistka `DRY_RUN_UPLOAD`:
- Ak je `DRY_RUN_UPLOAD = true`, dáta sa iba vypisujú do sériového monitora a **neodošlú sa do žiadneho cloudu** (ochrana pred poškodením databázy pri ladení).
- Pre bežnú a ostrú prevádzku musí byť `DRY_RUN_UPLOAD = false`.

---

## 17. On-Demand OTA cez Adafruit IO a Nočná kontrola (00:10)

Systém podporuje bleskový update na diaľku bez čakania na 24h periódu:

### 1. Ako funguje On-Demand príkaz:
1. V Adafruit IO dashboarde máte tlačidlo priradené k feedu **`meteo-cmd`**.
2. Keď na mobile/PC prepnete tlačidlo na **`UPDATE`**:
   - ESP32 pri najbližšom 1-minútovom spojení zachytí príkaz.
   - **Reset:** ESP32 ihneď zapíše do feedu `meteo-cmd` hodnotu `IDLE` (ochrana pred zacyklením po reštarte).
   - **Porovnanie verzie:** Stiahne `version.json`. Ak je verzia novšia, stiahne `.bin` a preflashuje sa. Ak je verzia rovnaká, operáciu bezpečne vynechá.
   - **Rýchlosť:** Reakcia nastane maximálne do **60 sekúnd** od kliknutia.

### 2. Ochrana kľúčov pred bezpečnostnými robotmi:
- Kľúč Adafruit IO je v `include/Secrets.h` uložený bez prefixu `aio_` (iba 28 znakov).
- Funkcia `Config::getAioKey()` ho za behu poskladá. Vďaka tomu roboty na GitHube kľúč v `.bin` súboroch nezachytia a kľúč sa **nezablokuje**.

---

## 18. Sprievodca prechodom na Ostrú Produkciu (Checklist)

Akonáhle je hardvér na vidieku doladený a chcete stanicu prepnúť na plnú ostrú prevádzku:

### Krok 1: Prepnutie profilu v kóde
V súbore `include/Config.h` zmeňte:
```cpp
// Prepnúť z testovacieho na ostrý profil:
#define CURRENT_SITE SITE_GO85   // pre ostrý vidiek (alebo SITE_RU48 pre ostré mesto)
```

### Krok 2: Konfigurácia ThingSpeak (Ostré kanály)
Ostré kanály majú v `Config.h` preddefinované parametre:
- **Vidiek (GO85):** Kanál `1554841`, Write API Key `M4SJ7BSW2LR4WQVD`
- **Mesto (RU48):** Kanál `287161`, Write API Key `WEO05BAL45Y3E52D`
- *Uistite sa, že na ThingSpeaku máte v kanáli aktivovaných všetkých 8 polí (Fields 1-8).*

### Krok 3: Konfigurácia Adafruit IO (Ostrá prevádzka)
1. V Adafruit IO overte existenciu skupiny **`meteo`** a feedov:
   - `tempin`, `tempout`, `humidity`, `pressure`, `wind-speed-avg`, `wind-speed-max`, `wind-direction`, `light`, `rain`
   - `meteo-cmd` (pre On-Demand OTA tlačidlo)
2. Kľúč majte zadaný v `include/Secrets.h` (bez `aio_` prefixu).

### Krok 4: Konfigurácia Google Sheets (Keď budete pripravený)
V priečinku `google_script/Code.gs` je pripravený skript pre automatické ukladanie:
1. Otvorte novú Google Tabuľku na [sheets.google.com](https://sheets.google.com).
2. V menu kliknite na **Rozšírenia (Extensions)** $\rightarrow$ **Apps Script**.
3. Skopírujte obsah súboru [`google_script/Code.gs`](file:///d:/SRC/Esp32/wtrStat-02/google_script/Code.gs) do editora.
4. Kliknite na **Nasadiť (Deploy)** $\rightarrow$ **Nové nasadenie (New deployment)**:
   - Typ: **Webová aplikácia (Web app)**
   - Spustiť ako (Execute as): **Ja (Me)**
   - Kto má prístup (Who has access): **Ktokoľvek (Anyone)**
5. Skopírujte vygenerovanú **URL webovej aplikácie** a vložte ju do `include/Config.h`:
   ```cpp
   constexpr const char* GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/TVOJ_NOVY_ID/exec";
   ```

### Krok 5: Nasadenie cez OTA na diaľku
1. Skompilujte projekt s profilom `SITE_GO85` (`pio run`).
2. Skopírujte `.pio/build/esp32dev/firmware.bin` do `bin/firmware_GO85.bin`.
3. V `version.json` zvýšte verziu pre `"GO85"`.
4. Spravte `git push`.
5. V Adafruit IO stlačte tlačidlo **`UPDATE`** $\rightarrow$ stanica na vidieku do 60s prejde z testovacieho profilu na ostrú produkciu.

---

## 19. Senzor intenzity osvetlenia a slnečného svitu (TEMT6000 / ALS-PT19)

Do stanice bol úspešne integrovaný analógový senzor okolitého svetla a detekcie slnečného svitu.

### 1. Hardvérové špecifiká a otočené zapojenie (Breakout PCB):
* **Súčiastka:** Everlight ALS-PT19-315C (puzdro SMD 1206 na fialovom breakout module).
* **Zistená polarita:** Moduly z AliExpressu majú často prehodenú orientáciu potlače (`S`, `G`, `V`) voči internému zapojeniu čipu.
* **Správne overené zapojenie do ESP32:**
* *Senzor bol vložený do puzdra LED žiarovky. Vyvedený 4 žilový káblik, farby sú uvedené pri Pinoch*
  * **Pin `S`:** Pripojený na **+3.3V** (napájanie). **HNEDÝ**
  * **Pin `G`:** Pripojený na **GND** (zem). **ZELENÝ**
  * **Pin `V`:** Výstup signálu pripojený na **GPIO 35 (ADC1)** + **paralelný zaťažovací odpor $20\text{ k}\Omega$ do GND**.  **ŽLTÝ**
* **Voľba odporu $20\text{ k}\Omega$:** 
  * Zabezpečuje bleskový pád na $0\text{ V}$ v tme bez plávania náboja.
  * Na priamom poludňajšom letnom slnku generuje napätie cca **$2.80\text{ V}$** ($93\,\%$ rozsahu ESP32), čím využíva maximum citlivosti a nehrozí presýtenie (clipping).

### 2. Softvérové spracovanie (`LightSensor`):
* **50 Hz AC Filter:** Pre elimináciu pulzovania umelého osvetlenia (sieťová frekvencia 50 Hz / 100 Hz blikanie žiariviek a LED) sa robí 20 vzoriek s odstupom 1 ms (celé 20 ms AC okno).
* **Kompenzácia nulového posunu ESP32:** Odpočítava hardvérový offset $142\text{ mV}$ na vstupe GPIO 35 (`LIGHT_ADC_ZERO_OFFSET_MV`).
* **Sledovanie slnečného svitu (Sunshine Duration - Campbell-Stokes princíp):**
  * Každú minútu, kedy je obloha vyhodnotená ako `isDirectSun()` ($\ge 70\,\%$ jasu), sa pripočíta 1 minúta svitu.
  * O polnoci sa počítadlo automaticky zaznamená a vynuluje pre nový deň (`updateSunshineDuration()`).

### 3. Zobrazenie a výstupy:
* **OLED Displej (128x64):** Zobrazuje jas v %, denný svit (napr. `5h 12m`), textový stav oblohy a dynamický spodný progress-bar intenzity svetla.
* **Lokálny Web Dashboard (HTTP):** Samostatná karta `Jas a Slnko`, farebný odznak stavu oblohy a riadky v detailnej tabuľke.
* **JSON API (`/api/live`):** `lightPercent`, `lightMv`, `estimatedLux`, `skyCondition`, `sunshineDuration`, `isDirectSun`.
* **ThingSpeak & Adafruit IO:** Odosielanie priemernej hodnoty jasu v 1-minútových a 15-minútových záverkách.




