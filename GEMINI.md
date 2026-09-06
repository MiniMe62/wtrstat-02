# Pravidlá pre projekt wtrStat-02

## 1. Automatické navyšovanie verzie firmvéru (x.x.X)
Pri akejkoľvek úprave kódu (aj menších zmenách, opravách bugov alebo vylepšeniach UI/firmvéru) je **povinné vždy navýšiť patch verziu firmvéru**:

1. **`include/Config.h`**:
   - Aktualizovať `#define WTRSTAT_FIRMWARE_VERSION "x.x.X"`
2. **`version.json`**:
   - Zvýšiť verziu pre stanice (napr. `TEST_VIDIEK`, `TEST`) a doplniť stručný popis zmien do `"notes"`.
3. **`docs/index.html`**:
   - Aktualizovať reťazec verzie v päte stránky (`<footer>`).
4. **Zostavenie OTA binárky**:
   - Skompilovať projekt cez PlatformIO (`pio run`).
   - Skopírovať `.pio/build/esp32dev/firmware.bin` do `bin/firmware_TEST_VIDIEK.bin`.
5. **Git a nasadenie**:
   - Commitnúť všetky zmenené súbory vrátane binárky.
   - Pushnúť do vetiev `develop` a `main`, aby sa prejavilo OTA aj GitHub Pages.

## 2. Pravidlo autonómneho dokončenia
Akonáhle používateľ odsúhlasí implementačný plán (alebo zadá požiadavku na implementáciu), dokonči celý cyklus autonómne bez pýtania sa na medzikroky: vykonaj úpravy kódu, navýš verzie, skompiluj projekt, nakopíruj binárku, sprav git commit a push do vetiev develop aj main. Zastav sa a pýtaj sa iba v prípade kritickej chyby, ktorú nevieš sám vyriešiť.