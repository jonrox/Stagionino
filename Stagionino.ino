/*
 * STAGIONINO - Centralina Intelligente per Stagionatura Salumi
 * Versione 1.0 - Sistema Di stagionatura automatica
 * 
 * Developed by: Arduino Framework Expert
 * Hardware: Arduino Mega 2560 + ILI9486 Touch Display + Sensori AM2315C/DHT11
 * 
 * Copyright (C) 2024 - Sistema di controllo ambientale per stagionatura salumi
 * 
 * Features:
 * - Sistema a Stati Finiti adattivo
 * - Modalità Automatica con programmi SD
 * - Modalità Manuale con controllo continuo
 * - Interfaccia Touch 3.5" con anti-bounce
 * - Sensori precisione AM2315C + DHT11
 * - Controllo 6 attuatori (Frigo,Risc,Deum,Umid,Vent1,Vent2)
 * - Sistema allarmi con LED WS2812B
 * - Modalità emergenza automatica
 * - Protezione Watchdog e overflow millis()
 * - Gestione SD con retry automatico
 */

// ===============================================================================
// LIBRERIE ARDUINO NECESSARIE
// ===============================================================================
// Installare tramite Arduino IDE Library Manager:

// Librerie Core Arduino
#include <Wire.h>                    // I2C (inclusa in Arduino)
#include <SPI.h>                     // SPI (inclusa in Arduino)
#include <EEPROM.h>                  // Memoria persistente (inclusa)
#include <avr/wdt.h>                 // Protezione watchdog (inclusa)

// Sensori e RTC
#include <Adafruit_AM2315.h>         // v2.1.0+ - Sensore AM2315C I2C
#include <DHT.h>                     // v1.4.4+ - Sensore DHT11
#include <RTClib.h>                  // v2.1.1+ - RTC DS1307

// Display e Touch
#include <Adafruit_GFX.h>            // v1.11.3+ - Grafica base
#include <MCUFRIEND_kbv.h>           // v2.9.9+ - Display ILI9486 8-bit
#include <XPT2046_Touchscreen.h>     // v1.4+ - Touch screen

// LED e Storage
#include <FastLED.h>                 // v3.5.0+ - LED WS2812B
#include <SD.h>                      // v1.2.4+ - SD Card

// ===============================================================================
// CONFIGURAZIONE HARDWARE PINS
// ===============================================================================

// Display ILI9486 (8-bit parallelo - pin automatici MCUFRIEND_kbv)
// Shield 3.5" si monta direttamente su Arduino Mega

// Touch XPT2046 (SPI condiviso)
#define TOUCH_CS    6               // Touch Chip Select
#define TOUCH_IRQ   7               // Touch Interrupt

// SD Card (SPI condiviso) 
#define SD_CS       4               // SD Card Chip Select

// Sensori
#define DHT_PIN     2               // DHT11 Data Pin
#define DHT_TYPE    DHT11           // Tipo sensore DHT
// AM2315C usa I2C: SDA=20, SCL=21 (automatici)
// DS1307 usa I2C: SDA=20, SCL=21 (automatici)

// LED Indicatori
#define LED_24BIT_PIN    8          // WS2812B 24 LED strip
#define LED_12BIT_PIN    9          // WS2812B 12 LED strip
#define NUM_LEDS_24      24         // Numero LED strip principale
#define NUM_LEDS_12      12         // Numero LED strip secondario

// Buzzer
#define BUZZER_PIN      10          // Buzzer passivo (PWM/tone)

// Relè Attuatori (logica invertita: LOW=ON, HIGH=OFF)
#define RELAY_FRIGORIFERO      22   // Relè 1 - Frigorifero
#define RELAY_RISCALDATORE     23   // Relè 2 - Riscaldatore  
#define RELAY_DEUMIDIFICATORE  24   // Relè 3 - Deumidificatore
#define RELAY_UMIDIFICATORE    25   // Relè 4 - Umidificatore
#define RELAY_VENTOLA_IN       26   // Relè 5 - Ventola Immissione
#define RELAY_VENTOLA_OUT      27   // Relè 6 - Ventola Estrazione

// ===============================================================================
// COSTANTI DI SISTEMA
// ===============================================================================

// Temporizzazioni (millisecondi)
#define SENSOR_READ_INTERVAL     30000    // Lettura sensori ogni 30s
#define DISPLAY_UPDATE_INTERVAL  15000    // Aggiornamento display ogni 15s
#define CONTROL_FRIDGE_INTERVAL  480000   // Controllo frigo ogni 8min
#define CONTROL_HEATER_INTERVAL  300000   // Controllo riscaldatore ogni 5min
#define CONTROL_DEHUM_INTERVAL   240000   // Controllo deumidificatore ogni 4min
#define CONTROL_HUM_INTERVAL     180000   // Controllo umidificatore ogni 3min
#define CONTROL_FAN_INTERVAL     120000   // Controllo ventole ogni 2min

// Cicli minimi dispositivi (protezione meccanica)
#define MIN_FRIDGE_ON_TIME       300000   // Frigo ON minimo 5min
#define MIN_FRIDGE_OFF_TIME      180000   // Frigo OFF minimo 3min
#define MIN_HEATER_ON_TIME       240000   // Riscaldatore ON minimo 4min
#define MIN_HEATER_OFF_TIME      120000   // Riscaldatore OFF minimo 2min
#define MIN_DEHUM_ON_TIME        600000   // Deumidificatore ON minimo 10min
#define MIN_DEHUM_OFF_TIME       300000   // Deumidificatore OFF minimo 5min
#define MIN_HUM_ON_TIME          300000   // Umidificatore ON minimo 5min
#define MIN_HUM_OFF_TIME         180000   // Umidificatore OFF minimo 3min
#define MIN_FAN_ON_TIME          60000    // Ventole ON minimo 1min
#define MIN_FAN_OFF_TIME         30000    // Ventole OFF minimo 30s

// Protezioni e sicurezza
#define TOUCH_DEBOUNCE_TIME      300      // Anti-bounce touch 300ms
#define WATCHDOG_TIMEOUT         8000     // Watchdog 8 secondi
#define SENSOR_RETRY_COUNT       3        // Tentativi lettura sensori
#define SD_RETRY_COUNT           3        // Tentativi inizializzazione SD
#define EMERGENCY_TEMP_OFFSET    5.0      // Offset emergenza temperatura
#define MUTE_ALARM_DURATION      300000   // Mute allarmi 5 minuti

// Range validazione sensori
#define AM2315C_TEMP_MIN         -40.0    // Temperatura minima AM2315C
#define AM2315C_TEMP_MAX         80.0     // Temperatura massima AM2315C
#define AM2315C_HUM_MIN          0.0      // Umidità minima AM2315C
#define AM2315C_HUM_MAX          100.0    // Umidità massima AM2315C
#define DHT11_TEMP_MIN           0.0      // Temperatura minima DHT11
#define DHT11_TEMP_MAX           50.0     // Temperatura massima DHT11
#define DHT11_HUM_MIN            20.0     // Umidità minima DHT11
#define DHT11_HUM_MAX            90.0     // Umidità massima DHT11

// Colori display (RGB565)
#define COLOR_BLACK              0x0000   // Nero
#define COLOR_WHITE              0xFFFF   // Bianco
#define COLOR_RED                0xF800   // Rosso
#define COLOR_GREEN              0x07E0   // Verde
#define COLOR_BLUE               0x001F   // Blu
#define COLOR_YELLOW             0xFFE0   // Giallo
#define COLOR_CYAN               0x07FF   // Ciano
#define COLOR_MAGENTA            0xF81F   // Magenta
#define COLOR_ORANGE             0xFD20   // Arancione
#define COLOR_GRAY               0x8410   // Grigio
#define COLOR_LIGHT_GRAY         0xC618   // Grigio chiaro
#define COLOR_DARK_GRAY          0x4208   // Grigio scuro
#define COLOR_EMERGENCY          0xF800   // Rosso emergenza

