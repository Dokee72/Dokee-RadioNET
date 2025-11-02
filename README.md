# Dokee-RadioNET
(Saját tervezésű Internet Rádió-m)

ESP32 WROOM, SSD1306 OLED (I2C), Rotary Encoder (KY040) &amp; PCM5102A Audio DAC-kal.

.....................................................................................

Feltöltés Arduino IDE progival, ESP32 WROOM-nál az ESP32 Dev Module alaplapot kiválasztva, a megfelelő port-al, ami nálam a COM7-es és a megfelelő partició kiválasztásával, ami lehet a default is, de nálam jobban megy a Huge APP (3MB No OTA/1MB SPIFFS)-el.

A rádió linkeket megváltoztathatod a listában de a rádió név listát is sorrendben kell szerkeszteni hozzá.

Nűködése: Indításnál az első rádió streem indul el.

Rotary-t tekergetve a hangerő változik 0 és 21 között.

A rotary-t megnyomva, a streem-ek között lépked, amire azonnal megszólal a választott  rádió csatorna. 3 másodperc tétlenség után visszaáll hangerő módba.

.....................................................................................

https://www.youtube.com/watch?v=iDNedIdGSz4

.....................................................................................


Saját otthoni Wifi-d neve = "Your-WiFi-ID";

Saját otthoni Wifi-d jelszava = "Your-WiFi-Passport";

128x64 pixeles i2c OLED kijelző szükséges a jelenlegi beállításoknál

ESP32 WROOM-nál: 21-es pinre az (SDA), 22-es pinre az (SCL)-t kel használni

A Rotary Encoder-nél a CLK-t a 32-es pinre, a DT-t a 33-as pinre, az SW-t a 15-ös pinre kötjük.

A PCM5102A DAC Modul Pinjei: BCK=26-osra, LRCK=25-ösre, DIN=27-esre, az SCK=GND-re.
kel kötni.

A többit én a következőkre kötöttem: 

1-es (FLT) -t GND-re, 2-es (DEMF) -t GND-re, 

3-as (XSMT) -t +3.3V-ra, 4-es (FMT) -t GND-re. 

De van itt egy leírás a megértéshez:    
.....................................................................................

FLT:  1    
// H1L - (HIGH v. LOW) (Filter Mode Select)

// LOW (GND-re kötve) → "Slow Roll-Off" (lassú esésű szűrő)

azaz Lágyabb, természetesebb tranziensek, de enyhén nagyobb frekvenciafázis-torzítás.

// HIGH (3.3V vagy 5V-ra kötve) → "Fast Roll-Off" (gyors esésű szűrő)

azaz Precízebb, gyorsabb tranziensek, de enyhén élesebb hangzás.

// Stúdió minőségű felvételeknél érzékelhetőbb (pl. FLAC 24-bit 96kHz), 
 MP3 és más tömörített fájlok esetén a különbség alig hallható.

.....................................................................................

DEMF:  2   
// H2L - (HIGH v. LOW) (Speciális magashang-korrekciós szűrő)

// DEMP (De-emphasis Control) pre-emphasis/de-emphasis technológia 
azaz Kiemelés-szabályozás 44,1 kHz-es mintavételi frekvenciához: Ki (alacsony) / Be (magas)

// A pre-emphasis egy régi zajcsökkentési technika, 
 amelyet a CD-k korai korszakában és FM rádiózásnál használtak. 
 A felvétel során a magas frekvenciákat megemelték (pre-emphasis).
 A lejátszás során ezt visszaállították (de-emphasis), hogy csökkentsék a zajt.

// Ha a DEMP=LOW (GND), akkor semmi sem változik (ez az alapértelmezett).

// Ha a DEMP=HIGH (3.3V/5V), akkor bekapcsol egy magasvágó szűrőt.

.....................................................................................

XSMT:  3   
// H3L - (HIGH v. LOW) (Lágy némítás)  

// Lágy némítás (LOW) / Lágy némítás feloldása (HIGH)

azaz egy MUTE kapcsoló gombot lehet létesíteni vele

.....................................................................................

FMT:   4   
// H4L - (LOW) (fix) (Hangformátum kiválasztása)

//  I2S (LOW) / Balra igazított (HIGH)

.....................................................................................

SCK:    // GND - (fix)

BCK:    // ESP32 - Pin: 26 (BCLK)

DIN:    // ESP32 - Pin: 27 (DOUT)

RCK:    // ESP32 - Pin: 25 (LRCK)

.....................................................................................

VIN:   // +3.3V  v. +5V (fix)

GND:   // GND (fix)

.....................................................................................


