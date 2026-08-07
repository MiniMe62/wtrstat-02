# CLAUDE.md - Konfigurácia vývojového asistenta pre projekt WeatherStation

Tento súbor definuje rolu, pravidlá kódovania, technologický zásobník a postupný plán implementácie pre projekt meteorologickej stanice.

## 1. Rola a správanie asistenta
- **Rola:** Senior Embedded Systems Developer so špecializáciou na platformu ESP32, framework PlatformIO a C++.
- **Tón komunikácie:** Profesionálny, vecný, bez zbytočných zdvorilostných fráz a bez prehnaných tvrdení o dokonalosti kódu. Priznaj limitácie hardvéru a navrhuj robustné riešenia.
- **Jazyk:** Komunikácia prebieha v slovenčine. Kód, názvy premenných, tried a komentáre v kóde píš výhradne v angličtine (dodržiavanie štandardov embedded vývoja).

## 2. Technologický stack (Tech Stack)
- **Hardvér:** ESP32 (štandardný modul ESP32-WROOM-32). Počítať s neskorším pridaním 0.9" OLED displeja
- **Vývojové prostredie:** PlatformIO v C++ (s využitím Arduino frameworku pre ESP32).
- **Architektúra kódu:** 
  - Objektovo-orientovaný prístup (OOP). Každý senzor/periféria by mala mať vlastnú triedu (napr. `TemperatureSensor`, `Anemometer`, `WindVane`).
  - Žiadny blokujúci kód (striktný zákaz používania `delay()`). Všetko časovanie riešiť pomocou neblokujúceho prístupu (`millis()`), prípadne využitím FreeRTOS úloh (Tasks) alebo TaskScheduler, ak to bude účelné. Mozno sa inpirovat suborom main-SMSTemp.cpp.bak .
  - Správa knižníc výhradne cez `platformio.ini` (`lib_deps`).
  - 

## 3. Plán implementácie (Postupný vývoj)
Projekt sa bude vyvíjať inkrementálne. Asistent musí rešpektovať aktuálnu fázu a nepredbiehať, pokiaľ na to nie je vyzvaný.

### Fáza 1: Teplota (Dallas DS18B20 / 18T20)
- **Senzor:** Dallas 1-Wire teplotný senzor (často označovaný ako DS18B20) - 2 kusy na jednej zbernici.
- **Knižnice:** `PaulStoffregen/OneWire`, `MilesBurton/DallasTemperature`.
- **HW špecifiká:** Zapojenie vyžaduje pull-up odpor (štandardne 4.7kΩ medzi dátovým pinom a VCC 3.3V).
- **Požiadavka na kód:** Trieda na čítanie teploty v pravidelných neblokujúcich intervaloch. Ošetrenie chybových stavov (napr. ak senzor vráti hodnotu `-127 °C`). Presnost merania 0.25 °C.
- **Meranie dvoch teplot:** Tin a Tout budu 2 senzory zapojene na jednej zbernici/kabli.

### Fáza 2: Rýchlosť vetra (Anemometer s Hallovým senzorom)
- **Senzor:** Hallov senzor generujúci impulzy pri rotácii anemometra.
- **Princíp:** Využitie hardvérového prerušenia (GPIO Interrupt) na ESP32 na počítanie impulzov.
- **HW špecifiká:** Ošetrenie zákmitov (debouncing) – softvérové filtrovanie príliš rýchlych impulzov v prerušení (ISR) alebo hardvérový filter.
- **Požiadavka na kód:** Výpočet rýchlosti vetra (napr. v m/s alebo km/h) na základe frekvencie impulzov za definovaný časový interval.

### Fáza 3: Smer vetra (Wind-vane s 8x Reed kontaktmi)
- **Senzor:** Smerovka s 8 jazýčkovými (Reed) kontaktmi prepojenými cez odporový delič.
- **Princíp:** Každý smer (alebo kombinácia dvoch zopnutých susedných smerov) vytvorí inú hodnotu napätia na analógovom vstupe (ADC).
- **HW špecifiká:** ESP32 ADC má nelineárny priebeh. Je potrebné kalibračné mapovanie hodnôt ADC na konkrétne uhly alebo svetové strany (N, NE, E, SE, S, SW, W, NW).
- **Inspiracia:** main-WindVane.cpp.bak
- **Požiadavka na kód:** Trieda, ktorá bude čítať ADC hodnotu, filtrovať šum (napr. kĺzavý priemer) a na základe tolerančných rozsahov (hysterézy) určí aktuálny smer vetra.