// Dimensioni interfaccia
#define SCREEN_WIDTH             480      // Larghezza schermo
#define SCREEN_HEIGHT            320      // Altezza schermo
#define BUTTON_HEIGHT            40       // Altezza pulsanti
#define BUTTON_MARGIN            10       // Margine pulsanti
#define TEXT_SIZE_SMALL          1        // Testo piccolo
#define TEXT_SIZE_MEDIUM         2        // Testo medio
#define TEXT_SIZE_LARGE          3        // Testo grande

// ===============================================================================
// STRUTTURE DATI SISTEMA
// ===============================================================================

// Struttura dati sensori
struct SensorData {
    float temp_internal;              // Temperatura interna (AM2315C)
    float hum_internal;               // Umidità interna (AM2315C)
    float temp_external;              // Temperatura esterna (DHT11)
    float hum_external;               // Umidità esterna (DHT11)
    bool internal_valid;              // Flag validità sensore interno
    bool external_valid;              // Flag validità sensore esterno
    unsigned long last_read_time;     // Timestamp ultima lettura
    int internal_error_count;         // Contatore errori consecutivi interno
    int external_error_count;         // Contatore errori consecutivi esterno
};

// Struttura stato sistema
struct SystemState {
    bool emergency_mode;              // Modalità emergenza attiva
    bool sd_available;                // SD card disponibile
    bool rtc_available;               // RTC disponibile  
    bool am2315_available;            // AM2315C disponibile
    bool dht11_available;             // DHT11 disponibile
    unsigned long uptime;             // Tempo di funzionamento
    unsigned long last_watchdog_reset; // Ultimo reset watchdog
};

// Enumerazione schermate interfaccia
enum DisplayScreen {
    SCREEN_MAIN_DASHBOARD,            // Dashboard principale
    SCREEN_SENSOR_DATA,               // Dati sensori dettagliati
    SCREEN_SETTINGS,                  // Impostazioni sistema
    SCREEN_PROGRAMS,                  // Gestione programmi
    SCREEN_EMERGENCY,                 // Schermata emergenza
    SCREEN_CALIBRATION               // Calibrazione touch
};

// Struttura gestione touch
struct TouchData {
    bool is_touched;                  // Touch attualmente premuto
    bool was_touched;                 // Touch nel ciclo precedente
    uint16_t x, y;                    // Coordinate attuali
    uint16_t last_x, last_y;          // Coordinate precedenti
    unsigned long touch_start_time;   // Inizio tocco
    unsigned long last_touch_time;    // Ultimo tocco valido
    unsigned long debounce_start;     // Inizio debounce
    bool debounce_active;             // Debounce in corso
};

// Struttura interfaccia utente
struct UIState {
    DisplayScreen current_screen;     // Schermata corrente
    DisplayScreen previous_screen;    // Schermata precedente
    bool screen_needs_redraw;         // Flag ridisegno schermata
    bool force_full_redraw;           // Flag ridisegno completo
    unsigned long last_screen_update; // Ultimo aggiornamento schermata
    uint16_t background_color;        // Colore sfondo
    uint16_t text_color;              // Colore testo
    uint16_t accent_color;            // Colore accento
};

// ===============================================================================
// DICHIARAZIONE OGGETTI HARDWARE
// ===============================================================================

// Display e Touch
MCUFRIEND_kbv tft;                  // Display ILI9486
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);  // Touch screen

// Sensori
Adafruit_AM2315 am2315;             // Sensore interno AM2315C
DHT dht(DHT_PIN, DHT_TYPE);         // Sensore esterno DHT11
RTC_DS1307 rtc;                     // RTC DS1307

// LED
CRGB leds_24[NUM_LEDS_24];          // LED strip principale 24bit
CRGB leds_12[NUM_LEDS_12];          // LED strip secondario 12bit

// ===============================================================================
// VARIABILI GLOBALI SISTEMA
// ===============================================================================

SensorData sensors;                 // Dati sensori globali
SystemState system_state;           // Stato sistema globale
TouchData touch_data;               // Dati gestione touch
UIState ui_state;                   // Stato interfaccia utente

// ===============================================================================
// FORWARD DECLARATIONS
// ===============================================================================

// Inizializzazione
void initializeHardware();
void initializeSensors();
void initializeDisplay();
void initializeRTC();
void initializeSD();
void initializeLEDs();
void initializeRelays();

// Gestione sensori
bool readSensors();
bool validateSensorData();
void handleSensorErrors();

// Sistema di controllo
void updateControlSystem();
void controlTemperature();
void controlHumidity();
void controlVentilation();

// Modalità emergenza
void checkEmergencyConditions();
void enterEmergencyMode();
void exitEmergencyMode();

// Interfaccia utente
void updateDisplay();
void handleTouch();
void updateLEDs();
void handleBuzzer();

// Sistema touch avanzato
bool processTouchDebounce();
void processValidTouch();
void handleDashboardTouch();
void handleSensorDataTouch();
void handleSettingsTouch();
void handleProgramsTouch();
void handleEmergencyTouch();
void handleCalibrationTouch();
void switchToScreen(DisplayScreen new_screen);
void updateUIColors();

// Sistema display avanzato
void drawCurrentScreen();
void drawMainDashboard();
void drawSensorDataBox(int x, int y, int width, int height);
void drawActuatorStatus(int x, int y, int width, int height);
void drawActuatorIcon(int x, int y, int size, const __FlashStringHelper* label, bool active, uint16_t color);
void drawNavigationButtons();
void drawButton(int x, int y, int width, int height, const __FlashStringHelper* label, uint16_t color);
void drawSensorDataScreen();
void drawSettingsScreen();
void drawProgramsScreen();
void drawEmergencyScreen();
void drawCalibrationScreen();
void drawErrorScreen();
void drawStatusBar();
void updateScreenData();

// Gestione programmi
void loadPrograms();
void executeProgram();
void switchToManualMode();

// Utilità
void resetWatchdog();
void handleMillisOverflow();
void saveSettings();
void loadSettings();

// ===============================================================================
// SETUP - INIZIALIZZAZIONE SISTEMA
// ===============================================================================

void setup() {
    // Inizializzazione seriale per debug
    Serial.begin(115200);
    Serial.println(F("=== STAGIONINO V1.0 - AVVIO SISTEMA ==="));
    Serial.println(F("Sistema di controllo ambientale per stagionatura salumi"));
    
    // Disabilita watchdog durante inizializzazione
    wdt_disable();
    
    // Inizializzazione strutture dati
    initializeSystemState();
    
    // Inizializzazione hardware step by step
    Serial.println(F("Inizializzazione hardware..."));
    initializeHardware();
    
    // Abilita watchdog per protezione sistema
    Serial.println(F("Abilitazione protezione watchdog..."));
    wdt_enable(WDTO_8S);  // Watchdog 8 secondi (massimo disponibile)
    
    Serial.println(F("=== SISTEMA STAGIONINO PRONTO ==="));
    printSystemStatus();
}

// ===============================================================================
// INIZIALIZZAZIONE STATO SISTEMA
// ===============================================================================

void initializeSystemState() {
    // Inizializzazione sensori
    sensors.temp_internal = NAN;
    sensors.hum_internal = NAN;
    sensors.temp_external = NAN;
    sensors.hum_external = NAN;
    sensors.internal_valid = false;
    sensors.external_valid = false;
    sensors.last_read_time = 0;
    sensors.internal_error_count = 0;
    sensors.external_error_count = 0;
    
    // Inizializzazione stato sistema
    system_state.emergency_mode = false;
    system_state.sd_available = false;
    system_state.rtc_available = false;
    system_state.am2315_available = false;
    system_state.dht11_available = false;
    system_state.uptime = millis();
    system_state.last_watchdog_reset = millis();
    
    // Inizializzazione touch
    touch_data.is_touched = false;
    touch_data.was_touched = false;
    touch_data.x = 0;
    touch_data.y = 0;
    touch_data.last_x = 0;
    touch_data.last_y = 0;
    touch_data.touch_start_time = 0;
    touch_data.last_touch_time = 0;
    touch_data.debounce_start = 0;
    touch_data.debounce_active = false;
    
    // Inizializzazione interfaccia utente
    ui_state.current_screen = SCREEN_MAIN_DASHBOARD;
    ui_state.previous_screen = SCREEN_MAIN_DASHBOARD;
    ui_state.screen_needs_redraw = true;
    ui_state.force_full_redraw = true;
    ui_state.last_screen_update = 0;
    ui_state.background_color = COLOR_BLACK;
    ui_state.text_color = COLOR_WHITE;
    ui_state.accent_color = COLOR_GREEN;
    
    Serial.println(F("Strutture dati sistema inizializzate"));
}

