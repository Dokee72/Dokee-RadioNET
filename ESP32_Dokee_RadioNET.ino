/***********************************************
 *       Laslie Dokee & the ChatGPT 5.0        *
 *                   2025                      *
 *         ESP32 - Net-Radio Project           *
 *           PCM5102A Audio DAC-al             *
 *          KY040 Rotary Encoder-rel           *
 *           SSD1306 Tipusú OLED-el            *
 *         Ami 0,91" 128x32 pixel I2C          *
 *        vagy 0,96" 128x64 pixel I2C          *
 ***********************************************/
//          🎶 Living on Music!!! 🎶          //
//         "Tech meets music forever!"         //
//     Rock-ON, Dokee-módra!  😎🎧✨🎶💡    //
//    „Turn up the volume & light the vibe!”   //
//*********************************************//
#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>

// OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ESP8266Audio (MP3-only)
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

// ===== WiFi =====
const char* ssid = "Your-WiFi-ID";
const char* password = "Your-WiFi-Passport";

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 // 128x64px OLED
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino ESP32:     21(SDA), 22(SCL)
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long lastScreenUpdate = 0;
int screenMode = 0;
// Logo Bmp Set
#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
// Adafruit Logo
static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };

// ===== Rotary =====
#define ROT_A 32
#define ROT_B 33
#define ROT_SW 15
int lastA = HIGH;
unsigned long lastEncTime = 0;
unsigned long lastBtnTime = 0;
const uint16_t ENC_DEBOUNCE_MS = 3;
const uint16_t BTN_DEBOUNCE_MS = 250;

// --- Encoder state machine ---
#define ENC_INVERT false          // ha fordítva teker, tedd true-ra
const int ENC_STEPS_PER_DETENT = 4; // 4 fél-lépés = 1 kattanás (a legtöbb KY-040 ilyen)

volatile uint8_t enc_last = 0;
volatile int8_t  enc_accum = 0;

// Gray-dekóder táblázat (A,B) -> irány
static const int8_t ENC_TABLE[16] = {
  0, -1, +1,  0,
 +1,  0,  0, -1,
 -1,  0,  0, +1,
  0, +1, -1,  0
};

// ===== Módok =====
enum UIMode { MODE_VOLUME = 0, MODE_STATION = 1 };
UIMode mode = MODE_VOLUME;
unsigned long lastInteraction = 0;
const uint16_t STATION_IDLE_BACK_MS = 3000;  // 3s tétlenség után vissza hangerő módba

// ===== Hangerő =====
int volumeSteps = 2; // 0..21
inline float gainFromSteps(int s) { return (float)s / 21.0f; }

// ===== Rádió állomások (MP3) =====
const char* radioStations[] = {
  "http://dancewave.online:8080/dance.mp3",  // Dance Wave (http://dancewave.online/dance.mp3, /dance.ogg, /retrodance.mp3)
  "http://188.165.11.30:4420/live.mp3",  // Magic Disco Radio - Ami elvarazsol!
  "http://retro.dancewave.online/retrodance.mp3",  // Dance Wave - Retro (/retrodance.ogg, /retrodance.mp3, /retrodance.aac)
  "http://megadanceradio.hopto.org:8000/livemega.mp3",  // MegaDance Rádió (http://megadanceradio.hopto.org:8000/megamobil)
  "http://45.67.156.157:18006/live",     // MixRadio
  "http://45.67.156.157:18004/retro",     // MixRadio RETRO
  "http://45.67.156.157:18010/creamix",    // MixRadio Creamix - NON-STOP SUMMER MUSIC
  "http://s03.diazol.hu:35150/stream",  // SuperDj Rádió - Emlékeket ébresztünk (/mobil, /stream)
  "http://188.165.11.30:8200/live.mp3",  // Poptarisznya - A Retro netrádió
  "http://icast.connectmedia.hu/5202/live.mp3",  // Rádió 1 (5201/live.mp3 v. 5202/live.mp3)
  "http://s2.audiostream.hu:8081/bdpstrock_192k",  // BDPST ROCK - Magyarország rockrádiója (bdpstrock_192k, bdpstrock_320k, bdpstrock_96k, bdpstrock_FLAC)
  "https://ice2.somafm.com/u80s-128-mp3",  // SomaFM - 80s
};
const char* stationNames[] = {
  "< Dance Wave >",
  "< Magic Disco Radio>",
  "< Dance Wave Retro>",
  "< MegaDance Radio >",
  "< MixRadio >",
  "< MixRadio RETRO >",
  "< MixRadio Creamix >", // NON-STOP SUMMER MUSIC
  "< SuperDj Radio >", // Emlékeket ébresztünk
  "< Poptarisznya >", // A Retro netradio
  "< Rádió 1 >", // Csak igazi mai sláger megy!
  "< BDPST ROCK >",  // Magyarország rockrádiója
  "< SomaFM - 80s >",
};