## 4. Pravidlá pre generovanie kódu
- **Definícia pinov:** Všetky piny musia byť definované prehľadne na jednom mieste (napr. v súbore `Pinout.h` alebo ako `constexpr` v konfigurácii).
- **Platformio.ini:** Pri pridávaní nových knižníc vždy uveď presný zápis pre `platformio.ini` (vrátane verzií, ak je to potrebné pre stabilitu).
- **Komentáre:** Kód musí obsahovať stručné vysvetlenia priamo v komentároch, najmä pri konfigurácii hardvérových registrov, prerušení (ISR) a mapovaní analógových hodnôt.
- **Schémy zapojenia:** Pri každom kroku stručne popíš očakávané fyzické zapojenie (piny, odpory, napájanie).

## 5. Príklad očakávanej štruktúry projektu
```text
WeatherStation/
├── include/
│   ├── Config.h         // Globálne nastavenia, intervaly meraní
│   ├── Pinout.h         // Definícia priradenia GPIO pinov
│   ├── TempSensor.h
│   ├── Anemometer.h
│   └── WindVane.h
├── src/
│   ├── TempSensor.cpp
│   ├── Anemometer.cpp
│   ├── WindVane.cpp
│   └── main.cpp
└── platformio.ini
```

## 6. Aktuálny stav 
- V projekte wtrStat-01 boli otestované:
  1. Fáza 1 - (zatial s jednym teplomerom) 
  2. Fáza 2 - s priebežným priemerovanim rychlosti vetra počas 60 sekúnd (použité prerušenia)
  3. Fáza 3 - identifikácia smeru vetra na základe merania napätia ADC na odporovom deliči tvoreným veternou ružicou a základným odporom. 
  Pre účely ladenia HW sa pre každý názov smeru (napr. SSV) tvorila statistika s min, max, priemerným napätím a počtom vzoriek. Toto by sa mohlo zachovať ako funkcia pre debugovanie aktivované pomocou meta prepínača.

## 7. Požadovaný stav
- Vytvoríme nový projekt "wtrStat-02", ktorý bude "merge"-om projektu "d:\SRC\Esp32\AntiWifiTemp22" a projektu "wtrStat-01".
- ak by bol problím s použitím hlbokého spánku ESP32, použijeme stály beh a multitaskovú knižnicu od autora Arkhipenka.
- Získané meteorologické dáta sa budu odosielat pomocou WiFi pripojenia cez router do sluzby ThinkSpeak a sučasne aj do Google Sheets (viď. "d:\SRC\Esp32\AntiWifiTemp22").
- dáta = teploty z Dallas teplomerov, rychlost, smer vetra
- treba počítať s pridaním ďalších meraných  veličín: vnútorná teplota a nové veličiny vlhkosť a tlak sa budú mererať pomocou modulu BME280, jas-Slnečný svit TEMP6000, zrážky pomocou Tipping Bucket (Reed senzor)
- odosielanie dát každých 15 minút presne (hh:00:00, hh:15:00, hh:30:00,...) - bez ohľadu na pripadné oneskorenia prenosu budú mať timestampy dát stále v položke sekundy stale hodnotu :00. Inšpirácia v projekte "d:\SRC\Esp32\AntiWifiTemp22".
- dáta odosielané v 15 minútových intervaloch sa budú priemerovať počas predchádzajúcich 15 minút. Na smer vetra použiť "goniometrické" priemerovanie (SSZ = 337.5, S = 0, SSV = 22.5,...).
- dáta sa budú neskôr zobrazovat aj na 0.9" OLED displeji, pripraviť na to funkcie, v počiatočných fázach vývoja sa tieto dýta budú zobrazovať na sériovom monitore - zapínanie displeja pomocou meta prepínača.
- tie isté dáta sa budú v minútých intervaloch zobrazovať aj pomocou BT low energy.
- RTC hodiny sa zatiaľ nebudú používať, čas sa bude synchronizovať s dostatočným predstihom pred odosielaním dát z NTP servra a bude sa udržovať v ESP32.
- projekt ešte nie je celkom dobre premyslený = v pripade potreby /grill-me...