void printSystemStatus() {
    Serial.println(F("=== STATUS SISTEMA ==="));
    Serial.print(F("AM2315C: "));
    Serial.println(system_state.am2315_available ? F("OK") : F("ERRORE"));
    Serial.print(F("DHT11: "));
    Serial.println(system_state.dht11_available ? F("OK") : F("ERRORE"));
    Serial.print(F("RTC: "));
    Serial.println(system_state.rtc_available ? F("OK") : F("ERRORE"));
    Serial.print(F("SD Card: "));
    Serial.println(system_state.sd_available ? F("OK") : F("ERRORE"));
    Serial.print(F("Modalità Emergenza: "));
    Serial.println(system_state.emergency_mode ? F("ATTIVA") : F("NORMALE"));
    
    if (system_state.am2315_available || system_state.dht11_available) {
        Serial.println(F("Modalità: Pronto per operazioni normali"));
    } else {
        Serial.println(F("ATTENZIONE: Nessun sensore disponibile!"));
    }
}

// ===============================================================================
// LOOP PRINCIPALE - CICLO INFINITO
// ===============================================================================

void loop() {
    // Reset watchdog ogni ciclo
    resetWatchdog();
    
    // Gestione overflow millis() (dopo ~50 giorni)
    handleMillisOverflow();
    
    // Lettura sensori con intervallo ottimizzato
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
        readSensors();
        handleSensorErrors();
        lastSensorRead = millis();
    }
    
    // Aggiornamento sistema di controllo
    updateControlSystem();
    
    // Controllo condizioni di emergenza
    checkEmergencyConditions();
    
    // Aggiornamento interfaccia utente
    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        updateDisplay();
        lastDisplayUpdate = millis();
    }
    
    // Gestione input touch
    handleTouch();
    
    // Aggiornamento LED e buzzer
    updateLEDs();
    handleBuzzer();
    
    // Piccolo delay per ottimizzare CPU
    delay(50);
}

// ===============================================================================
// FUNZIONI DI INIZIALIZZAZIONE
// ===============================================================================

void initializeHardware() {
    Serial.println(F("-> Inizializzazione pin e componenti base"));
    
    // Inizializzazione pin relè (stato iniziale OFF)
    initializeRelays();
    
    // Inizializzazione sensori
    Serial.println(F("-> Inizializzazione sensori"));
    initializeSensors();
    
    // Inizializzazione display
    Serial.println(F("-> Inizializzazione display"));
    initializeDisplay();
    
    // Inizializzazione RTC
    Serial.println(F("-> Inizializzazione RTC"));
    initializeRTC();
    
    // Inizializzazione LED
    Serial.println(F("-> Inizializzazione LED"));
    initializeLEDs();
    
    // Inizializzazione SD (con retry)
    Serial.println(F("-> Inizializzazione SD Card"));
    initializeSD();
    
    Serial.println(F("-> Inizializzazione hardware completata"));
}

// ===============================================================================
// FUNZIONI STUB - IMPLEMENTAZIONE NEI PROSSIMI STEP
// ===============================================================================

void initializeRelays() {
    // Configurazione pin relè come OUTPUT
    pinMode(RELAY_FRIGORIFERO, OUTPUT);
    pinMode(RELAY_RISCALDATORE, OUTPUT);
    pinMode(RELAY_DEUMIDIFICATORE, OUTPUT);
    pinMode(RELAY_UMIDIFICATORE, OUTPUT);
    pinMode(RELAY_VENTOLA_IN, OUTPUT);
    pinMode(RELAY_VENTOLA_OUT, OUTPUT);
    
    // Stato iniziale: tutti i relè OFF (logica invertita: HIGH = OFF)
    digitalWrite(RELAY_FRIGORIFERO, HIGH);
    digitalWrite(RELAY_RISCALDATORE, HIGH);
    digitalWrite(RELAY_DEUMIDIFICATORE, HIGH);
    digitalWrite(RELAY_UMIDIFICATORE, HIGH);
    digitalWrite(RELAY_VENTOLA_IN, HIGH);
    digitalWrite(RELAY_VENTOLA_OUT, HIGH);
    
    Serial.println(F("  Relè inizializzati - Stato: TUTTI OFF"));
}

void initializeSensors() {
    Serial.println(F("  -> Inizializzazione AM2315C (I2C)"));
    
    // Inizializzazione AM2315C con retry
    bool am2315_ok = false;
    for (int retry = 0; retry < SENSOR_RETRY_COUNT; retry++) {
        if (am2315.begin()) {
            am2315_ok = true;
            Serial.println(F("     AM2315C: OK"));
            break;
        } else {
            Serial.print(F("     AM2315C tentativo "));
            Serial.print(retry + 1);
            Serial.println(F(" fallito"));
            delay(1000);
        }
    }
    
    if (!am2315_ok) {
        Serial.println(F("     ERRORE: AM2315C non risponde!"));
        Serial.println(F("     Verificare collegamenti I2C"));
        system_state.am2315_available = false;
    } else {
        system_state.am2315_available = true;
    }
    
    // Inizializzazione DHT11
    Serial.println(F("  -> Inizializzazione DHT11"));
    dht.begin();
    Serial.println(F("     DHT11: Inizializzato"));
    system_state.dht11_available = true; // Assume OK, sarà testato dopo
    
    // Test lettura sensori
    Serial.println(F("  -> Test lettura sensori"));
    delay(2000); // DHT11 richiede 2s per prima lettura
    
    // Test AM2315C
    if (am2315_ok) {
        float temp, hum;
        if (am2315.readTemperatureAndHumidity(&temp, &hum)) {
            Serial.print(F("     AM2315C - T: "));
            Serial.print(temp);
            Serial.print(F("°C, H: "));
            Serial.print(hum);
            Serial.println(F("%"));
        } else {
            Serial.println(F("     ERRORE: Lettura AM2315C fallita"));
        }
    }
    
    // Test DHT11
    float dht_temp = dht.readTemperature();
    float dht_hum = dht.readHumidity();
    if (!isnan(dht_temp) && !isnan(dht_hum)) {
        Serial.print(F("     DHT11 - T: "));
        Serial.print(dht_temp);
        Serial.print(F("°C, H: "));
        Serial.print(dht_hum);
        Serial.println(F("%"));
    } else {
        Serial.println(F("     ERRORE: Lettura DHT11 fallita"));
        Serial.println(F("     Verificare collegamento pin 2"));
    }
}

void initializeDisplay() {
    Serial.println(F("  -> Inizializzazione Display ILI9486"));
    
    // Identificazione automatica display
    uint16_t ID = tft.readID();
    Serial.print(F("     Display ID: 0x"));
    Serial.println(ID, HEX);
    
    // Inizializzazione display
    if (ID == 0xFFFF) {
        Serial.println(F("     ERRORE: Display non rilevato!"));
        Serial.println(F("     Verificare collegamenti shield"));
        ID = 0x9486; // Fallback per ILI9486
    }
    
    tft.begin(ID);
    tft.setRotation(1); // Orientamento landscape
    tft.fillScreen(0x0000); // Schermo nero
    
    // Test display con messaggio di avvio
    tft.setTextColor(0xFFFF, 0x0000); // Bianco su nero
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println(F("STAGIONINO V1.0"));
    tft.setTextSize(1);
    tft.setCursor(10, 40);
    tft.println(F("Sistema Stagionatura Salumi"));
    tft.setCursor(10, 60);
    tft.println(F("Inizializzazione..."));
    
    Serial.print(F("     Display: OK - Risoluzione: "));
    Serial.print(tft.width());
    Serial.print(F("x"));
    Serial.println(tft.height());
    
    // Inizializzazione touchscreen
    Serial.println(F("  -> Inizializzazione Touchscreen XPT2046"));
    touch.begin();
    touch.setRotation(1); // Stesso orientamento del display
    
    // Test touch
    if (touch.tirqTouched()) {
        Serial.println(F("     Touchscreen: Tocco rilevato durante init"));
    }
    Serial.println(F("     Touchscreen: OK"));
}

