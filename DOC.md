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
