/*
 * STAGIONINO - Centralina Intelligente per Stagionatura Salumi
 * Versione 1.2 - Sistema Di stagionatura automatica con Display Nextion
 * 
 * Developed by: Arduino Framework Expert
 * Hardware: Arduino Mega 2560 + Nextion NX4832K035 Display + Sensori AM2315C/DHT11
 * 
 * Copyright (C) 2024 - Sistema di controllo ambientale per stagionatura salumi
 * 
 * Features:
 * - Sistema a Stati Finiti adattivo
 * - Modalità Automatica con programmi SD
 * - Modalità Manuale con controllo continuo
 * - Interfaccia Nextion 3.5" Touch
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

// Librerie Core Arduino
#include <Wire.h>                    // I2C (inclusa in Arduino)
#include <SPI.h>                     // SPI (inclusa in Arduino)
#include <EEPROM.h>                  // Memoria persistente (inclusa)
#include <avr/wdt.h>                 // Protezione watchdog (inclusa)

// Sensori e RTC
#include <Adafruit_AM2315.h>         // v2.1.0+ - Sensore AM2315C I2C
#include <DHT.h>                     // v1.4.4+ - Sensore DHT11
#include <RTClib.h>                  // v2.1.1+ - RTC DS1307

// Display Nextion
#include "nextion_protocol.h"        // Protocollo comunicazione Nextion

// LED e Storage
#include <FastLED.h>                 // v3.5.0+ - LED WS2812B
#include <SD.h>                      // v1.2.4+ - SD Card

// ===============================================================================
// CONFIGURAZIONE HARDWARE PINS
// ===============================================================================

// Display Nextion NX4832K035 (UART)
#define NEXTION_SERIAL    Serial1     // Usa Serial1 su Arduino Mega (TX1=18, RX1=19)
#define NEXTION_BAUD      9600        // Velocità comunicazione

// SD Card (SPI) - PIN OBBLIGATORI se si usa shield
#define SD_CS       4               // SD Card Chip Select

// Display Backlight Control (se disponibile)
#define BACKLIGHT_PIN    44         // Pin controllo retroilluminazione (PWM)

// Sensori - CONFIGURAZIONE OTTIMIZZATA PER ARDUINO MEGA
#define DHT_PIN     28              // DHT11 Data Pin
#define DHT_TYPE    DHT11           // Tipo sensore DHT
// AM2315C usa I2C: SDA=20, SCL=21 (obbligatori)
// DS1307 usa I2C: SDA=20, SCL=21 (obbligatori)

// LED Indicatori - CONFIGURAZIONE OTTIMIZZATA
#define LED_24BIT_PIN    30         // WS2812B 24 LED strip
#define LED_12BIT_PIN    32         // WS2812B 12 LED strip
#define NUM_LEDS_24      24         // Numero LED strip principale
#define NUM_LEDS_12      12         // Numero LED strip secondario

// Buzzer
#define BUZZER_PIN      34          // Buzzer passivo (PWM/tone)

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

// Stringhe costanti in PROGMEM per risparmiare RAM
const char PROGMEM str_system_name[] = "STAGIONINO V1.2";
const char PROGMEM str_system_desc[] = "Sistema Stagionatura Salumi";
const char PROGMEM str_initializing[] = "Inizializzazione...";
const char PROGMEM str_sensors[] = "SENSORI";
const char PROGMEM str_actuators[] = "ATTUATORI";
const char PROGMEM str_emergency[] = "EMERGENZA";
const char PROGMEM str_programs[] = "PROGRAMMI";
const char PROGMEM str_settings[] = "SETTINGS";

// Timing sistema (in millisecondi)
#define SENSOR_READ_INTERVAL     2000    // Lettura sensori ogni 2s
#define DISPLAY_UPDATE_INTERVAL  1000    // Aggiornamento display ogni 1s
#define LED_UPDATE_INTERVAL      100     // Aggiornamento LED ogni 100ms
#define CONTROL_CYCLE_INTERVAL   5000    // Ciclo controllo ogni 5s
#define DEMO_UPDATE_INTERVAL     3000    // Aggiornamento demo ogni 3s

// Limiti sensori per validazione
#define AM2315_TEMP_MIN          -20.0   // Temperatura minima AM2315C
#define AM2315_TEMP_MAX          80.0    // Temperatura massima AM2315C
#define AM2315_HUM_MIN           0.0     // Umidità minima AM2315C
#define AM2315_HUM_MAX           100.0   // Umidità massima AM2315C
#define DHT11_TEMP_MIN           0.0     // Temperatura minima DHT11
#define DHT11_TEMP_MAX           50.0    // Temperatura massima DHT11
#define DHT11_HUM_MIN            20.0    // Umidità minima DHT11
#define DHT11_HUM_MAX            90.0    // Umidità massima DHT11

// ===============================================================================
// STRUTTURE DATI SISTEMA
// ===============================================================================

// Struttura dati sensori con filtri
struct SensorData {
    // Sensore interno (AM2315C)
    float temp_internal;              // Temperatura interna filtrata
    float hum_internal;               // Umidità interna filtrata
    bool internal_valid;              // Sensore interno funzionante
    unsigned long internal_last_read; // Timestamp ultima lettura valida
    int internal_error_count;         // Contatore errori consecutivi
    
    // Sensore esterno (DHT11)
    float temp_external;              // Temperatura esterna filtrata
    float hum_external;               // Umidità esterna filtrata
    bool external_valid;              // Sensore esterno funzionante
    unsigned long external_last_read; // Timestamp ultima lettura valida
    int external_error_count;         // Contatore errori consecutivi
    
    // Timing e validazione
    unsigned long last_read_time;     // Ultimo tentativo lettura
    bool demo_mode;                   // Modalità demo attiva
    
    // Filtri digitali
    float temp_internal_filtered[3];  // Buffer filtro passa-basso
    float hum_internal_filtered[3];   // Buffer filtro passa-basso
    float temp_external_filtered[3];  // Buffer filtro passa-basso
    float hum_external_filtered[3];   // Buffer filtro passa-basso
    int filter_index;                 // Indice circolare filtro
};

// Enumerazione stati sistema
enum SystemState {
    STATE_STARTUP,                    // Avvio sistema
    STATE_MANUAL_CONTROL,             // Controllo manuale
    STATE_AUTO_PROGRAM,               // Programma automatico
    STATE_EMERGENCY,                  // Modalità emergenza
    STATE_MAINTENANCE,                // Manutenzione
    STATE_DEMO                        // Modalità demo
};

// Struttura stato principale sistema
struct MainSystemState {
    SystemState current_state;        // Stato corrente
    SystemState previous_state;       // Stato precedente
    unsigned long state_start_time;   // Inizio stato corrente
    bool emergency_mode;              // Flag emergenza attiva
    bool demo_mode_active;            // Demo mode attivo
    bool demo_mode_forced;            // Demo forzato da comando
    unsigned long last_activity;      // Ultima attività sistema
    
    // Controlli manuali
    bool manual_override;             // Override manuale attivo
    bool manual_frigorifero;          // Stato manuale frigorifero
    bool manual_riscaldatore;         // Stato manuale riscaldatore
    bool manual_deumidificatore;      // Stato manuale deumidificatore
    bool manual_umidificatore;        // Stato manuale umidificatore
    bool manual_ventola_in;           // Stato manuale ventola IN
    bool manual_ventola_out;          // Stato manuale ventola OUT
    
    // Sistema allarmi
    bool alarm_muted;                 // Allarmi silenziati
    unsigned long alarm_mute_until;   // Mute fino a timestamp
    bool critical_error;              // Errore critico rilevato
};

// Enumerazione schermate interfaccia
enum DisplayScreen {
    SCREEN_MAIN_DASHBOARD,            // Dashboard principale
    SCREEN_SENSOR_DATA,               // Dati sensori dettagliati
    SCREEN_SETTINGS,                  // Impostazioni sistema
    SCREEN_PROGRAMS,                  // Gestione programmi
    SCREEN_EMERGENCY,                 // Schermata emergenza
    SCREEN_DIAGNOSTIC                 // Test diagnostici
};

// Struttura interfaccia utente
struct UIState {
    DisplayScreen current_screen;     // Schermata corrente
    DisplayScreen previous_screen;    // Schermata precedente
    bool screen_needs_redraw;         // Flag ridisegno schermata
    bool force_full_redraw;           // Flag ridisegno completo
    unsigned long last_screen_update; // Ultimo aggiornamento schermata
    unsigned long last_touch_time;    // Ultimo tocco rilevato
};

// Struttura gestione retroilluminazione display
struct BacklightSystem {
    bool is_enabled;                  // Retroilluminazione abilitata
    uint8_t brightness_level;         // Livello luminosità (0-100)
    uint8_t day_brightness;           // Luminosità diurna
    uint8_t night_brightness;         // Luminosità notturna
    bool auto_dim_enabled;            // Auto-dimming abilitato
    unsigned long last_activity_time; // Ultima attività rilevata
    unsigned long auto_dim_delay;     // Delay auto-dimming (ms)
    bool is_dimmed;                   // Stato dimmed corrente
    bool fade_in_progress;            // Fade in corso
    uint8_t fade_target;              // Target luminosità fade
    unsigned long fade_start_time;    // Inizio fade
    unsigned long fade_duration;     // Durata fade (ms)
};

// Struttura sistema emergenza
struct EmergencySystem {
    bool is_active;                   // Emergenza attiva
    unsigned long activation_time;    // Timestamp attivazione
    int error_code;                   // Codice errore specifico
    char error_description[64];       // Descrizione errore
    bool auto_recovery_enabled;       // Recovery automatico abilitato
    int recovery_attempts;            // Tentativi recovery effettuati
    unsigned long last_recovery_time; // Ultimo tentativo recovery
    bool buzzer_active;               // Buzzer emergenza attivo
    unsigned long buzzer_last_toggle; // Ultimo toggle buzzer
};

// Enumerazione modalità LED
enum LEDMode {
    LED_MODE_NORMAL,                  // Modalità normale
    LED_MODE_EMERGENCY,               // Modalità emergenza
    LED_MODE_PROGRAM_RUNNING,         // Programma in esecuzione
    LED_MODE_STARTUP,                 // Avvio sistema
    LED_MODE_ERROR,                   // Errore sistema
    LED_MODE_MAINTENANCE             // Manutenzione
};

// Struttura gestione LED avanzata
struct LEDSystem {
    LEDMode current_mode;             // Modalità corrente
    bool leds_enabled;                // LED abilitati
    uint8_t brightness;               // Luminosità (0-255)
    bool animation_active;            // Animazione in corso
    unsigned long animation_start;    // Inizio animazione
    int animation_step;               // Step animazione corrente
    unsigned long last_update;        // Ultimo aggiornamento
    
    // Pattern LED
    int led_pattern;                  // Pattern corrente
    unsigned long pattern_duration;   // Durata pattern
    bool auto_cycle;                  // Ciclo automatico pattern
    
    // Indicatori status
    CRGB status_colors[6];            // Colori per ogni attuatore
    bool status_blink[6];             // Blink per ogni attuatore
    unsigned long last_blink_time;    // Ultimo blink
    bool blink_state;                 // Stato corrente blink
    
    // Effetti speciali
    bool rainbow_effect;              // Effetto arcobaleno
    uint8_t rainbow_hue;              // Hue arcobaleno corrente
    bool breathing_effect;            // Effetto breathing
    uint8_t breathing_value;          // Valore breathing corrente
    bool breathing_direction;         // Direzione breathing
};

// ===============================================================================
// DICHIARAZIONE OGGETTI HARDWARE
// ===============================================================================

// Sensori
Adafruit_AM2315 am2315;             // Sensore interno AM2315C (I2C)
DHT dht(DHT_PIN, DHT_TYPE);         // Sensore esterno DHT11

// RTC
RTC_DS1307 rtc;                     // Real Time Clock

// LED Strips
CRGB leds_24[NUM_LEDS_24];          // Array LED strip 24
CRGB leds_12[NUM_LEDS_12];          // Array LED strip 12

// ===============================================================================
// VARIABILI GLOBALI SISTEMA
// ===============================================================================

// Strutture dati principali
SensorData sensors;
MainSystemState system_state;
UIState ui_state;
BacklightSystem backlight_system;
EmergencySystem emergency_system;
LEDSystem led_system;

// Variabili temporali
unsigned long previous_sensor_time = 0;
unsigned long previous_display_time = 0;
unsigned long previous_led_time = 0;
unsigned long previous_control_time = 0;
unsigned long previous_demo_time = 0;

// Buffer per operazioni SD e seriali
char sd_buffer[256];
char serial_buffer[128];

// Variabili per gestione millis() overflow
unsigned long last_millis = 0;
bool millis_overflow_detected = false;

// ===============================================================================
// DICHIARAZIONE FUNZIONI
// ===============================================================================

// Inizializzazione
void setup();
void initializeSystemState();
void initializeHardware();
void initializeSensors();
void initializeDisplay();
void initializeRTC();
void initializeLEDs();
void initializeSD();
void initializeRelays();

// Loop principale e gestione stati
void loop();
void handleSystemStates();
void updateSystemState();

// Lettura sensori
void readSensors();
bool readAM2315();
bool readDHT11();
void updateSensorValidation();
bool validateSensorReading(float value, float min_val, float max_val);
void applySensorFilters();

// Gestione display e interfaccia
void updateDisplay();
void handleNextionEvents();
void updateDashboardData();
void switchToScreen(DisplayScreen screen);

// Controllo attuatori
void updateActuators();
void controlFrigorifero();
void controlRiscaldatore();
void controlDeumidificatore();
void controlUmidificatore();
void controlVentole();

// Sistema LED
void updateLEDs();
void updateLEDMode();
void updateNormalLEDs();
void updateEmergencyLEDs();
void updateProgramLEDs();
void updateErrorLEDs();
void updateMaintenanceLEDs();
void playStartupAnimation();
void applyRainbowEffect();
void applyBreathingEffect();

// Gestione emergenze
void checkEmergencyConditions();
void activateEmergency(int error_code, const char* description);
void deactivateEmergency();
void attemptEmergencyRecovery();
void updateEmergencySystem();

// Modalità demo
void updateDemoMode();
void generateDemoData();
bool isDemoModeActive();
void toggleDemoMode();
void updateDemoModeStatus();

// Sistema retroilluminazione
void initializeBacklightSystem();
void updateBacklightSystem();
void backlight_activity_detected();
void setBacklightBrightness(uint8_t brightness);
void fadeBacklight(uint8_t target_brightness, unsigned long duration);

// Controlli manuali
void setManualControl(bool enabled);
void setActuatorManual(int relay_pin, bool state);

// Gestione comandi seriali
void handleSerialCommands();
void processSerialCommand(String command);

// Utilità
void logSystemHealth();
void resetWatchdog();
void handleMillisOverflow();
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);

// ===============================================================================
// IMPLEMENTAZIONE SETUP
// ===============================================================================

void setup() {
    // Inizializzazione seriale per debug
    Serial.begin(115200);
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║          STAGIONINO v1.2             ║"));
    Serial.println(F("║    Sistema Stagionatura Salumi       ║"));
    Serial.println(F("║      *** NEXTION EDITION ***         ║"));
    Serial.println(F("║         Avvio Sistema...             ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    Serial.println(F(""));
    Serial.println(F("🎛️  COMANDI SERIALI DISPONIBILI:"));
    Serial.println(F("   - 'demo' → Attiva modalità demo"));
    Serial.println(F("   - 'nodemo' → Disattiva modalità demo"));
    Serial.println(F("   - 'status' → Mostra stato sistema"));
    Serial.println(F("   - 'backlight' → Test controllo retroilluminazione"));
    Serial.println(F("   - 'refresh' → Forza aggiornamento display"));
    Serial.println(F("   - 'nextion' → Test display Nextion"));
    Serial.println(F("   - 'sdtest' → Diagnosi SD card"));
    Serial.println(F(""));
    
    // Disabilita watchdog durante inizializzazione
    wdt_disable();
    
    // Inizializzazione strutture dati
    initializeSystemState();
    
    // Controllo precoce modalità demo per evitare blocchi
    Serial.println(F("Controllo sensori disponibili..."));
    Wire.begin();
    delay(100);
    
    // Test rapido AM2315 per decidere modalità demo
    if (!am2315.begin()) {
        Serial.println(F("⚠️  AM2315 non trovato - attivando modalità demo automatica"));
        system_state.demo_mode_active = true;
        system_state.demo_mode_forced = false; // Demo automatico, non forzato
    } else {
        Serial.println(F("✅ AM2315 rilevato - modalità normale"));
        system_state.demo_mode_active = false;
    }
    
    // Aggiorna stato demo
    updateDemoModeStatus();
    
    // Inizializzazione hardware
    Serial.println(F("Inizializzazione componenti hardware..."));
    initializeHardware();
    
    // Abilita watchdog (8 secondi timeout)
    wdt_enable(WDTO_8S);
    Serial.println(F("⏰ Watchdog abilitato (8s timeout)"));
    
    // Sistema pronto
    Serial.println(F(""));
    Serial.println(F("🚀 SISTEMA PRONTO"));
    Serial.println(F("📊 Modalità: ") + (isDemoModeActive() ? F("DEMO") : F("NORMALE")));
    Serial.println(F(""));
}

// ===============================================================================
// IMPLEMENTAZIONE LOOP PRINCIPALE
// ===============================================================================

void loop() {
    // Reset watchdog ogni ciclo
    wdt_reset();
    
    // Gestione overflow millis()
    handleMillisOverflow();
    
    // Lettura comandi seriali
    handleSerialCommands();
    
    // Gestione eventi Nextion
    handleNextionEvents();
    
    // Aggiornamento sensori (ogni 2 secondi)
    if (millis() - previous_sensor_time >= SENSOR_READ_INTERVAL) {
        readSensors();
        previous_sensor_time = millis();
    }
    
    // Aggiornamento display (ogni 1 secondo)
    if (millis() - previous_display_time >= DISPLAY_UPDATE_INTERVAL) {
        updateDisplay();
        previous_display_time = millis();
    }
    
    // Aggiornamento LED (ogni 100ms)
    if (millis() - previous_led_time >= LED_UPDATE_INTERVAL) {
        updateLEDs();
        previous_led_time = millis();
    }
    
    // Ciclo controllo sistema (ogni 5 secondi)
    if (millis() - previous_control_time >= CONTROL_CYCLE_INTERVAL) {
        handleSystemStates();
        updateActuators();
        checkEmergencyConditions();
        updateBacklightSystem();
        previous_control_time = millis();
    }
    
    // Aggiornamento modalità demo (ogni 3 secondi)
    if (isDemoModeActive() && millis() - previous_demo_time >= DEMO_UPDATE_INTERVAL) {
        updateDemoMode();
        previous_demo_time = millis();
    }
    
    // Aggiornamento sistema emergenza
    updateEmergencySystem();
    
    // Log salute sistema ogni ora
    static unsigned long lastHealthLog = 0;
    if (millis() - lastHealthLog >= 3600000) {
        logSystemHealth();
        lastHealthLog = millis();
    }
    
    // Piccolo delay per ottimizzare CPU
    delay(50);
}

// ===============================================================================
// FUNZIONI DI INIZIALIZZAZIONE
// ===============================================================================

void initializeSystemState() {
    Serial.println(F("-> Inizializzazione strutture dati sistema"));
    
    // Stato sistema principale
    system_state.current_state = STATE_STARTUP;
    system_state.previous_state = STATE_STARTUP;
    system_state.state_start_time = millis();
    system_state.emergency_mode = false;
    system_state.demo_mode_active = false;
    system_state.demo_mode_forced = false;
    system_state.last_activity = millis();
    system_state.manual_override = false;
    system_state.alarm_muted = false;
    system_state.alarm_mute_until = 0;
    system_state.critical_error = false;
    
    // Stato sensori
    sensors.temp_internal = 0.0;
    sensors.hum_internal = 0.0;
    sensors.internal_valid = false;
    sensors.internal_last_read = 0;
    sensors.internal_error_count = 0;
    
    sensors.temp_external = 0.0;
    sensors.hum_external = 0.0;
    sensors.external_valid = false;
    sensors.external_last_read = 0;
    sensors.external_error_count = 0;
    
    sensors.last_read_time = 0;
    sensors.demo_mode = false;
    sensors.filter_index = 0;
    
    // Azzeramento filtri
    for (int i = 0; i < 3; i++) {
        sensors.temp_internal_filtered[i] = 0.0;
        sensors.hum_internal_filtered[i] = 0.0;
        sensors.temp_external_filtered[i] = 0.0;
        sensors.hum_external_filtered[i] = 0.0;
    }
    
    // Stato interfaccia utente
    ui_state.current_screen = SCREEN_MAIN_DASHBOARD;
    ui_state.previous_screen = SCREEN_MAIN_DASHBOARD;
    ui_state.screen_needs_redraw = true;
    ui_state.force_full_redraw = true;
    ui_state.last_screen_update = 0;
    ui_state.last_touch_time = 0;
    
    // Sistema emergenza
    emergency_system.is_active = false;
    emergency_system.activation_time = 0;
    emergency_system.error_code = 0;
    strcpy(emergency_system.error_description, "");
    emergency_system.auto_recovery_enabled = true;
    emergency_system.recovery_attempts = 0;
    emergency_system.last_recovery_time = 0;
    emergency_system.buzzer_active = false;
    emergency_system.buzzer_last_toggle = 0;
    
    // Sistema LED
    led_system.current_mode = LED_MODE_STARTUP;
    led_system.leds_enabled = true;
    led_system.brightness = 128;
    led_system.animation_active = false;
    led_system.animation_start = 0;
    led_system.animation_step = 0;
    led_system.last_update = 0;
    led_system.led_pattern = 0;
    led_system.pattern_duration = 5000;
    led_system.auto_cycle = true;
    led_system.last_blink_time = 0;
    led_system.blink_state = false;
    led_system.rainbow_effect = false;
    led_system.rainbow_hue = 0;
    led_system.breathing_effect = false;
    led_system.breathing_value = 0;
    led_system.breathing_direction = true;
    
    // Inizializzazione sistema retroilluminazione
    initializeBacklightSystem();
    
    // Inizializzazione controlli manuali (tutti OFF)
    system_state.manual_frigorifero = false;
    system_state.manual_riscaldatore = false;
    system_state.manual_deumidificatore = false;
    system_state.manual_umidificatore = false;
    system_state.manual_ventola_in = false;
    system_state.manual_ventola_out = false;
    
    // Contatori errori sensori
    sensors.internal_error_count = 0;
    sensors.external_error_count = 0;
}

void initializeHardware() {
    Serial.println(F("-> Inizializzazione pin e componenti base"));
    
    // Inizializzazione pin relè (stato iniziale OFF)
    initializeRelays();
    
    // Inizializzazione sensori
    Serial.println(F("-> Inizializzazione sensori"));
    initializeSensors();
    
    // Inizializzazione display Nextion
    Serial.println(F("-> Inizializzazione display Nextion"));
    initializeDisplay();
    Serial.println(F("   Display inizializzato - continuando..."));
    
    wdt_reset(); // Reset watchdog dopo display
    
    // Inizializzazione RTC con timeout
    Serial.println(F("-> Inizializzazione RTC"));
    unsigned long rtc_start = millis();
    initializeRTC();
    Serial.print(F("   RTC completato in "));
    Serial.print(millis() - rtc_start);
    Serial.println(F("ms"));
    
    wdt_reset(); // Reset watchdog dopo RTC
    
    // Inizializzazione LED con timeout
    Serial.println(F("-> Inizializzazione LED"));
    unsigned long led_start = millis();
    initializeLEDs();
    Serial.print(F("   LED completato in "));
    Serial.print(millis() - led_start);
    Serial.println(F("ms"));
    
    wdt_reset(); // Reset watchdog dopo LED
    
    // Inizializzazione SD (con retry)
    Serial.println(F("-> Inizializzazione SD Card"));
    unsigned long sd_start = millis();
    initializeSD();
    Serial.print(F("   SD completato in "));
    Serial.print(millis() - sd_start);
    Serial.println(F("ms"));
    
    wdt_reset(); // Reset watchdog dopo SD
    
    Serial.println(F("✅ Tutti i componenti hardware inizializzati"));
}

void initializeSensors() {
    Serial.println(F("  -> Sensori temperatura/umidità"));
    
    // Inizializzazione AM2315C (I2C)
    if (!isDemoModeActive()) {
        if (am2315.begin()) {
            Serial.println(F("     AM2315C: OK"));
            sensors.internal_valid = true;
        } else {
            Serial.println(F("     AM2315C: ERRORE - sensore non trovato"));
            sensors.internal_valid = false;
            sensors.internal_error_count = 10; // Marca come non funzionante
        }
    } else {
        Serial.println(F("     AM2315C: MODALITÀ DEMO - simulazione attiva"));
        sensors.internal_valid = true; // In demo considera sempre valido
    }
    
    // Inizializzazione DHT11
    dht.begin();
    
    // Test lettura DHT11
    if (!isDemoModeActive()) {
        delay(2000); // DHT11 richiede tempo di stabilizzazione
        float test_temp = dht.readTemperature();
        if (!isnan(test_temp) && test_temp > 0 && test_temp < 50) {
            Serial.println(F("     DHT11: OK"));
            sensors.external_valid = true;
        } else {
            Serial.println(F("     DHT11: WARNING - lettura non valida"));
            sensors.external_valid = false;
            sensors.external_error_count = 5; // Alcuni errori tollerati
        }
    } else {
        Serial.println(F("     DHT11: MODALITÀ DEMO - simulazione attiva"));
        sensors.external_valid = true; // In demo considera sempre valido
    }
    
    // Prima lettura sensori
    readSensors();
}

void initializeDisplay() {
    Serial.println(F("  -> Display Nextion NX4832K035"));
    
    // Inizializza comunicazione Nextion
    if (!nextion.begin(NEXTION_BAUD)) {
        Serial.println(F("     ERRORE: Nextion non risponde"));
        // Continua comunque - il sistema può funzionare senza display
        return;
    }
    
    Serial.println(F("     Nextion: OK"));
    
    // Imposta pagina iniziale
    nextion.setPage(PAGE_DASHBOARD);
    
    // Mostra messaggio di avvio
    nextion.setText("t0", "STAGIONINO V1.2");
    nextion.setText("t1", "Sistema Stagionatura Salumi");
    nextion.setText("t2", "Inizializzazione...");
    
    // Aggiorna flag UI
    ui_state.screen_needs_redraw = true;
    ui_state.force_full_redraw = true;
}

void initializeRTC() {
    Serial.println(F("  -> Real Time Clock DS1307"));
    
    if (!rtc.begin()) {
        Serial.println(F("     RTC: ERRORE - non trovato"));
        return;
    }
    
    if (!rtc.isrunning()) {
        Serial.println(F("     RTC: WARNING - non in funzione, impostando data/ora"));
        // Imposta data/ora di compilazione se RTC non configurato
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    
    DateTime now = rtc.now();
    Serial.print(F("     RTC: OK - "));
    Serial.print(now.day());
    Serial.print(F("/"));
    Serial.print(now.month());
    Serial.print(F("/"));
    Serial.print(now.year());
    Serial.print(F(" "));
    Serial.print(now.hour());
    Serial.print(F(":"));
    Serial.println(now.minute());
}

void initializeLEDs() {
    Serial.println(F("  -> LED WS2812B (FastLED)"));
    
    // Configurazione LED strips
    FastLED.addLeds<WS2812B, LED_24BIT_PIN, GRB>(leds_24, NUM_LEDS_24);
    FastLED.addLeds<WS2812B, LED_12BIT_PIN, GRB>(leds_12, NUM_LEDS_12);
    
    // Luminosità iniziale
    FastLED.setBrightness(led_system.brightness);
    
    // Spegne tutti i LED
    fill_solid(leds_24, NUM_LEDS_24, CRGB::Black);
    fill_solid(leds_12, NUM_LEDS_12, CRGB::Black);
    FastLED.show();
    
    // Test LED
    Serial.println(F("     Test LED..."));
    
    // LED strip 24: sequenza rossa-verde-blu
    for (int i = 0; i < 3 && i < NUM_LEDS_24; i++) {
        leds_24[i] = (i == 0) ? CRGB::Red : (i == 1) ? CRGB::Green : CRGB::Blue;
    }
    
    // LED strip 12: sequenza rossa-verde-blu
    for (int i = 0; i < 3 && i < NUM_LEDS_12; i++) {
        leds_12[i] = (i == 0) ? CRGB::Red : (i == 1) ? CRGB::Green : CRGB::Blue;
    }
    
    FastLED.show();
    delay(1000);
    
    // Spegne LED test
    fill_solid(leds_24, NUM_LEDS_24, CRGB::Black);
    fill_solid(leds_12, NUM_LEDS_12, CRGB::Black);
    FastLED.show();
    
    Serial.println(F("     LED: OK"));
    
    // Avvia animazione di startup
    playStartupAnimation();
}

void initializeSD() {
    Serial.println(F("  -> SD Card"));
    
    // Inizializzazione pin CS
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    
    // Tentativo inizializzazione SD con retry
    bool sd_initialized = false;
    for (int retry = 0; retry < 3 && !sd_initialized; retry++) {
        if (retry > 0) {
            Serial.print(F("     Retry SD inizializzazione #"));
            Serial.println(retry);
            delay(500);
        }
        
        if (SD.begin(SD_CS)) {
            sd_initialized = true;
            Serial.println(F("     SD Card: OK"));
            
            // Test scrittura/lettura
            File test_file = SD.open("test.txt", FILE_WRITE);
            if (test_file) {
                test_file.println(F("Test Stagionino"));
                test_file.close();
                
                test_file = SD.open("test.txt");
                if (test_file) {
                    String content = test_file.readString();
                    test_file.close();
                    SD.remove("test.txt");
                    
                    if (content.indexOf("Test") >= 0) {
                        Serial.println(F("     SD R/W: OK"));
                    } else {
                        Serial.println(F("     SD R/W: ERRORE lettura"));
                    }
                } else {
                    Serial.println(F("     SD R/W: ERRORE apertura lettura"));
                }
            } else {
                Serial.println(F("     SD R/W: ERRORE apertura scrittura"));
            }
            
        } else {
            Serial.println(F("     SD Card: ERRORE inizializzazione"));
        }
    }
    
    if (!sd_initialized) {
        Serial.println(F("     SD Card: NON DISPONIBILE - continua senza"));
    }
}

void initializeRelays() {
    Serial.println(F("  -> Relè controllo attuatori"));
    
    // Configurazione pin relè come output
    pinMode(RELAY_FRIGORIFERO, OUTPUT);
    pinMode(RELAY_RISCALDATORE, OUTPUT);
    pinMode(RELAY_DEUMIDIFICATORE, OUTPUT);
    pinMode(RELAY_UMIDIFICATORE, OUTPUT);
    pinMode(RELAY_VENTOLA_IN, OUTPUT);
    pinMode(RELAY_VENTOLA_OUT, OUTPUT);
    
    // Stato iniziale: tutti OFF (logica invertita: HIGH = OFF)
    digitalWrite(RELAY_FRIGORIFERO, HIGH);
    digitalWrite(RELAY_RISCALDATORE, HIGH);
    digitalWrite(RELAY_DEUMIDIFICATORE, HIGH);
    digitalWrite(RELAY_UMIDIFICATORE, HIGH);
    digitalWrite(RELAY_VENTOLA_IN, HIGH);
    digitalWrite(RELAY_VENTOLA_OUT, HIGH);
    
    Serial.println(F("     Relè: Tutti spenti (stato sicuro)"));
    
    // Pin buzzer
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    
    // Pin backlight se disponibile
    if (BACKLIGHT_PIN > 0) {
        pinMode(BACKLIGHT_PIN, OUTPUT);
        analogWrite(BACKLIGHT_PIN, 200); // Luminosità iniziale
    }
    
    Serial.println(F("     Pin controllo: OK"));
}

void initializeBacklightSystem() {
    backlight_system.is_enabled = true;
    backlight_system.brightness_level = 80;
    backlight_system.day_brightness = 100;
    backlight_system.night_brightness = 30;
    backlight_system.auto_dim_enabled = true;
    backlight_system.last_activity_time = millis();
    backlight_system.auto_dim_delay = 30000; // 30 secondi
    backlight_system.is_dimmed = false;
    backlight_system.fade_in_progress = false;
    backlight_system.fade_target = 80;
    backlight_system.fade_start_time = 0;
    backlight_system.fade_duration = 1000;
}

// ===============================================================================
// [CONTINUA NELL'IMPLEMENTAZIONE...]
// ===============================================================================

// Placeholder per le altre funzioni - da implementare nelle successive iterazioni
void readSensors() {
    // TODO: Implementazione lettura sensori con demo mode
}

void updateDisplay() {
    // TODO: Aggiornamento display Nextion
}

void handleNextionEvents() {
    // TODO: Gestione eventi touch Nextion
}

void updateActuators() {
    // TODO: Controllo attuatori
}

void handleSystemStates() {
    // TODO: Gestione stati sistema
}

void updateLEDs() {
    // TODO: Aggiornamento LED
}

void checkEmergencyConditions() {
    // TODO: Controllo condizioni emergenza
}

void updateDemoMode() {
    // TODO: Aggiornamento modalità demo
}

void updateBacklightSystem() {
    // TODO: Gestione retroilluminazione
}

void updateEmergencySystem() {
    // TODO: Sistema emergenza
}

void handleSerialCommands() {
    // TODO: Comandi seriali
}

void logSystemHealth() {
    // TODO: Log stato sistema
}

void handleMillisOverflow() {
    // TODO: Gestione overflow millis()
}

// Funzioni LED
void updateLEDMode() {}
void updateNormalLEDs() {}
void updateEmergencyLEDs() {}
void updateProgramLEDs() {}
void updateErrorLEDs() {}
void updateMaintenanceLEDs() {}
void playStartupAnimation() {}
void applyRainbowEffect() {}
void applyBreathingEffect() {}

// Funzioni emergenza
void activateEmergency(int error_code, const char* description) {}
void deactivateEmergency() {}
void attemptEmergencyRecovery() {}

// Funzioni demo
void generateDemoData() {}
bool isDemoModeActive() { return system_state.demo_mode_active; }
void toggleDemoMode() {}
void updateDemoModeStatus() {}

// Funzioni backlight
void backlight_activity_detected() {}
void setBacklightBrightness(uint8_t brightness) {}
void fadeBacklight(uint8_t target_brightness, unsigned long duration) {}

// Funzioni controllo
void setManualControl(bool enabled) {}
void setActuatorManual(int relay_pin, bool state) {}

// Funzioni seriali
void processSerialCommand(String command) {}

// Funzioni sensori
bool readAM2315() { return false; }
bool readDHT11() { return false; }
void updateSensorValidation() {}
bool validateSensorReading(float value, float min_val, float max_val) { return true; }
void applySensorFilters() {}

// Funzioni display
void updateDashboardData() {}
void switchToScreen(DisplayScreen screen) {}

// Funzioni controllo attuatori
void controlFrigorifero() {}
void controlRiscaldatore() {}
void controlDeumidificatore() {}
void controlUmidificatore() {}
void controlVentole() {}

// Utilità
void resetWatchdog() { wdt_reset(); }
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void updateSystemState() {}