void initializeRTC() {
    Serial.println(F("  -> Inizializzazione RTC DS1307"));
    
    if (!rtc.begin()) {
        Serial.println(F("     ERRORE: RTC DS1307 non trovato!"));
        Serial.println(F("     Verificare collegamenti I2C"));
        Serial.println(F("     Verificare batteria backup"));
        system_state.rtc_available = false;
        return;
    }
    
    system_state.rtc_available = true;
    
    // Controllo se RTC ha perso l'orario
    if (!rtc.isrunning()) {
        Serial.println(F("     RTC non in esecuzione, impostazione orario..."));
        // Imposta data/ora di compilazione come fallback
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        Serial.println(F("     Orario impostato a data/ora compilazione"));
    } else {
        Serial.println(F("     RTC: OK - Orario mantenuto"));
    }
    
    // Visualizza orario attuale
    DateTime now = rtc.now();
    Serial.print(F("     Data/Ora: "));
    Serial.print(now.day());
    Serial.print(F("/"));
    Serial.print(now.month());
    Serial.print(F("/"));
    Serial.print(now.year());
    Serial.print(F(" "));
    Serial.print(now.hour());
    Serial.print(F(":"));
    if (now.minute() < 10) Serial.print(F("0"));
    Serial.print(now.minute());
    Serial.print(F(":"));
    if (now.second() < 10) Serial.print(F("0"));
    Serial.println(now.second());
}

void initializeLEDs() {
    Serial.println(F("  -> Inizializzazione LED WS2812B"));
    
    // Configurazione LED strip 24bit (principale)
    FastLED.addLeds<WS2812B, LED_24BIT_PIN, GRB>(leds_24, NUM_LEDS_24);
    
    // Configurazione LED strip 12bit (secondario)  
    FastLED.addLeds<WS2812B, LED_12BIT_PIN, GRB>(leds_12, NUM_LEDS_12);
    
    // Impostazione luminosità globale (50% per evitare sovraccarico)
    FastLED.setBrightness(128);
    
    // Test LED - sequenza di avvio
    Serial.println(F("     Test sequenza LED..."));
    
    // Spegni tutti i LED
    fill_solid(leds_24, NUM_LEDS_24, CRGB::Black);
    fill_solid(leds_12, NUM_LEDS_12, CRGB::Black);
    FastLED.show();
    delay(200);
    
    // Test LED 24bit - Verde
    fill_solid(leds_24, NUM_LEDS_24, CRGB::Green);
    FastLED.show();
    delay(500);
    fill_solid(leds_24, NUM_LEDS_24, CRGB::Black);
    FastLED.show();
    
    // Test LED 12bit - Blu
    fill_solid(leds_12, NUM_LEDS_12, CRGB::Blue);
    FastLED.show();
    delay(500);
    fill_solid(leds_12, NUM_LEDS_12, CRGB::Black);
    FastLED.show();
    
    // Entrambi - Bianco breve
    fill_solid(leds_24, NUM_LEDS_24, CRGB::White);
    fill_solid(leds_12, NUM_LEDS_12, CRGB::White);
    FastLED.show();
    delay(200);
    fill_solid(leds_24, NUM_LEDS_24, CRGB::Black);
    fill_solid(leds_12, NUM_LEDS_12, CRGB::Black);
    FastLED.show();
    
    Serial.print(F("     LED Strip 24bit: "));
    Serial.print(NUM_LEDS_24);
    Serial.println(F(" LED - OK"));
    Serial.print(F("     LED Strip 12bit: "));
    Serial.print(NUM_LEDS_12);
    Serial.println(F(" LED - OK"));
}

void initializeSD() {
    Serial.println(F("  -> Inizializzazione SD Card"));
    
    bool sd_ok = false;
    
    // Tentativi multipli di inizializzazione SD
    for (int retry = 0; retry < SD_RETRY_COUNT; retry++) {
        Serial.print(F("     Tentativo SD "));
        Serial.print(retry + 1);
        Serial.print(F("/"));
        Serial.println(SD_RETRY_COUNT);
        
        if (SD.begin(SD_CS)) {
            sd_ok = true;
            Serial.println(F("     SD Card: Inizializzazione OK"));
            break;
        } else {
            Serial.println(F("     SD Card: Inizializzazione fallita"));
            delay(1000);
        }
    }
    
    if (!sd_ok) {
        Serial.println(F("     ERRORE: SD Card non disponibile!"));
        Serial.println(F("     Sistema continuerà in modalità limitata"));
        Serial.println(F("     - Solo modalità manuale disponibile"));
        Serial.println(F("     - Nessun salvataggio programmi"));
        system_state.sd_available = false;
        return;
    }
    
    system_state.sd_available = true;
    
    // Verifica tipo e dimensioni SD
    uint32_t cardSize = SD.cardSize() / (1024 * 1024);
    uint8_t cardType = SD.cardType();
    
    Serial.print(F("     Tipo SD: "));
    switch (cardType) {
        case CARD_MMC:
            Serial.println(F("MMC"));
            break;
        case CARD_SD:
            Serial.println(F("SDSC"));
            break;
        case CARD_SDHC:
            Serial.println(F("SDHC"));
            break;
        default:
            Serial.println(F("Sconosciuto"));
    }
    
    Serial.print(F("     Dimensione: "));
    Serial.print(cardSize);
    Serial.println(F(" MB"));
    
    // Creazione directory se non esistono
    if (!SD.exists("/programs")) {
        if (SD.mkdir("/programs")) {
            Serial.println(F("     Directory '/programs' creata"));
        } else {
            Serial.println(F("     ERRORE: Impossibile creare '/programs'"));
        }
    }
    
    if (!SD.exists("/logs")) {
        if (SD.mkdir("/logs")) {
            Serial.println(F("     Directory '/logs' creata"));
        } else {
            Serial.println(F("     ERRORE: Impossibile creare '/logs'"));
        }
    }
    
    // Test scrittura
    File testFile = SD.open("/test.txt", FILE_WRITE);
    if (testFile) {
        testFile.println(F("Stagionino SD Test"));
        testFile.close();
        Serial.println(F("     Test scrittura: OK"));
        
        // Rimuovi file di test
        SD.remove("/test.txt");
    } else {
        Serial.println(F("     ERRORE: Test scrittura fallito"));
    }
}