String lastStreamTitle = "";   // ide kerül a StreamTitle

const int STATION_COUNT = sizeof(radioStations)/sizeof(radioStations[0]);
int currentStation = 0;

// ===== Audio objektumok =====
AudioGeneratorMP3 *mp3 = nullptr;
AudioFileSourceICYStream *file = nullptr;
AudioFileSourceBuffer *buff = nullptr;
AudioOutputI2S *out = nullptr;

// ===== OLED rajzolás =====
void drawVolumeUI() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Dokee - Radio FreeNET");

  display.fillRect(0, 10, SCREEN_WIDTH, 4, SSD1306_WHITE);

  display.setCursor(0, 16);
  display.print(stationNames[currentStation]);
  Serial.println(stationNames[currentStation]);

  // keret + kitoltes
  int barW = map(volumeSteps, 0, 21, 0, SCREEN_WIDTH - 6);
  display.drawRect(0, 26, SCREEN_WIDTH, 8, SSD1306_WHITE);
  display.fillRect(2, 27, barW, 6, SSD1306_WHITE);

  // Volume szam + csik
  display.setCursor(0, 36);
  display.print("Vol: ");
  display.print(volumeSteps);
  display.print("/21");

  display.setCursor(0, 45);
  // Radio Stream Infodisplay.setCursor(0, 55);
if (lastStreamTitle.length()) {
  if (lastStreamTitle.length() > 21) {
    display.print(lastStreamTitle.substring(0, 21));
    display.print(lastStreamTitle.substring(21, 42));
    Serial.print(lastStreamTitle.substring(0, 100));
  } else {
    display.print(lastStreamTitle);
    Serial.println(lastStreamTitle);
  }
} else {
  display.print("Enjoy,the MusicRadio!"); // amíg nem jön meta
  display.setCursor(0, 55);
  display.print("EnjoyTheFill TheWibe!"); // amíg nem jön meta
  Serial.println("  - Dokee RadioNET! -"); // amíg nem jön meta
  Serial.println("Enjoy The Fill The Wibe!"); // amíg nem jön meta
}
  display.display();
}

void drawStationUI() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Dokee - Radio FreeNET");

  display.fillRect(0, 12, SCREEN_WIDTH, 6, SSD1306_WHITE);

  display.setCursor(21, 20);
  display.println("Radio Station");

  // keretezve kiemeljuk az aktuális állomást
  display.drawRect(0, 30, SCREEN_WIDTH, 12, SSD1306_WHITE);
  display.setCursor(3, 32);
  display.println(stationNames[currentStation]);
  Serial.println(stationNames[currentStation]);

  display.setCursor(0, 45);
  display.printf("RSSI: %d dBm\n", WiFi.RSSI());

  display.setCursor(0, 55);
  display.printf("IP: %s\n", WiFi.localIP().toString().c_str());  

  display.display();
}

// ===== Audio indítás/váltás =====
void stopStream() {
  if (mp3) { mp3->stop(); delete mp3; mp3 = nullptr; }
  if (buff) { delete buff; buff = nullptr; }
  if (file) { delete file; file = nullptr; }
}

void playStation(int idx) {
  stopStream();

  file = new AudioFileSourceICYStream(radioStations[idx]);
  // file = new AudioFileSourceICYStream(radioStations[currentStation]);

  // meta/státusz callback regisztrálása
  file->RegisterMetadataCB(MDCallback, (void*)"meta");
  file->RegisterStatusCB(StatusCallback, (void*)"stat");

  // ha a libed tudja ezeket, érdemes bekapcsolni:
  // file->useHTTP10(true);     // HTTP/1.0 -> nincs chunked
  // file->setReconnect(true);  // automata újracsatlakozás (ha elérhető)
  // file->setTimeoutMs(8000);  // nagyobb hálózati timeout

  // nagyobb buffer = kevesebb akadás (24–32 KB jó kompromisszum)
  // létrehozásnál:
  buff = new AudioFileSourceBuffer(file, 32 * 1024);  // próbáld 24K, 32K, esetleg 48K, 64K
  // hasznos: nézd meg mennyi a szabad heap indítás előtt
  Serial.printf("Free heap: %lu, Largest block: %u\n",
                 ESP.getFreeHeap(), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));


  if (!out) {
    out = new AudioOutputI2S();          // külső I2S DAC
    out->SetPinout(26, 25, 27);          // BCK, LRCK, DOUT (PCM5102A)
    /*
    
    */
  }
  out->SetGain(gainFromSteps(volumeSteps));

  mp3 = new AudioGeneratorMP3();
  mp3->begin(buff, out);

  if (mode == MODE_VOLUME) drawVolumeUI();
  else drawStationUI();
}
// ICY/ID3 meta callback – ESP8266Audio
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string) {
  (void)cbData; (void)isUnicode;
  // A legtöbb rádió "StreamTitle"-t küld (pl. előadó - szám)
  if (!strcasecmp(type, "StreamTitle") || !strcasecmp(type, "icy-name")) {
    lastStreamTitle = String(string ? string : "");
    // OLED alsó sor frissítés
    display.fillRect(0, 45, SCREEN_WIDTH, 19, SSD1306_BLACK);
    display.setCursor(0, 45);
    display.setTextSize(1);
    // kb. 21-22 karakter fér ki
    if (lastStreamTitle.length() > 22) {
      display.print(lastStreamTitle.substring(0, 21));
      display.print(lastStreamTitle.substring(21, 42));
    } else {
      display.print(lastStreamTitle);
    }
    display.display();
  }
}