bool readSensors() {
    bool success = true;
    
    // Reset flag validità per questa lettura
    sensors.internal_valid = false;
    sensors.external_valid = false;
    
    // ===== LETTURA SENSORE INTERNO AM2315C =====
    if (system_state.am2315_available) {
        float temp_int, hum_int;
        
        // Tentativi multipli per AM2315C
        bool am2315_success = false;
        for (int retry = 0; retry < SENSOR_RETRY_COUNT; retry++) {
            if (am2315.readTemperatureAndHumidity(&temp_int, &hum_int)) {
                // Validazione range dati AM2315C
                if (temp_int >= AM2315C_TEMP_MIN && temp_int <= AM2315C_TEMP_MAX &&
                    hum_int >= AM2315C_HUM_MIN && hum_int <= AM2315C_HUM_MAX) {
                    
                    sensors.temp_internal = temp_int;
                    sensors.hum_internal = hum_int;
                    sensors.internal_valid = true;
                    sensors.internal_error_count = 0;
                    am2315_success = true;
                    break;
                } else {
                    Serial.print(F("ERRORE: Dati AM2315C fuori range - T:"));
                    Serial.print(temp_int);
                    Serial.print(F("°C, H:"));
                    Serial.print(hum_int);
                    Serial.println(F("%"));
                }
            }
            
            if (retry < SENSOR_RETRY_COUNT - 1) {
                delay(100); // Pausa tra tentativi
            }
        }
        
        if (!am2315_success) {
            sensors.internal_error_count++;
            Serial.print(F("ERRORE: Lettura AM2315C fallita (errori consecutivi: "));
            Serial.print(sensors.internal_error_count);
            Serial.println(F(")"));
            
            // Disabilita sensore dopo troppi errori
            if (sensors.internal_error_count >= 10) {
                system_state.am2315_available = false;
                Serial.println(F("CRITICO: AM2315C disabilitato dopo 10 errori consecutivi"));
            }
            success = false;
        }
    } else {
        // Sensore interno non disponibile
        sensors.temp_internal = NAN;
        sensors.hum_internal = NAN;
        sensors.internal_error_count++;
    }
    
    // ===== LETTURA SENSORE ESTERNO DHT11 =====
    if (system_state.dht11_available) {
        float temp_ext = dht.readTemperature();
        float hum_ext = dht.readHumidity();
        
        // Validazione DHT11
        if (!isnan(temp_ext) && !isnan(hum_ext) &&
            temp_ext >= DHT11_TEMP_MIN && temp_ext <= DHT11_TEMP_MAX &&
            hum_ext >= DHT11_HUM_MIN && hum_ext <= DHT11_HUM_MAX) {
            
            sensors.temp_external = temp_ext;
            sensors.hum_external = hum_ext;
            sensors.external_valid = true;
            sensors.external_error_count = 0;
        } else {
            sensors.external_error_count++;
            sensors.temp_external = NAN;
            sensors.hum_external = NAN;
            
            if (!isnan(temp_ext) || !isnan(hum_ext)) {
                Serial.print(F("ERRORE: Dati DHT11 fuori range - T:"));
                Serial.print(temp_ext);
                Serial.print(F("°C, H:"));
                Serial.print(hum_ext);
                Serial.println(F("%"));
            } else {
                Serial.print(F("ERRORE: Lettura DHT11 fallita (errori consecutivi: "));
                Serial.print(sensors.external_error_count);
                Serial.println(F(")"));
            }
            
            // Disabilita sensore dopo troppi errori
            if (sensors.external_error_count >= 10) {
                system_state.dht11_available = false;
                Serial.println(F("CRITICO: DHT11 disabilitato dopo 10 errori consecutivi"));
            }
            success = false;
        }
    } else {
        // Sensore esterno non disponibile
        sensors.temp_external = NAN;
        sensors.hum_external = NAN;
        sensors.external_error_count++;
    }
    
    // Aggiorna timestamp
    sensors.last_read_time = millis();
    
    // Log dati sensori ogni 5 letture (debug)
    static int log_counter = 0;
    if (++log_counter >= 5) {
        logSensorData();
        log_counter = 0;
    }
    
    return success;
}

// ===============================================================================
// FUNZIONI SUPPORTO SENSORI
// ===============================================================================

bool validateSensorData() {
    // Validazione completa dati sensori
    bool valid = false;
    
    // Controllo sensore interno
    if (sensors.internal_valid) {
        if (sensors.temp_internal >= AM2315C_TEMP_MIN && 
            sensors.temp_internal <= AM2315C_TEMP_MAX &&
            sensors.hum_internal >= AM2315C_HUM_MIN && 
            sensors.hum_internal <= AM2315C_HUM_MAX) {
            valid = true;
        }
    }
    
    // Controllo sensore esterno (opzionale per controllo)
    if (sensors.external_valid) {
        if (sensors.temp_external >= DHT11_TEMP_MIN && 
            sensors.temp_external <= DHT11_TEMP_MAX &&
            sensors.hum_external >= DHT11_HUM_MIN && 
            sensors.hum_external <= DHT11_HUM_MAX) {
            // Sensore esterno OK ma non critico per validazione generale
        }
    }
    
    return valid;
}

void handleSensorErrors() {
    // Gestione errori sensori
    
    // Controllo modalità emergenza per sensore interno
    if (!sensors.internal_valid && sensors.internal_error_count >= 10) {
        if (!system_state.emergency_mode) {
            Serial.println(F("EMERGENZA: Attivazione modalità emergenza per perdita sensore interno"));
            enterEmergencyMode();
        }
    }
    
    // Log errori persistenti
    if (sensors.internal_error_count > 0 || sensors.external_error_count > 0) {
        Serial.print(F("Status errori - Interno: "));
        Serial.print(sensors.internal_error_count);
        Serial.print(F(", Esterno: "));
        Serial.println(sensors.external_error_count);
    }
}

void logSensorData() {
    // Log dati sensori per debug
    Serial.println(F("=== DATI SENSORI ==="));
    
    if (sensors.internal_valid) {
        Serial.print(F("Interno (AM2315C): T="));
        Serial.print(sensors.temp_internal, 1);
        Serial.print(F("°C, H="));
        Serial.print(sensors.hum_internal, 1);
        Serial.println(F("%"));
    } else {
        Serial.println(F("Interno (AM2315C): ERRORE"));
    }
    
    if (sensors.external_valid) {
        Serial.print(F("Esterno (DHT11): T="));
        Serial.print(sensors.temp_external, 1);
        Serial.print(F("°C, H="));
        Serial.print(sensors.hum_external, 1);
        Serial.println(F("%"));
    } else {
        Serial.println(F("Esterno (DHT11): ERRORE"));
    }
    
    Serial.print(F("Uptime: "));
    Serial.print(millis() / 1000);
    Serial.println(F(" secondi"));
}

void updateControlSystem() {
    // TODO: Implementazione completa nel prossimo step
}

void checkEmergencyConditions() {
    // Controllo condizioni che richiedono modalità emergenza
    
    // Condizione 1: Sensore interno non risponde per 10 cicli consecutivi
    if (sensors.internal_error_count >= 10 && !system_state.emergency_mode) {
        Serial.println(F("EMERGENZA: Sensore interno non risponde da 10 cicli"));
        enterEmergencyMode();
        return;
    }
    
    // Condizione 2: Temperatura critica (implementazione base)
    if (sensors.internal_valid && !system_state.emergency_mode) {
        if (sensors.temp_internal < (0.0 - EMERGENCY_TEMP_OFFSET)) {
            Serial.print(F("EMERGENZA: Temperatura critica rilevata: "));
            Serial.print(sensors.temp_internal);
            Serial.println(F("°C"));
            enterEmergencyMode();
            return;
        }
    }
    
    // Condizione di uscita: sensore interno torna funzionante
    if (system_state.emergency_mode && sensors.internal_valid && 
        sensors.internal_error_count == 0) {
        Serial.println(F("Condizioni normalizzate, uscita da modalità emergenza"));
        exitEmergencyMode();
    }
}