// (opcionális) státusz callback – debugra
void StatusCallback(void *cbData, int code, const char *string) {
  (void)cbData;
  Serial.printf("ICY status %d: %s\n", code, string ? string : "");
}

// ===== Rotary kezelés =====
void handleEncoder() {
  unsigned long now = millis();

  // --- Quadrature dekódolás (csak teljes kattanásra léptet) ---
  uint8_t state = (digitalRead(ROT_A) << 1) | digitalRead(ROT_B);
  uint8_t idx   = (enc_last << 2) | state;   // előző + most
  int8_t  mov   = ENC_TABLE[idx];

  if (mov) {
    if (ENC_INVERT) mov = -mov;
    enc_accum += mov;         // fél-lépések gyűjtése
    enc_last   = state;

    if (enc_accum >= ENC_STEPS_PER_DETENT) {
      // egy teljes kattanás jobbra
      if (mode == MODE_VOLUME) {
        if (volumeSteps < 21) volumeSteps++;
        if (out) out->SetGain((float)volumeSteps / 21.0f);
        drawVolumeUI();
      } else { // MODE_STATION
        currentStation = (currentStation + 1) % STATION_COUNT;
        playStation(currentStation);
      }
      enc_accum = 0;
      lastInteraction = now;
    } else if (enc_accum <= -ENC_STEPS_PER_DETENT) {
      // egy teljes kattanás balra
      if (mode == MODE_VOLUME) {
        if (volumeSteps > 0) volumeSteps--;
        if (out) out->SetGain((float)volumeSteps / 21.0f);
        drawVolumeUI();
      } else {
        currentStation = (currentStation - 1 + STATION_COUNT) % STATION_COUNT;
        playStation(currentStation);
      }
      enc_accum = 0;
      lastInteraction = now;
    }
  } else {
    enc_last = state; // pattogásnál se maradjon régi
  }

  // --- gombnyomás (debounce-olt) ---
  static unsigned long lastBtnTime = 0;
  if (digitalRead(ROT_SW) == LOW && (now - lastBtnTime) > 250) {
    mode = (mode == MODE_VOLUME) ? MODE_STATION : MODE_VOLUME;
    if (mode == MODE_VOLUME) drawVolumeUI(); else drawStationUI();
    lastBtnTime = now;
    lastInteraction = now;
  }

  // --- 3s tétlenség után vissza hangerő módba ---
  if (mode == MODE_STATION && (now - lastInteraction) > 3000) {
    mode = MODE_VOLUME;
    drawVolumeUI();
  }
}

// ===== Setup / Loop =====
void setup() {
  Serial.begin(115200);
  // I2C fix lábakra
  Wire.begin(21, 22);
  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 init FAIL"));
    while (1) delay(10);
  }
    Serial.println(" ");
    Serial.println("SSD1306 OLED inicializálása...");
    delay(500); // Pause for 0,5 seconds
    display.display();
    display.clearDisplay();
    // Logo Megjelenítése
    // Draw a single pixel in white
    display.drawPixel(10, 10, SSD1306_WHITE);
    delay(3000); // Pause for 3 seconds

  display.clearDisplay(); display.display();
  Serial.println("Oled Kijelző Inicializálva!");
  delay(500);
  // Rotary Encoder
  pinMode(ROT_A, INPUT_PULLUP);
  pinMode(ROT_B, INPUT_PULLUP);
  pinMode(ROT_SW, INPUT_PULLUP);
  // lastA = digitalRead(ROT_A);
  // kezdő 2 bites állapot (A a magasabb bit)
  enc_last = (digitalRead(ROT_A) << 1) | digitalRead(ROT_B);
  Serial.println("Rotary Encoder Kalibrálva... ");
  delay(500);

  // WiFi
  WiFi.begin(ssid, password);
  // WiFi.setSleep(false); // fontos: ne akadozzon a stream (nem takarékoskodik)
  // WiFi.setTxPower(WIFI_POWER_19_5dBm);     // nagyobb térerő (ha az országodban oké)
  while (WiFi.status() != WL_CONNECTED) { delay(150); Serial.print("."); }
  Serial.printf("\nWiFi OK, IP: %s\n", WiFi.localIP().toString().c_str());
  // Indítás
  drawVolumeUI();
  playStation(currentStation);
}