void enterEmergencyMode() {
    system_state.emergency_mode = true;
    
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║       MODALITÀ EMERGENZA ATTIVA     ║"));
    Serial.println(F("║  MANTENIMENTO TEMPERATURA SICURA    ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    Serial.println(F(""));
    
    // Spegni tutti gli attuatori tranne frigorifero se necessario
    digitalWrite(RELAY_RISCALDATORE, HIGH);      // OFF
    digitalWrite(RELAY_DEUMIDIFICATORE, HIGH);   // OFF
    digitalWrite(RELAY_UMIDIFICATORE, HIGH);     // OFF
    digitalWrite(RELAY_VENTOLA_IN, HIGH);        // OFF
    digitalWrite(RELAY_VENTOLA_OUT, HIGH);       // OFF
    
    // Il controllo frigorifero sarà gestito nel sistema di controllo
    Serial.println(F("Attuatori disattivati - Solo controllo frigorifero attivo"));
    
    // Forza cambio alla schermata di emergenza
    switchToScreen(SCREEN_EMERGENCY);
}

void exitEmergencyMode() {
    system_state.emergency_mode = false;
    
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║     USCITA MODALITÀ EMERGENZA       ║"));
    Serial.println(F("║    RIPRISTINO OPERAZIONI NORMALI    ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    Serial.println(F(""));
}

void updateDisplay() {
    // Controlla se è necessario aggiornare il display
    unsigned long current_time = millis();
    
    // Forza ridisegno se schermata cambiata o in modalità emergenza
    if (ui_state.screen_needs_redraw || system_state.emergency_mode) {
        ui_state.force_full_redraw = true;
        ui_state.screen_needs_redraw = false;
    }
    
    // Ridisegno completo o aggiornamento incrementale
    if (ui_state.force_full_redraw) {
        drawCurrentScreen();
        ui_state.force_full_redraw = false;
        ui_state.last_screen_update = current_time;
    } else {
        // Aggiornamento incrementale solo per dati che cambiano
        updateScreenData();
    }
}

void drawCurrentScreen() {
    // Pulisce schermo con colore di sfondo
    tft.fillScreen(ui_state.background_color);
    
    // Disegna la schermata corrente
    switch (ui_state.current_screen) {
        case SCREEN_MAIN_DASHBOARD:
            drawMainDashboard();
            break;
            
        case SCREEN_SENSOR_DATA:
            drawSensorDataScreen();
            break;
            
        case SCREEN_SETTINGS:
            drawSettingsScreen();
            break;
            
        case SCREEN_PROGRAMS:
            drawProgramsScreen();
            break;
            
        case SCREEN_EMERGENCY:
            drawEmergencyScreen();
            break;
            
        case SCREEN_CALIBRATION:
            drawCalibrationScreen();
            break;
            
        default:
            // Schermata sconosciuta - disegna errore
            drawErrorScreen();
            break;
    }
    
    // Disegna sempre la barra di stato
    drawStatusBar();
}

// ===============================================================================
// IMPLEMENTAZIONE SCHERMATE SPECIFICHE
// ===============================================================================

void drawMainDashboard() {
    // Titolo principale
    tft.setTextColor(ui_state.text_color);
    tft.setTextSize(TEXT_SIZE_LARGE);
    tft.setCursor(10, 10);
    tft.println(F("STAGIONINO"));
    
    // Sottotitolo
    tft.setTextSize(TEXT_SIZE_SMALL);
    tft.setCursor(10, 40);
    tft.println(F("Sistema Stagionatura Salumi"));
    
    // Area dati sensori principali
    drawSensorDataBox(10, 70, SCREEN_WIDTH - 20, 120);
    
    // Indicatori stato attuatori
    drawActuatorStatus(10, 200, SCREEN_WIDTH - 20, 60);
    
    // Pulsanti di navigazione
    drawNavigationButtons();
}

void drawSensorDataBox(int x, int y, int width, int height) {
    // Cornice dati sensori
    tft.drawRect(x, y, width, height, ui_state.accent_color);
    
    // Titolo sezione
    tft.setTextColor(ui_state.accent_color);
    tft.setTextSize(TEXT_SIZE_MEDIUM);
    tft.setCursor(x + 10, y + 10);
    tft.println(F("SENSORI"));
    
    // Dati sensore interno
    tft.setTextColor(ui_state.text_color);
    tft.setTextSize(TEXT_SIZE_SMALL);
    tft.setCursor(x + 10, y + 35);
    tft.print(F("Interno: "));
    
    if (sensors.internal_valid) {
        tft.setTextColor(COLOR_GREEN);
        tft.print(sensors.temp_internal, 1);
        tft.print(F("C "));
        tft.print(sensors.hum_internal, 1);
        tft.print(F("%"));
    } else {
        tft.setTextColor(COLOR_RED);
        tft.print(F("ERRORE"));
    }
    
    // Dati sensore esterno
    tft.setTextColor(ui_state.text_color);
    tft.setCursor(x + 10, y + 55);
    tft.print(F("Esterno: "));
    
    if (sensors.external_valid) {
        tft.setTextColor(COLOR_CYAN);
        tft.print(sensors.temp_external, 1);
        tft.print(F("C "));
        tft.print(sensors.hum_external, 1);
        tft.print(F("%"));
    } else {
        tft.setTextColor(COLOR_YELLOW);
        tft.print(F("NON DISP"));
    }
    
    // Timestamp ultima lettura
    tft.setTextColor(COLOR_GRAY);
    tft.setCursor(x + 10, y + 75);
    tft.print(F("Agg: "));
    tft.print((millis() - sensors.last_read_time) / 1000);
    tft.print(F("s fa"));
}

void drawActuatorStatus(int x, int y, int width, int height) {
    // Cornice stato attuatori
    tft.drawRect(x, y, width, height, ui_state.accent_color);
    
    // Titolo sezione
    tft.setTextColor(ui_state.accent_color);
    tft.setTextSize(TEXT_SIZE_MEDIUM);
    tft.setCursor(x + 10, y + 10);
    tft.println(F("ATTUATORI"));
    
    // Indicatori stato (simulato per ora)
    int icon_size = 20;
    int icon_spacing = 60;
    int start_x = x + 20;
    int icon_y = y + 35;
    
    // Frigorifero
    drawActuatorIcon(start_x, icon_y, icon_size, F("F"), 
                     digitalRead(RELAY_FRIGORIFERO) == LOW, COLOR_BLUE);
    
    // Riscaldatore
    drawActuatorIcon(start_x + icon_spacing, icon_y, icon_size, F("R"), 
                     digitalRead(RELAY_RISCALDATORE) == LOW, COLOR_RED);
    
    // Deumidificatore
    drawActuatorIcon(start_x + 2*icon_spacing, icon_y, icon_size, F("D"), 
                     digitalRead(RELAY_DEUMIDIFICATORE) == LOW, COLOR_ORANGE);
    
    // Umidificatore
    drawActuatorIcon(start_x + 3*icon_spacing, icon_y, icon_size, F("U"), 
                     digitalRead(RELAY_UMIDIFICATORE) == LOW, COLOR_CYAN);
    
    // Ventole
    drawActuatorIcon(start_x + 4*icon_spacing, icon_y, icon_size, F("V1"), 
                     digitalRead(RELAY_VENTOLA_IN) == LOW, COLOR_GREEN);
    
    drawActuatorIcon(start_x + 5*icon_spacing, icon_y, icon_size, F("V2"), 
                     digitalRead(RELAY_VENTOLA_OUT) == LOW, COLOR_GREEN);
}

void drawActuatorIcon(int x, int y, int size, const __FlashStringHelper* label, 
                      bool active, uint16_t color) {
    if (active) {
        tft.fillCircle(x + size/2, y + size/2, size/2, color);
        tft.setTextColor(COLOR_BLACK);
    } else {
        tft.drawCircle(x + size/2, y + size/2, size/2, COLOR_GRAY);
        tft.setTextColor(COLOR_GRAY);
    }
    
    tft.setTextSize(TEXT_SIZE_SMALL);
    int text_width = strlen_P((const char*)label) * 6;
    tft.setCursor(x + (size - text_width) / 2, y + size/2 - 4);
    tft.print(label);
}

void drawNavigationButtons() {
    int button_width = (SCREEN_WIDTH - 40) / 3;
    int button_y = SCREEN_HEIGHT - 50;
    
    // Pulsante Sensori
    drawButton(10, button_y, button_width, 40, F("SENSORI"), COLOR_BLUE);
    
    // Pulsante Programmi  
    drawButton(20 + button_width, button_y, button_width, 40, F("PROGRAMMI"), COLOR_GREEN);
    
    // Pulsante Impostazioni
    drawButton(30 + 2*button_width, button_y, button_width, 40, F("SETTINGS"), COLOR_ORANGE);
}

void drawButton(int x, int y, int width, int height, 
                const __FlashStringHelper* label, uint16_t color) {
    // Cornice pulsante
    tft.drawRect(x, y, width, height, color);
    tft.drawRect(x+1, y+1, width-2, height-2, color);
    
    // Testo centrato
    tft.setTextColor(color);
    tft.setTextSize(TEXT_SIZE_SMALL);
    int text_width = strlen_P((const char*)label) * 6;
    int text_x = x + (width - text_width) / 2;
    int text_y = y + (height - 8) / 2;
    tft.setCursor(text_x, text_y);
    tft.print(label);
}

void drawSensorDataScreen() {
    // Implementazione dettagliata schermata sensori
    tft.setTextColor(ui_state.text_color);
    tft.setTextSize(TEXT_SIZE_MEDIUM);
    tft.setCursor(10, 10);
    tft.println(F("DATI SENSORI"));
    
    // Sensore interno dettagliato
    tft.setTextSize(TEXT_SIZE_SMALL);
    tft.setCursor(10, 50);
    tft.println(F("=== SENSORE INTERNO (AM2315C) ==="));
    
    if (sensors.internal_valid) {
        tft.setTextColor(COLOR_GREEN);
        tft.setCursor(10, 70);
        tft.print(F("Temperatura: "));
        tft.print(sensors.temp_internal, 2);
        tft.println(F(" C"));
        
        tft.setCursor(10, 90);
        tft.print(F("Umidita: "));
        tft.print(sensors.hum_internal, 2);
        tft.println(F(" %"));
        
        tft.setCursor(10, 110);
        tft.print(F("Errori: "));
        tft.println(sensors.internal_error_count);
    } else {
        tft.setTextColor(COLOR_RED);
        tft.setCursor(10, 70);
        tft.println(F("SENSORE NON DISPONIBILE"));
        tft.setCursor(10, 90);
        tft.print(F("Errori consecutivi: "));
        tft.println(sensors.internal_error_count);
    }
    
    // Sensore esterno dettagliato
    tft.setTextColor(ui_state.text_color);
    tft.setCursor(10, 140);
    tft.println(F("=== SENSORE ESTERNO (DHT11) ==="));
    
    if (sensors.external_valid) {
        tft.setTextColor(COLOR_CYAN);
        tft.setCursor(10, 160);
        tft.print(F("Temperatura: "));
        tft.print(sensors.temp_external, 1);
        tft.println(F(" C"));
        
        tft.setCursor(10, 180);
        tft.print(F("Umidita: "));
        tft.print(sensors.hum_external, 1);
        tft.println(F(" %"));
        
        tft.setCursor(10, 200);
        tft.print(F("Errori: "));
        tft.println(sensors.external_error_count);
    } else {
        tft.setTextColor(COLOR_YELLOW);
        tft.setCursor(10, 160);
        tft.println(F("SENSORE NON DISPONIBILE"));
        tft.setCursor(10, 180);
        tft.print(F("Errori consecutivi: "));
        tft.println(sensors.external_error_count);
    }
    
    // Pulsante indietro
    drawButton(10, SCREEN_HEIGHT - 50, 80, 40, F("BACK"), ui_state.accent_color);
}

void drawSettingsScreen() {
    tft.setTextColor(ui_state.text_color);
    tft.setTextSize(TEXT_SIZE_MEDIUM);
    tft.setCursor(10, 10);
    tft.println(F("IMPOSTAZIONI"));
    
    // Area calibrazione touch
    tft.setTextSize(TEXT_SIZE_SMALL);
    tft.setCursor(10, 50);
    tft.println(F("Calibrazione Touch:"));
    
    drawButton(SCREEN_WIDTH/4, SCREEN_HEIGHT/3, SCREEN_WIDTH/2, 60, 
               F("CALIBRA"), COLOR_YELLOW);
    
    // Pulsante indietro
    drawButton(10, SCREEN_HEIGHT - 50, 80, 40, F("BACK"), ui_state.accent_color);
}

void drawProgramsScreen() {
    tft.setTextColor(ui_state.text_color);
    tft.setTextSize(TEXT_SIZE_MEDIUM);
    tft.setCursor(10, 10);
    tft.println(F("PROGRAMMI"));
    
    tft.setTextSize(TEXT_SIZE_SMALL);
    tft.setCursor(10, 50);
    tft.println(F("Funzionalita in sviluppo..."));
    
    // Pulsante indietro
    drawButton(10, SCREEN_HEIGHT - 50, 80, 40, F("BACK"), ui_state.accent_color);
}

void drawEmergencyScreen() {
    // Schermata emergenza con sfondo rosso
    tft.fillScreen(COLOR_EMERGENCY);
    
    tft.setTextColor(COLOR_WHITE);
    tft.setTextSize(TEXT_SIZE_LARGE);
    tft.setCursor(50, 50);
    tft.println(F("EMERGENZA"));
    
    tft.setTextSize(TEXT_SIZE_MEDIUM);
    tft.setCursor(20, 100);
    tft.println(F("MODALITA SICURA"));
    
    tft.setTextSize(TEXT_SIZE_SMALL);
    tft.setCursor(20, 140);
    tft.println(F("- Sensore interno offline"));
    tft.setCursor(20, 160);
    tft.println(F("- Solo controllo frigorifero"));
    tft.setCursor(20, 180);
    tft.println(F("- Temperatura sicurezza: 4C"));
    
    // Tempo in emergenza
    tft.setCursor(20, 220);
    tft.print(F("Tempo emergenza: "));
    tft.print(millis() / 60000);
    tft.println(F(" min"));
    
    drawButton(SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT - 50, 120, 40, 
               F("DASHBOARD"), COLOR_WHITE);
}

void drawCalibrationScreen() {
    tft.setTextColor(ui_state.text_color);
    tft.setTextSize(TEXT_SIZE_MEDIUM);
    tft.setCursor(10, 10);
    tft.println(F("CALIBRAZIONE"));
    
    tft.setTextSize(TEXT_SIZE_SMALL);
    tft.setCursor(10, 50);
    tft.println(F("Tocca i punti per calibrare:"));
    
    // Punti di calibrazione
    tft.fillCircle(50, 100, 5, COLOR_RED);
    tft.fillCircle(SCREEN_WIDTH-50, 100, 5, COLOR_RED);
    tft.fillCircle(50, SCREEN_HEIGHT-100, 5, COLOR_RED);
    tft.fillCircle(SCREEN_WIDTH-50, SCREEN_HEIGHT-100, 5, COLOR_RED);
    
    // Coordinate attuali
    tft.setCursor(10, 200);
    tft.print(F("X: "));
    tft.print(touch_data.x);
    tft.print(F(", Y: "));
    tft.println(touch_data.y);
    
    drawButton(SCREEN_WIDTH - 110, SCREEN_HEIGHT - 50, 100, 40, 
               F("FATTO"), COLOR_GREEN);
}

void drawErrorScreen() {
    tft.setTextColor(COLOR_RED);
    tft.setTextSize(TEXT_SIZE_LARGE);
    tft.setCursor(50, 100);
    tft.println(F("ERRORE"));
    
    tft.setTextSize(TEXT_SIZE_SMALL);
    tft.setCursor(50, 150);
    tft.println(F("Schermata sconosciuta"));
}

void drawStatusBar() {
    // Barra di stato in alto a destra
    int status_x = SCREEN_WIDTH - 150;
    int status_y = 5;
    
    tft.setTextColor(COLOR_GRAY);
    tft.setTextSize(TEXT_SIZE_SMALL);
    
    // Indicatore modalità emergenza
    if (system_state.emergency_mode) {
        tft.setTextColor(COLOR_RED);
        tft.setCursor(status_x, status_y);
        tft.print(F("EMERGENZA"));
    }
    
    // Uptime
    tft.setTextColor(COLOR_GRAY);
    tft.setCursor(status_x, status_y + 15);
    tft.print(F("Up: "));
    tft.print(millis() / 60000);
    tft.print(F("m"));
}

void updateScreenData() {
    // Aggiornamento incrementale solo per dati che cambiano rapidamente
    // Implementazione per ottimizzare le prestazioni
    switch (ui_state.current_screen) {
        case SCREEN_MAIN_DASHBOARD:
            // Aggiorna solo i valori numerici dei sensori
            break;
        case SCREEN_SENSOR_DATA:
            // Aggiorna timestamp e contatori errori
            break;
        case SCREEN_EMERGENCY:
            // Aggiorna timer emergenza
            break;
        default:
            // Nessun aggiornamento incrementale per altre schermate
            break;
    }
}

void handleTouch() {
    // Aggiorna stato touch precedente
    touch_data.was_touched = touch_data.is_touched;
    touch_data.last_x = touch_data.x;
    touch_data.last_y = touch_data.y;
    
    // Legge stato touch corrente
    touch_data.is_touched = touch.touched();
    
    if (touch_data.is_touched) {
        // XPT2046_Touchscreen non ha getPoint(), usa i metodi diretti
        // Ottieni coordinate raw dal touch
        uint16_t raw_x, raw_y, raw_z;
        touch.readData(&raw_x, &raw_y, &raw_z);
        
        // Mappa coordinate touch a coordinate schermo
        touch_data.x = map(raw_x, 200, 3700, 0, SCREEN_WIDTH);
        touch_data.y = map(raw_y, 200, 3700, 0, SCREEN_HEIGHT);
        
        // Limita coordinate ai bordi schermo
        touch_data.x = constrain(touch_data.x, 0, SCREEN_WIDTH - 1);
        touch_data.y = constrain(touch_data.y, 0, SCREEN_HEIGHT - 1);
    }
    
    // Gestione anti-bounce touch
    if (processTouchDebounce()) {
        // Touch valido rilevato
        processValidTouch();
    }
}

bool processTouchDebounce() {
    unsigned long current_time = millis();
    
    // Rilevamento inizio tocco
    if (touch_data.is_touched && !touch_data.was_touched) {
        touch_data.touch_start_time = current_time;
        touch_data.debounce_start = current_time;
        touch_data.debounce_active = true;
        return false; // Attende fine debounce
    }
    
    // Controllo debounce in corso
    if (touch_data.debounce_active) {
        if (current_time - touch_data.debounce_start >= TOUCH_DEBOUNCE_TIME) {
            touch_data.debounce_active = false;
            
            // Verifica che il touch sia ancora attivo dopo debounce
            if (touch_data.is_touched) {
                touch_data.last_touch_time = current_time;
                return true; // Touch valido
            }
        }
        return false; // Debounce ancora in corso
    }
    
    // Touch già stabilizzato - controlla rilascio
    if (!touch_data.is_touched && touch_data.was_touched) {
        // Fine touch - resetta debounce per prossimo tocco
        touch_data.debounce_active = false;
        return false;
    }
    
    return false; // Nessun nuovo touch valido
}

void processValidTouch() {
    // Debug coordinate touch
    Serial.print(F("Touch valido: X="));
    Serial.print(touch_data.x);
    Serial.print(F(", Y="));
    Serial.println(touch_data.y);
    
    // Processa touch in base alla schermata corrente
    switch (ui_state.current_screen) {
        case SCREEN_MAIN_DASHBOARD:
            handleDashboardTouch();
            break;
            
        case SCREEN_SENSOR_DATA:
            handleSensorDataTouch();
            break;
            
        case SCREEN_SETTINGS:
            handleSettingsTouch();
            break;
            
        case SCREEN_PROGRAMS:
            handleProgramsTouch();
            break;
            
        case SCREEN_EMERGENCY:
            handleEmergencyTouch();
            break;
            
        case SCREEN_CALIBRATION:
            handleCalibrationTouch();
            break;
            
        default:
            // Schermata sconosciuta - torna al dashboard
            switchToScreen(SCREEN_MAIN_DASHBOARD);
            break;
    }
}

// ===============================================================================
// GESTIONE TOUCH PER SCHERMATE SPECIFICHE
// ===============================================================================

void handleDashboardTouch() {
    // Area pulsante "Sensori" (lato sinistro)
    if (touch_data.x < SCREEN_WIDTH / 3 && touch_data.y > SCREEN_HEIGHT - 60) {
        switchToScreen(SCREEN_SENSOR_DATA);
        return;
    }
    
    // Area pulsante "Programmi" (centro)
    if (touch_data.x > SCREEN_WIDTH / 3 && touch_data.x < 2 * SCREEN_WIDTH / 3 && 
        touch_data.y > SCREEN_HEIGHT - 60) {
        switchToScreen(SCREEN_PROGRAMS);
        return;
    }
    
    // Area pulsante "Impostazioni" (lato destro)
    if (touch_data.x > 2 * SCREEN_WIDTH / 3 && touch_data.y > SCREEN_HEIGHT - 60) {
        switchToScreen(SCREEN_SETTINGS);
        return;
    }
    
    // Touch su area dati sensori - mostra dettagli
    if (touch_data.y < SCREEN_HEIGHT / 2) {
        switchToScreen(SCREEN_SENSOR_DATA);
        return;
    }
}

void handleSensorDataTouch() {
    // Pulsante "Indietro" (angolo in basso a sinistra)
    if (touch_data.x < 100 && touch_data.y > SCREEN_HEIGHT - 60) {
        switchToScreen(SCREEN_MAIN_DASHBOARD);
        return;
    }
}

void handleSettingsTouch() {
    // Pulsante "Indietro"
    if (touch_data.x < 100 && touch_data.y > SCREEN_HEIGHT - 60) {
        switchToScreen(SCREEN_MAIN_DASHBOARD);
        return;
    }
    
    // Area calibrazione touch (centro schermo)
    if (touch_data.x > SCREEN_WIDTH / 4 && touch_data.x < 3 * SCREEN_WIDTH / 4 &&
        touch_data.y > SCREEN_HEIGHT / 3 && touch_data.y < 2 * SCREEN_HEIGHT / 3) {
        switchToScreen(SCREEN_CALIBRATION);
        return;
    }
}

void handleProgramsTouch() {
    // Pulsante "Indietro"
    if (touch_data.x < 100 && touch_data.y > SCREEN_HEIGHT - 60) {
        switchToScreen(SCREEN_MAIN_DASHBOARD);
        return;
    }
}

void handleEmergencyTouch() {
    // In modalità emergenza, permettere solo di tornare al dashboard
    if (touch_data.y > SCREEN_HEIGHT - 60) {
        switchToScreen(SCREEN_MAIN_DASHBOARD);
        return;
    }
}

void handleCalibrationTouch() {
    // Pulsante "Fatto" per uscire dalla calibrazione
    if (touch_data.x > SCREEN_WIDTH - 120 && touch_data.y > SCREEN_HEIGHT - 60) {
        switchToScreen(SCREEN_SETTINGS);
        return;
    }
    
    // Log coordinate per calibrazione
    Serial.print(F("Calibrazione - Raw: X="));
    Serial.print(touch_data.x);
    Serial.print(F(", Y="));
    Serial.println(touch_data.y);
}

void switchToScreen(DisplayScreen new_screen) {
    if (ui_state.current_screen != new_screen) {
        ui_state.previous_screen = ui_state.current_screen;
        ui_state.current_screen = new_screen;
        ui_state.screen_needs_redraw = true;
        ui_state.force_full_redraw = true;
        
        Serial.print(F("Cambio schermata: "));
        Serial.println(new_screen);
        
        // Aggiorna colori in base alla modalità
        updateUIColors();
    }
}

void updateUIColors() {
    if (system_state.emergency_mode) {
        ui_state.background_color = COLOR_EMERGENCY;
        ui_state.text_color = COLOR_WHITE;
        ui_state.accent_color = COLOR_YELLOW;
    } else {
        ui_state.background_color = COLOR_BLACK;
        ui_state.text_color = COLOR_WHITE;
        ui_state.accent_color = COLOR_GREEN;
    }
}

void updateLEDs() {
    // TODO: Implementazione completa nel prossimo step
}

void handleBuzzer() {
    // TODO: Implementazione completa nel prossimo step
}

void resetWatchdog() {
    wdt_reset();
}

void handleMillisOverflow() {
    // TODO: Implementazione gestione overflow millis()
}

// ===============================================================================
// FINE FILE PRINCIPALE
// =============================================================================== 