void loop() {
  
  // Memória debug - Serial monitoron
  static unsigned long utolsoKiiras = 0;
  if (millis() - utolsoKiiras > 30000) {  // 30 másodpercenként
    utolsoKiiras = millis();
    // Szabad heap memória lekérdezése: 
    Serial.println(" ");   
    Serial.print("Teljes heap memória: ");
    Serial.print(ESP.getHeapSize());
    Serial.println(" byte");
    Serial.print("Szabad heap memória: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" byte");
    Serial.print("Legnagyobb egyben lefoglalható blokk mérete: ");
    Serial.print(ESP.getMaxAllocHeap());
    Serial.println(" byte");   
    Serial.println(" ");   
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
    
  }

  handleEncoder();

  if (mp3 && mp3->isRunning()) {
    if (!mp3->loop()) {
      mp3->stop();
      delay(50);
      // egyszerű újraindítás ugyanarra az állomásra
      playStation(currentStation);
    }
  } else {
    // ha valamiért leállt, próbáld újra
    static unsigned long lastTry = 0;
    if (millis() - lastTry > 3000) {
      playStation(currentStation);
      lastTry = millis();
    }
  }

  // egyszerű WiFi-reconnect
  if (WiFi.status() != WL_CONNECTED) WiFi.reconnect();
}
/******************************************************************************
-----------------------------------------
PCM5102A DAC Modul Pinjei: BCK=26, LRCK=25, DIN=27
-----------------------------------------
FLT:  1    // H1L - (HIGH v. LOW) (Filter Mode Select)
// LOW (GND-re kötve) → "Slow Roll-Off" (lassú esésű szűrő)
// Lágyabb, természetesebb tranziensek, de enyhén nagyobb frekvenciafázis-torzítás.
// HIGH (3.3V vagy 5V-ra kötve) → "Fast Roll-Off" (gyors esésű szűrő)
// Precízebb, gyorsabb tranziensek, de enyhén élesebb hangzás.
// Stúdió minőségű felvételeknél érzékelhetőbb (pl. FLAC 24-bit 96kHz), 
// MP3 és más tömörített fájlok esetén a különbség alig hallható.
-----------------------------------------
DEMF:  2   // H2L - (HIGH v. LOW) (Speciális magashang-korrekciós szűrő)
// DEMP (De-emphasis Control) pre-emphasis/de-emphasis technológia 
// Kiemelés-szabályozás 44,1 kHz-es mintavételi frekvenciához: Ki (alacsony) / Be (magas)
// A pre-emphasis egy régi zajcsökkentési technika, 
// amelyet a CD-k korai korszakában és FM rádiózásnál használtak. 
//  A felvétel során a magas frekvenciákat megemelték (pre-emphasis).
// A lejátszás során ezt visszaállították (de-emphasis), hogy csökkentsék a zajt.
// Ha a DEMP=LOW (GND), akkor semmi sem változik (ez az alapértelmezett).
// Ha a DEMP=HIGH (3.3V/5V), akkor bekapcsol egy magasvágó szűrőt.
-----------------------------------------
XSMT:  3   // H3L - (HIGH v. LOW) (Lágy némítás)  
// Lágy némítás (LOW) / Lágy némítás feloldása (HIGH)
-----------------------------------------
FMT:   4   // H4L - (LOW) (fix) (Hangformátum kiválasztása)
//  I2S (LOW) / Balra igazított (HIGH)
----------------------------------------
SCK:     // GND - (fix)
BCK:     // ESP32 - Pin: 26 (BCLK)
DIN:    // ESP32 - Pin: 27 (DOUT)
RCK:    // ESP32 - Pin: 25 (LRCK)
---------------------------------------
VIN:   // +3.3V  v. +5V (fix)
GND: // GND (fix)
---------------------------------------
******************************************************************************/
