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

// Touch XPT2046 (SPI condiviso) - PIN OBBLIGATORI SHIELD
#define TOUCH_CS    6               // Touch Chip Select (obbligatorio shield)
#define TOUCH_IRQ   7               // Touch Interrupt (obbligatorio shield)

// SD Card (SPI condiviso) - PIN OBBLIGATORI SHIELD
#define SD_CS       4               // SD Card Chip Select (obbligatorio shield)

// Display Backlight Control
#define BACKLIGHT_PIN    44         // Pin controllo retroilluminazione (PWM)

// Sensori - AGGIORNATI PER AREA 22-53
#define DHT_PIN     28              // DHT11 Data Pin (era pin 2)
#define DHT_TYPE    DHT11           // Tipo sensore DHT
// AM2315C usa I2C: SDA=20, SCL=21 (obbligatori)
// DS1307 usa I2C: SDA=20, SCL=21 (obbligatori)

// LED Indicatori - AGGIORNATI PER AREA 22-53
#define LED_24BIT_PIN    30         // WS2812B 24 LED strip (era pin 8)
#define LED_12BIT_PIN    32         // WS2812B 12 LED strip (era pin 9)
#define NUM_LEDS_24      24         // Numero LED strip principale
#define NUM_LEDS_12      12         // Numero LED strip secondario

// Buzzer - AGGIORNATO PER AREA 22-53
#define BUZZER_PIN      34          // Buzzer passivo (PWM/tone) (era pin 10)

// Relè Attuatori (logica invertita: LOW=ON, HIGH=OFF) - GIÀ CORRETTI
#define RELAY_FRIGORIFERO      22   // Relè 1 - Frigorifero
#define RELAY_RISCALDATORE     23   // Relè 2 - Riscaldatore  
#define RELAY_DEUMIDIFICATORE  24   // Relè 3 - Deumidificatore
#define RELAY_UMIDIFICATORE    25   // Relè 4 - Umidificatore
#define RELAY_VENTOLA_IN       26   // Relè 5 - Ventola Immissione
#define RELAY_VENTOLA_OUT      27   // Relè 6 - Ventola Estrazione

// ===============================================================================
// COSTANTI DI SISTEMA E STRINGHE PROGMEM
// ===============================================================================

// Stringhe costanti in PROGMEM per risparmiare RAM
const char PROGMEM str_system_name[] = "STAGIONINO V1.0";
const char PROGMEM str_system_desc[] = "Sistema Stagionatura Salumi";
const char PROGMEM str_initializing[] = "Inizializzazione...";
const char PROGMEM str_sensors[] = "SENSORI";
const char PROGMEM str_actuators[] = "ATTUATORI";
const char PROGMEM str_emergency[] = "EMERGENZA";
const char PROGMEM str_programs[] = "PROGRAMMI";
const char PROGMEM str_settings[] = "SETTINGS";
const char PROGMEM str_back[] = "BACK";
const char PROGMEM str_ok[] = "OK";
const char PROGMEM str_error[] = "ERRORE";
const char PROGMEM str_none[] = "Nessuno";

// ===============================================================================
// COSTANTI DI SISTEMA
// ===============================================================================

// Temporizzazioni ottimizzate per produzione (millisecondi)
#define SENSOR_READ_INTERVAL     15000    // Lettura sensori ogni 15s (era 30s)
#define DISPLAY_UPDATE_INTERVAL  2000     // Aggiornamento display ogni 2s (ottimizzato per UI reattiva)
#define CONTROL_FRIDGE_INTERVAL  120000   // Controllo frigo ogni 2min (era 8min)
#define CONTROL_HEATER_INTERVAL  90000    // Controllo riscaldatore ogni 1.5min (era 5min)
#define CONTROL_DEHUM_INTERVAL   60000    // Controllo deumidificatore ogni 1min (era 4min)
#define CONTROL_HUM_INTERVAL     60000    // Controllo umidificatore ogni 1min (era 3min)
#define CONTROL_FAN_INTERVAL     45000    // Controllo ventole ogni 45s (era 2min)

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

// Controllo retroilluminazione display
#define BACKLIGHT_TIMEOUT        30000    // Timeout retroilluminazione inattiva (30s)
#define BACKLIGHT_MIN_LEVEL      30       // Livello minimo retroilluminazione (0-255)
#define BACKLIGHT_MAX_LEVEL      255      // Livello massimo retroilluminazione (0-255)
#define BACKLIGHT_AUTO_DIM       120      // Livello automatico ridotto (0-255)

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

// Struttura dati sensori con filtri
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
    
    // Filtri per stabilità (media mobile)
    float temp_internal_filtered;     // Temperatura interna filtrata
    float hum_internal_filtered;      // Umidità interna filtrata
    float temp_buffer[3];             // Buffer per media mobile temperatura
    float hum_buffer[3];              // Buffer per media mobile umidità
    int buffer_index;                 // Indice buffer circolare
    bool filter_initialized;          // Flag inizializzazione filtro
};

// Struttura stato sistema
struct SystemState {
    bool emergency_mode;              // Modalità emergenza attiva
    bool sd_available;                // SD card disponibile
    bool rtc_available;               // RTC disponibile  
    bool am2315_available;            // AM2315C disponibile
    bool dht11_available;             // DHT11 disponibile
    bool demo_mode_forced;            // Modalità demo forzata manualmente
    bool demo_mode_active;            // Modalità demo attualmente attiva
    unsigned long uptime;             // Tempo di funzionamento
    unsigned long manual_mode_start;  // Tempo avvio modalità manuale (millis)
    unsigned long last_watchdog_reset; // Ultimo reset watchdog
};

// Enumerazione schermate interfaccia
enum DisplayScreen {
    SCREEN_MAIN_DASHBOARD,            // Dashboard principale
    SCREEN_SENSOR_DATA,               // Dati sensori dettagliati
    SCREEN_SETTINGS,                  // Impostazioni sistema
    SCREEN_PROGRAMS,                  // Gestione programmi
    SCREEN_EMERGENCY,                 // Schermata emergenza
    SCREEN_CALIBRATION,              // Calibrazione touch
    SCREEN_DIAGNOSTIC                // Test diagnostici produzione
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

// Struttura gestione retroilluminazione display
struct BacklightSystem {
    bool is_enabled;                  // Retroilluminazione abilitata
    uint8_t current_level;            // Livello corrente (0-255)
    uint8_t target_level;             // Livello target (0-255)
    unsigned long last_activity;     // Ultimo tocco/attività
    unsigned long last_update;       // Ultimo aggiornamento PWM
    bool auto_dim_enabled;            // Auto-dim abilitato
    bool manual_control;              // Controllo manuale attivo
    bool emergency_mode;              // Modalità emergenza (sempre acceso)
    
    // Profili preimpostati
    uint8_t day_level;                // Livello diurno
    uint8_t night_level;              // Livello notturno
    uint8_t standby_level;            // Livello standby
    
    // Fade progressivo
    bool fade_in_progress;            // Fade in corso
    unsigned long fade_start_time;    // Inizio fade
    uint8_t fade_start_level;         // Livello iniziale fade
    uint8_t fade_target_level;        // Livello finale fade
    unsigned long fade_duration;     // Durata fade (ms)
};

// Struttura controllo attuatori
struct ActuatorState {
    bool is_active;                   // Stato corrente (ON/OFF)
    unsigned long last_change_time;   // Ultimo cambio stato
    unsigned long total_on_time;      // Tempo totale acceso
    unsigned long total_off_time;     // Tempo totale spento
    bool protection_active;           // Protezione cicli minimi attiva
    int error_count;                  // Contatore errori
};

// Struttura controllo sistema completo
struct ControlSystem {
    ActuatorState frigorifero;        // Stato frigorifero
    ActuatorState riscaldatore;       // Stato riscaldatore
    ActuatorState deumidificatore;    // Stato deumidificatore
    ActuatorState umidificatore;      // Stato umidificatore
    ActuatorState ventola_in;         // Stato ventola immissione
    ActuatorState ventola_out;        // Stato ventola estrazione
    
    // Parametri di controllo
    float target_temp_min;            // Temperatura minima target
    float target_temp_max;            // Temperatura massima target
    float target_hum_min;             // Umidità minima target
    float target_hum_max;             // Umidità massima target
    
    // Isteresi per evitare oscillazioni
    float temp_hysteresis;            // Isteresi temperatura
    float hum_hysteresis;             // Isteresi umidità
    
    // Timing controlli
    unsigned long last_temp_control;  // Ultimo controllo temperatura
    unsigned long last_hum_control;   // Ultimo controllo umidità
    unsigned long last_vent_control;  // Ultimo controllo ventilazione
    
    // Flag abilitazione dispositivi
    bool dehumidifier_available;      // Deumidificatore disponibile
    bool humidifier_available;        // Umidificatore disponibile
    bool ventilation_available;       // Ventilazione disponibile
    
    // Modalità operative
    bool manual_mode;                 // Modalità manuale
    bool auto_mode;                   // Modalità automatica
    bool temp_only_mode;              // Solo controllo temperatura
};

// Struttura fase programma (ottimizzata)
struct ProgramPhase {
    char name[16];                    // Nome fase ridotto (16 char)
    float temp_min;                   // Temperatura minima
    float temp_max;                   // Temperatura massima
    float hum_min;                    // Umidità minima (-1 = non controllata)
    float hum_max;                    // Umidità massima (-1 = non controllata)
    unsigned long duration_hours;     // Durata in ore (0 = infinita)
    bool is_final_phase;              // True se fase finale/stagionatura
};

// Struttura programma completo (ottimizzata)
struct StagingProgram {
    char name[32];                    // Nome programma ridotto
    char description[64];             // Descrizione ridotta
    int total_phases;                 // Numero totale fasi
    ProgramPhase phases[15];          // Fasi ridotte (max 15 invece di 30)
    bool is_loaded;                   // Programma caricato in memoria
    bool is_valid;                    // Programma valido
};

// Struttura esecuzione programma
struct ProgramExecution {
    bool is_running;                  // Programma in esecuzione
    int current_phase;                // Fase corrente (0-based)
    unsigned long phase_start_time;   // Inizio fase corrente
    unsigned long program_start_time; // Inizio programma
    unsigned long total_elapsed_hours; // Ore totali trascorse
    StagingProgram* current_program;  // Puntatore al programma corrente
    bool auto_advance;                // Avanzamento automatico fasi
    bool phase_completed;             // Fase corrente completata
};

// Struttura gestione SD (ottimizzata memoria)
struct SDManager {
    bool is_available;                // SD disponibile
    bool retry_needed;                // Necessita retry
    int retry_count;                  // Contatore tentativi
    unsigned long last_retry_time;    // Ultimo tentativo
    char last_error[32];              // Ultimo errore (ridotto da 64)
    unsigned long last_operation_time; // Ultima operazione
    int programs_count;               // Numero programmi disponibili
    char program_list[10][32];        // Lista nomi programmi (ridotto: 10 programmi, 32 char)
};

// Enumerazione tipi emergenza
enum EmergencyType {
    EMERGENCY_NONE,                   // Nessuna emergenza
    EMERGENCY_SENSOR_INTERNAL,        // Sensore interno offline
    EMERGENCY_TEMP_CRITICAL,          // Temperatura critica
    EMERGENCY_TEMP_EXTERNAL_EXTREME,  // Temperatura esterna estrema
    EMERGENCY_POWER_ISSUES,           // Problemi alimentazione
    EMERGENCY_SD_FAILURE,             // Fallimento SD critico
    EMERGENCY_ACTUATOR_FAILURE        // Fallimento attuatori
};

// Struttura gestione emergenze (ottimizzata)
struct EmergencySystem {
    bool is_active;                   // Emergenza attiva
    EmergencyType current_type;       // Tipo emergenza corrente
    unsigned long start_time;         // Inizio emergenza
    unsigned long last_check_time;    // Ultimo controllo
    int trigger_count;                // Contatore trigger
    char description[64];             // Descrizione emergenza (ridotta)
    
    // Parametri recovery
    bool recovery_attempted;          // Recovery tentato
    int recovery_attempts;            // Tentativi recovery
    unsigned long last_recovery_time; // Ultimo tentativo recovery
    
    // Condizioni specifiche
    float critical_temp_threshold;    // Soglia temperatura critica
    float extreme_temp_low;           // Temperatura estrema bassa
    float extreme_temp_high;          // Temperatura estrema alta
    int sensor_failure_threshold;     // Soglia fallimenti sensore
    
    // Log emergenze
    unsigned long total_emergency_time; // Tempo totale in emergenza
    int emergency_episodes;           // Episodi emergenza totali
};

// Struttura allarmi avanzati
struct AlarmSystem {
    bool buzzer_enabled;              // Buzzer abilitato
    bool mute_active;                 // Mute temporaneo attivo
    unsigned long mute_start_time;    // Inizio mute
    unsigned long mute_duration;      // Durata mute
    
    // Pattern allarmi
    int alarm_pattern;                // Pattern corrente (0=off, 1=beep, 2=continuous)
    unsigned long last_beep_time;     // Ultimo beep
    int beep_count;                   // Contatore beep
    
    // Priorità allarmi
    bool high_priority_alarm;         // Allarme alta priorità
    bool critical_alarm;              // Allarme critico
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

// Variabili globali per coordinate touch (soluzione forum Arduino MCUFRIEND)
int pixel_x, pixel_y;
SystemState system_state;           // Stato sistema globale
TouchData touch_data;               // Dati gestione touch
UIState ui_state;                   // Stato interfaccia utente
ControlSystem control_system;       // Sistema di controllo
StagingProgram current_program;     // Programma corrente
ProgramExecution program_execution; // Esecuzione programma
SDManager sd_manager;               // Gestione SD card
EmergencySystem emergency_system;   // Sistema emergenze
AlarmSystem alarm_system;           // Sistema allarmi
LEDSystem led_system;               // Sistema LED avanzato
BacklightSystem backlight_system;   // Sistema retroilluminazione display

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
void applySensorFilters(float temp_raw, float hum_raw);
bool detectSensorSpike(float new_value, float filtered_value);

// Sistema di controllo
void updateControlSystem();
void controlTemperature();
void controlHumidity();
void controlVentilation();
void emergencyTemperatureControl();

// Gestione attuatori
void initializeControlSystem();
void initActuatorState(ActuatorState* actuator);
bool canActivateActuator(ActuatorState* actuator, int relay_pin, unsigned long min_on_time, unsigned long min_off_time);
bool canDeactivateActuator(ActuatorState* actuator, unsigned long min_on_time);
void activateActuator(ActuatorState* actuator, int relay_pin, bool activate);
void updateActuatorStatistics();
void logActuatorStats(const __FlashStringHelper* name, ActuatorState* actuator);

// Sistema emergenze avanzato
void initializeEmergencySystem();
void checkEmergencyConditions();
void triggerEmergency(EmergencyType type, const char* description);
void checkEmergencyRecovery();
void attemptEmergencyRecovery();
void enterEmergencyMode();
void exitEmergencyMode();
const char* getEmergencyTypeName(EmergencyType type);

// Sistema allarmi
void updateAlarmSystem();
void playAlarmBeep(int beep_count);
void muteAlarms(unsigned long duration_ms);
void toggleBuzzer();

// Interfaccia utente
void updateDisplay();
void handleTouch();
void updateLEDs();
void handleBuzzer();

// Sistema retroilluminazione display
void initializeBacklight();
void updateBacklight();
void setBacklightLevel(uint8_t level);
void fadeBacklightTo(uint8_t target_level, unsigned long duration_ms);
void backlight_activity_detected();
void toggleBacklight();
void setBacklightProfile(int profile); // 0=Day, 1=Night, 2=Standby
void backlight_emergency_mode(bool enable);

// Sistema LED avanzato
void playStartupAnimation();
void updateLEDMode();
void updateNormalLEDs();
void updateEmergencyLEDs();
void updateProgramLEDs();
void updateErrorLEDs();
void updateMaintenanceLEDs();
void updateSystemStatusLEDs();
ActuatorState* getActuatorState(int actuator_index);
const char* getLEDModeName(LEDMode mode);

// Effetti LED
void applyRainbowEffect();
void applyBreathingEffect();
void toggleLEDs();
void setBrightness(uint8_t brightness);

// Sistema LED Ring Adattivo
void updateManualModeLEDs();        // Modalità manuale: Ore funzionamento + Attuatori
void updateAutomaticModeLEDs();     // Modalità automatica: Ore rimanenti + Fasi
void updateManualHoursRing24();     // Ring 24: Ore funzionamento (conto in avanti)
void updateActuatorsRing12();       // Ring 12: Attuatori attivi (legenda colori)
void updateProgramRing24();         // Ring 24: Ore rimanenti (conto alla rovescia)
void updatePhaseCountdownRing12();  // Ring 12: Fasi programma

// Gestione pin SPI condivisi (Touch + SD)
void spi_select_touch();
void spi_select_sd();
void spi_deselect_all();
bool Touch_getXY(void);  // Funzione helper dal forum Arduino

// Sistema touch avanzato
bool processTouchDebounce();
void processValidTouch();
void handleDashboardTouch();
void handleSensorDataTouch();
void handleSettingsTouch();
void handleProgramsTouch();
void handleEmergencyTouch();
void handleCalibrationTouch();
void handleDiagnosticTouch();
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
void drawDiagnosticScreen();
void drawErrorScreen();
void drawStatusBar();
void updateScreenData();

// Gestione programmi e SD
void initializeProgramSystem();
bool checkSDAvailability();
bool retrySDOperation();
void loadProgramList();
bool loadProgram(const char* program_name);
bool parsePhase(String& line, ProgramPhase* phase);

// Esecuzione programmi
bool startProgram(const char* program_name);
void stopProgram();
void updateProgramExecution();
void advanceToNextPhase();
void applyPhaseParameters(int phase_index);
void logProgramStatus();

// Utilità
void resetWatchdog();
void handleMillisOverflow();
void saveSettings();
void loadSettings();

// Modalità demo
void toggleDemoMode();
bool isDemoModeActive();
void updateDemoModeStatus();

// Comandi seriali
void handleSerialCommands();

// Test e diagnostica produzione
void runDiagnosticTests();
void logSystemHealth();
void stressTestSensors();
void validateMemoryUsage();
void printProductionReport();

// ===============================================================================
// SETUP - INIZIALIZZAZIONE SISTEMA
// ===============================================================================

void setup() {
    // Inizializzazione seriale per debug
    Serial.begin(115200);
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║          STAGIONINO v1.2             ║"));
    Serial.println(F("║    Sistema Stagionatura Salumi       ║"));
    Serial.println(F("║         Avvio Sistema...             ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    Serial.println(F(""));
    Serial.println(F("🎛️  COMANDI SERIALI DISPONIBILI:"));
    Serial.println(F("   - 'demo' → Attiva modalità demo"));
    Serial.println(F("   - 'nodemo' → Disattiva modalità demo"));
    Serial.println(F("   - 'status' → Mostra stato sistema"));
    Serial.println(F("   - 'backlight' → Test controllo retroilluminazione"));
    Serial.println(F("   - 'refresh' → Forza aggiornamento display"));
    Serial.println(F("   - 'testdisplay' → Test diagnostico display ILI9486/9488"));
    Serial.println(F("   - 'testtouch' → Test diagnostico touchscreen XPT2046"));
    Serial.println(F("   - 'uiinfo' → Mostra stato interfaccia utente"));
    Serial.println(F("   - 'dashboard' → Forza disegno dashboard principale"));
    Serial.println(F("   - 'tfttest' → Test base display TFT (solo colori)"));
    Serial.println(F("   - 'sdtest' → Diagnosi SD card e bus SPI condiviso"));
    Serial.println(F(""));
    Serial.println(F("ℹ️  NOTA: Touch condivide pin con TFT+SD - gestione ottimizzata"));
    Serial.println(F(""));
    
    // Disabilita watchdog durante inizializzazione
    wdt_disable();
    
    // Inizializzazione strutture dati
    initializeSystemState();
    
    // Controllo precoce modalità demo per evitare blocchi
    Serial.println(F("Controllo sensori disponibili..."));
    delay(500); // Breve pausa per stabilizzare
    
    // Inizializzazione hardware step by step
    Serial.println(F("Inizializzazione hardware..."));
    initializeHardware();
    
    // Se nessun sensore disponibile, forza modalità demo immediatamente
    if (!system_state.am2315_available && !system_state.dht11_available) {
        Serial.println(F(""));
        Serial.println(F("⚠️  NESSUN SENSORE RILEVATO"));
        Serial.println(F("🎛️  ATTIVAZIONE AUTOMATICA MODALITÀ DEMO"));
        Serial.println(F("   Digita 'demo' per confermare o attendere 3 secondi..."));
        
        // Attendi comando o timeout
        unsigned long start_wait = millis();
        bool demo_confirmed = false;
        while (millis() - start_wait < 3000 && !demo_confirmed) {
            if (Serial.available()) {
                String early_cmd = Serial.readStringUntil('\n');
                early_cmd.trim();
                early_cmd.toLowerCase();
                if (early_cmd == "demo") {
                    demo_confirmed = true;
                    Serial.println(F("✅ Demo confermata da utente"));
                }
            }
            delay(100);
        }
        
        // Attiva modalità demo
        updateDemoModeStatus(); // Questo attiverà la demo automaticamente
        if (system_state.demo_mode_active) {
            // Inizializza dati demo
            sensors.temp_internal = 12.5;
            sensors.hum_internal = 60.0;
            sensors.temp_external = 15.0;
            sensors.hum_external = 55.0;
            sensors.internal_valid = true;
            sensors.external_valid = true;
            sensors.internal_error_count = 0;
            sensors.external_error_count = 0;
            sensors.last_read_time = millis();
            
            Serial.println(F("✅ MODALITÀ DEMO ATTIVATA"));
            Serial.println(F("   Sistema procederà con dati simulati"));
        }
    }
    
    // Abilita watchdog per protezione sistema
    Serial.println(F("Abilitazione protezione watchdog..."));
    wdt_enable(WDTO_8S);  // Watchdog 8 secondi (massimo disponibile)
    
    Serial.println(F("=== SISTEMA STAGIONINO PRONTO ==="));
    printSystemStatus();
    
    // Forza il ridisegno della schermata principale
    ui_state.screen_needs_redraw = true;
    ui_state.force_full_redraw = true;
    
    // Aggiornamento immediato del display per uscire dalla schermata di caricamento
    Serial.println(F("Aggiornamento display..."));
    updateDisplay();
    
    // Test diagnostici semplificati durante inizializzazione
    Serial.println(F("Test memoria sistema..."));
    validateMemoryUsage();
    Serial.println(F("=== SISTEMA STAGIONINO PRONTO ==="));
    Serial.println(F("Setup completato - avvio loop principale..."));
    delay(500); // Breve pausa prima del loop
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
    
    // Inizializzazione filtri sensori
    sensors.temp_internal_filtered = NAN;
    sensors.hum_internal_filtered = NAN;
    sensors.buffer_index = 0;
    sensors.filter_initialized = false;
    for (int i = 0; i < 3; i++) {
        sensors.temp_buffer[i] = NAN;
        sensors.hum_buffer[i] = NAN;
    }
    
    // Inizializzazione stato sistema
    system_state.emergency_mode = false;
    system_state.sd_available = false;
    system_state.rtc_available = false;
    system_state.am2315_available = false;
    system_state.dht11_available = false;
    system_state.demo_mode_forced = false;     // Modalità demo disabilitata di default
    system_state.demo_mode_active = false;     // Nessuna modalità demo attiva inizialmente
    system_state.uptime = millis();
    system_state.manual_mode_start = millis();  // Inizializza tempo modalità manuale
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
    
    // Inizializzazione sistema di controllo
    initializeControlSystem();
    
    // Inizializzazione gestione programmi e SD
    initializeProgramSystem();
    
    // Inizializzazione sistema emergenze e allarmi
    initializeEmergencySystem();
    
    // Inizializzazione sistema retroilluminazione
    backlight_system.is_enabled = true;
    backlight_system.current_level = BACKLIGHT_MAX_LEVEL;
    backlight_system.target_level = BACKLIGHT_MAX_LEVEL;
    backlight_system.last_activity = millis();
    backlight_system.last_update = millis();
    backlight_system.auto_dim_enabled = true;
    backlight_system.manual_control = false;
    backlight_system.emergency_mode = false;
    
    // Profili preimpostati
    backlight_system.day_level = BACKLIGHT_MAX_LEVEL;      // 255 (100%)
    backlight_system.night_level = BACKLIGHT_AUTO_DIM;     // 120 (47%)
    backlight_system.standby_level = BACKLIGHT_MIN_LEVEL;  // 30 (12%)
    
    // Fade
    backlight_system.fade_in_progress = false;
    backlight_system.fade_start_time = 0;
    backlight_system.fade_start_level = BACKLIGHT_MAX_LEVEL;
    backlight_system.fade_target_level = BACKLIGHT_MAX_LEVEL;
    backlight_system.fade_duration = 1000; // 1 secondo default
    
    Serial.println(F("Strutture dati sistema inizializzate"));
}

void initializeProgramSystem() {
    // Inizializzazione programma corrente
    current_program.is_loaded = false;
    current_program.is_valid = false;
    current_program.total_phases = 0;
    strcpy(current_program.name, "Nessuno");
    strcpy(current_program.description, "Nessun programma caricato");
    
    // Inizializzazione esecuzione programma
    program_execution.is_running = false;
    program_execution.current_phase = 0;
    program_execution.phase_start_time = 0;
    program_execution.program_start_time = 0;
    program_execution.total_elapsed_hours = 0;
    program_execution.current_program = nullptr;
    program_execution.auto_advance = true;
    program_execution.phase_completed = false;
    
    // Inizializzazione gestione SD
    sd_manager.is_available = system_state.sd_available;
    sd_manager.retry_needed = false;
    sd_manager.retry_count = 0;
    sd_manager.last_retry_time = 0;
    strcpy(sd_manager.last_error, "Nessun errore");
    sd_manager.last_operation_time = 0;
    sd_manager.programs_count = 0;
    
    Serial.println(F("Sistema programmi inizializzato"));
    
    // Carica lista programmi se SD disponibile
    if (sd_manager.is_available) {
        loadProgramList();
        if (sd_manager.programs_count > 0) {
            Serial.print(F("Trovati "));
            Serial.print(sd_manager.programs_count);
            Serial.println(F(" programmi disponibili"));
        }
    } else {
        Serial.println(F("SD non disponibile - Solo modalità manuale"));
    }
}

void initializeEmergencySystem() {
    // Inizializzazione sistema emergenze
    emergency_system.is_active = false;
    emergency_system.current_type = EMERGENCY_NONE;
    emergency_system.start_time = 0;
    emergency_system.last_check_time = millis();
    emergency_system.trigger_count = 0;
    strcpy(emergency_system.description, "Sistema normale");
    
    // Parametri recovery
    emergency_system.recovery_attempted = false;
    emergency_system.recovery_attempts = 0;
    emergency_system.last_recovery_time = 0;
    
    // Soglie critiche bilanciate per produzione
    emergency_system.critical_temp_threshold = 5.0;      // °C sotto target (bilanciato vs sbalzi)
    emergency_system.extreme_temp_low = -5.0;            // °C assoluta (più conservativo)
    emergency_system.extreme_temp_high = 35.0;           // °C assoluta (più conservativo)
    emergency_system.sensor_failure_threshold = 6;       // Fallimenti consecutivi (ridotto)
    
    // Statistiche
    emergency_system.total_emergency_time = 0;
    emergency_system.emergency_episodes = 0;
    
    // Inizializzazione sistema allarmi
    alarm_system.buzzer_enabled = true;                  // Buzzer abilitato di default
    alarm_system.mute_active = false;
    alarm_system.mute_start_time = 0;
    alarm_system.mute_duration = MUTE_ALARM_DURATION;    // 5 minuti default
    
    // Pattern allarmi
    alarm_system.alarm_pattern = 0;                      // Nessun allarme
    alarm_system.last_beep_time = 0;
    alarm_system.beep_count = 0;
    
    // Priorità
    alarm_system.high_priority_alarm = false;
    alarm_system.critical_alarm = false;
    
    Serial.println(F("Sistema emergenze inizializzato"));
    Serial.print(F("Soglie: Temp critica <"));
    Serial.print(emergency_system.critical_temp_threshold, 1);
    Serial.print(F("°C, Estrema "));
    Serial.print(emergency_system.extreme_temp_low, 1);
    Serial.print(F("-"));
    Serial.print(emergency_system.extreme_temp_high, 1);
    Serial.println(F("°C"));
}

void initializeControlSystem() {
    // Inizializzazione stati attuatori
    initActuatorState(&control_system.frigorifero);
    initActuatorState(&control_system.riscaldatore);
    initActuatorState(&control_system.deumidificatore);
    initActuatorState(&control_system.umidificatore);
    initActuatorState(&control_system.ventola_in);
    initActuatorState(&control_system.ventola_out);
    
    // Parametri di controllo default (esempio per salame)
    control_system.target_temp_min = 10.0;      // °C
    control_system.target_temp_max = 12.0;      // °C
    control_system.target_hum_min = 58.0;       // %
    control_system.target_hum_max = 62.0;       // %
    
    // Isteresi per stabilità
    control_system.temp_hysteresis = 1.0;       // ±1°C
    control_system.hum_hysteresis = 3.0;        // ±3%
    
    // Inizializzazione timing
    unsigned long current_time = millis();
    control_system.last_temp_control = current_time;
    control_system.last_hum_control = current_time;
    control_system.last_vent_control = current_time;
    
    // Dispositivi disponibili (configurabili)
    control_system.dehumidifier_available = true;
    control_system.humidifier_available = true;
    control_system.ventilation_available = true;
    
    // Modalità operative
    control_system.manual_mode = true;          // Inizia in manuale
    control_system.auto_mode = false;
    control_system.temp_only_mode = false;
    
    Serial.println(F("Sistema di controllo inizializzato"));
    Serial.print(F("Target: T="));
    Serial.print(control_system.target_temp_min, 1);
    Serial.print(F("-"));
    Serial.print(control_system.target_temp_max, 1);
    Serial.print(F("°C, H="));
    Serial.print(control_system.target_hum_min, 1);
    Serial.print(F("-"));
    Serial.print(control_system.target_hum_max, 1);
    Serial.println(F("%"));
}

void initActuatorState(ActuatorState* actuator) {
    actuator->is_active = false;
    actuator->last_change_time = millis();
    actuator->total_on_time = 0;
    actuator->total_off_time = 0;
    actuator->protection_active = false;
    actuator->error_count = 0;
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
    
    // Status sistema di controllo
    Serial.print(F("Modalità Controllo: "));
    if (control_system.manual_mode) Serial.print(F("MANUALE "));
    if (control_system.auto_mode) Serial.print(F("AUTOMATICA "));
    if (control_system.temp_only_mode) Serial.print(F("SOLO-TEMP "));
    Serial.println();
    
    if (system_state.am2315_available || system_state.dht11_available) {
        Serial.println(F("Sistema: Pronto per operazioni normali"));
    } else {
        Serial.println(F("ATTENZIONE: Nessun sensore disponibile!"));
    }
}

// ===============================================================================
// LOOP PRINCIPALE - CICLO INFINITO
// ===============================================================================

void loop() {
    // Debug: messaggio solo al primo ciclo
    static bool first_loop = true;
    if (first_loop) {
        Serial.println(F(">>> LOOP PRINCIPALE AVVIATO <<<"));
        first_loop = false;
    }
    
    // Reset watchdog ogni ciclo
    resetWatchdog();
    
    // Gestione overflow millis() (dopo ~50 giorni)
    handleMillisOverflow();
    
    // Gestisci comandi seriali
    handleSerialCommands();
    
    // Lettura sensori con intervallo ottimizzato
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
        readSensors();
        handleSensorErrors();
        lastSensorRead = millis();
    }
    
    // Aggiornamento esecuzione programmi
    updateProgramExecution();
    
    // Aggiornamento sistema di controllo
    updateControlSystem();
    
    // Controllo condizioni di emergenza
    checkEmergencyConditions();
    
    // Aggiornamento interfaccia utente
    static unsigned long lastDisplayUpdate = 0;
    static int force_updates = 5; // Forza aggiornamenti per i primi 5 cicli
    
    if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        // Forza ridisegno per i primi cicli dopo l'avvio
        if (force_updates > 0) {
            ui_state.screen_needs_redraw = true;
            ui_state.force_full_redraw = true;
            force_updates--;
            Serial.print(F("Forzando aggiornamento display "));
            Serial.println(6 - force_updates);
        }
        
        updateDisplay();
        lastDisplayUpdate = millis();
    }
    
    // Gestione input touch
    handleTouch();
    
    // Aggiornamento LED e buzzer
    updateLEDs();
    handleBuzzer();
    
    // Aggiornamento retroilluminazione display
    updateBacklight();
    
    // Log periodico salute sistema (ogni ora)
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
    Serial.println(F("   Display inizializzato - continuando..."));
    
    // Aggiorna display con progresso inizializzazione
    tft.setCursor(10, 50);
    tft.println(F("-> RTC..."));
    
    wdt_reset(); // Reset watchdog dopo display
    
    // Inizializzazione RTC con timeout
    Serial.println(F("-> Inizializzazione RTC"));
    unsigned long rtc_start = millis();
    initializeRTC();
    Serial.print(F("   RTC completato in "));
    Serial.print(millis() - rtc_start);
    Serial.println(F("ms"));
    
    // Aggiorna display
    tft.setCursor(10, 70);
    tft.println(F("-> LED..."));
    
    wdt_reset(); // Reset watchdog dopo RTC
    
    // Inizializzazione LED con timeout
    Serial.println(F("-> Inizializzazione LED"));
    unsigned long led_start = millis();
    initializeLEDs();
    Serial.print(F("   LED completato in "));
    Serial.print(millis() - led_start);
    Serial.println(F("ms"));
    
    // Aggiorna display
    tft.setCursor(10, 90);
    tft.println(F("-> SD Card..."));
    
    wdt_reset(); // Reset watchdog dopo LED
    
    // Inizializzazione SD (con retry)
    Serial.println(F("-> Inizializzazione SD Card"));
    unsigned long sd_start = millis();
    initializeSD();
    Serial.print(F("   SD completato in "));
    Serial.print(millis() - sd_start);
    Serial.println(F("ms"));
    
    // Aggiorna display
    tft.setCursor(10, 110);
    tft.println(F("-> Sistema pronto!"));
    
    wdt_reset(); // Reset watchdog dopo SD
    
    // Inizializzazione retroilluminazione display
    Serial.println(F("-> Inizializzazione retroilluminazione display"));
    unsigned long backlight_start = millis();
    initializeBacklight();
    Serial.print(F("   Backlight completato in "));
    Serial.print(millis() - backlight_start);
    Serial.println(F("ms"));
    wdt_reset(); // Reset watchdog dopo backlight
    
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
        system_state.dht11_available = false;
    }
    
    // Aggiorna status modalità demo
    updateDemoModeStatus();
    
    // Modalità DEMO se attiva (automatica o forzata)
    if (system_state.demo_mode_active) {
        Serial.println(F(""));
        Serial.println(F("╔══════════════════════════════════════╗"));
        Serial.println(F("║      ⚠️  MODALITÀ DEMO ATTIVA       ║"));
        if (system_state.demo_mode_forced) {
            Serial.println(F("║   MODALITÀ DEMO FORZATA MANUALMENTE  ║"));
        } else {
            Serial.println(F("║   NESSUN SENSORE RILEVATO            ║"));
        }
        Serial.println(F("║   USANDO DATI SIMULATI               ║"));
        Serial.println(F("╚══════════════════════════════════════╝"));
        Serial.println(F("Sistema continuerà con dati simulati per test display"));
        
        // Simula dati sensori per permettere il funzionamento
        sensors.temp_internal = 12.5;          // Temperatura simulata
        sensors.hum_internal = 60.0;           // Umidità simulata
        sensors.temp_external = 15.0;          // Temperatura esterna simulata
        sensors.hum_external = 55.0;           // Umidità esterna simulata
        sensors.internal_valid = true;         // Forza validità per evitare emergenze
        sensors.external_valid = true;         // Forza validità
        sensors.internal_error_count = 0;
        sensors.external_error_count = 0;
    }
}

void initializeDisplay() {
    Serial.println(F("  -> Inizializzazione Display ILI9486/ILI9488"));
    
    // IMPORTANTE: Gestione pin SPI condivisi (Touch + SD)
    // Assicura che tutti i CS siano HIGH prima dell'inizializzazione
    pinMode(TOUCH_CS, OUTPUT);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);  // Disabilita touch
    digitalWrite(SD_CS, HIGH);     // Disabilita SD
    delay(100); // Stabilizzazione
    
    // Identificazione automatica display
    uint16_t ID = tft.readID();
    Serial.print(F("     Display ID rilevato: 0x"));
    Serial.println(ID, HEX);
    
    // Gestione diversi controller basata su forum Arduino
    if (ID == 0xFFFF || ID == 0x0000) {
        Serial.println(F("     ATTENZIONE: Display non rilevato automaticamente!"));
        Serial.println(F("     Tentativo con ILI9486 (fallback)..."));
        ID = 0x9486; // Fallback per ILI9486
    } else if (ID == 0x9488) {
        Serial.println(F("     Display ILI9488 rilevato"));
    } else if (ID == 0x9486) {
        Serial.println(F("     Display ILI9486 rilevato"));
    } else {
        Serial.print(F("     Display controller sconosciuto: 0x"));
        Serial.println(ID, HEX);
        Serial.println(F("     Tentativo con ID rilevato..."));
    }
    
    tft.begin(ID);
    tft.setRotation(1); // Orientamento landscape
    
    // Test riempimento schermo per verificare funzionamento
    Serial.println(F("     Test display in corso..."));
    tft.fillScreen(0xFFFF); // Test schermo bianco
    delay(500);
    tft.fillScreen(0x0000); // Schermo nero
    delay(500);
    
    // Verifica risoluzione
    uint16_t width = tft.width();
    uint16_t height = tft.height();
    Serial.print(F("     Risoluzione: "));
    Serial.print(width);
    Serial.print(F("x"));
    Serial.println(height);
    
    if (width == 0 || height == 0) {
        Serial.println(F("     ERRORE: Risoluzione non valida - problema inizializzazione"));
        // Tentativo di re-inizializzazione
        delay(1000);
        tft.begin(0x9486); // Forza ILI9486
        width = tft.width();
        height = tft.height();
        Serial.print(F("     Risoluzione dopo retry: "));
        Serial.print(width);
        Serial.print(F("x"));
        Serial.println(height);
    }
    
    // Test display con messaggio di avvio
    tft.setTextColor(0xFFFF, 0x0000); // Bianco su nero
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println(F("STAGIONINO V1.2"));
    tft.setTextSize(1);
    tft.setCursor(10, 40);
    tft.println(F("Sistema Stagionatura Salumi"));
    tft.setCursor(10, 60);
    tft.println(F("Caricamento..."));
    
    // Indicatore visivo del controller rilevato
    tft.setCursor(10, 80);
    tft.print(F("Controller: 0x"));
    tft.println(ID, HEX);
    
    // Breve pausa per mostrare il messaggio
    delay(1000); // Ridotto da 2000ms a 1000ms per velocizzare
    
    // Cancella schermata di caricamento e mostra stato
    tft.fillScreen(0x0000); // Schermo nero
    tft.setTextColor(0xFFFF, 0x0000);
    tft.setTextSize(1);
    tft.setCursor(10, 10);
    tft.println(F("STAGIONINO V1.2 - Inizializzazione..."));
    tft.setCursor(10, 30);
    tft.print(F("Display OK - Controller: 0x"));
    tft.println(ID, HEX);
    
    Serial.print(F("     Display: OK - Risoluzione: "));
    Serial.print(tft.width());
    Serial.print(F("x"));
    Serial.println(tft.height());
    
    // Inizializzazione touchscreen con gestione pin condivisi
    Serial.println(F("  -> Inizializzazione Touchscreen XPT2046"));
    
    // Assicura che SD sia disabilitata durante init touch
    digitalWrite(SD_CS, HIGH);
    delay(50);
    
    touch.begin();
    touch.setRotation(1); // Stesso orientamento del display
    
    // Test touch con gestione conflitti SPI
    bool touch_working = false;
    digitalWrite(TOUCH_CS, LOW);  // Seleziona touch
    delay(10);
    if (touch.tirqTouched()) {
        Serial.println(F("     Touchscreen: Tocco rilevato durante init"));
        touch_working = true;
    }
    digitalWrite(TOUCH_CS, HIGH); // Deseleziona touch
    
    if (touch_working) {
        Serial.println(F("     Touchscreen: OK (touch attivo)"));
    } else {
        Serial.println(F("     Touchscreen: OK (nessun tocco)"));
    }
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
    
    // Inizializzazione sistema LED avanzato
    led_system.current_mode = LED_MODE_STARTUP;
    led_system.leds_enabled = true;
    led_system.brightness = 128;                  // 50% luminosità default
    led_system.animation_active = false;
    led_system.animation_start = 0;
    led_system.animation_step = 0;
    led_system.last_update = millis();
    
    // Pattern LED
    led_system.led_pattern = 0;
    led_system.pattern_duration = 2000;           // 2 secondi default
    led_system.auto_cycle = false;
    
    // Inizializza colori attuatori
    led_system.status_colors[0] = CRGB::Blue;     // Frigorifero
    led_system.status_colors[1] = CRGB::Red;      // Riscaldatore  
    led_system.status_colors[2] = CRGB::Orange;   // Deumidificatore
    led_system.status_colors[3] = CRGB::Cyan;     // Umidificatore
    led_system.status_colors[4] = CRGB::Green;    // Ventola IN
    led_system.status_colors[5] = CRGB::Green;    // Ventola OUT
    
    // Reset blink status
    for (int i = 0; i < 6; i++) {
        led_system.status_blink[i] = false;
    }
    led_system.last_blink_time = millis();
    led_system.blink_state = false;
    
    // Effetti speciali
    led_system.rainbow_effect = false;
    led_system.rainbow_hue = 0;
    led_system.breathing_effect = false;
    led_system.breathing_value = 0;
    led_system.breathing_direction = true;
    
    // Impostazione luminosità globale
    FastLED.setBrightness(led_system.brightness);
    
    // Sequenza avvio animata
    Serial.println(F("     Sequenza avvio LED..."));
    playStartupAnimation();
    
    // Passa a modalità normale (forza NORMAL per evitare ERROR durante init)
    led_system.current_mode = LED_MODE_NORMAL;
    
    Serial.print(F("     LED Strip 24bit: "));
    Serial.print(NUM_LEDS_24);
    Serial.println(F(" LED - OK"));
    Serial.print(F("     LED Strip 12bit: "));
    Serial.print(NUM_LEDS_12);
    Serial.println(F(" LED - OK"));
}

void initializeSD() {
    Serial.println(F("  -> Inizializzazione SD Card (pin SPI condivisi)"));
    
    // IMPORTANTE: Gestione pin SPI condivisi
    // Disabilita touch prima di inizializzare SD
    digitalWrite(TOUCH_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
    delay(100); // Stabilizzazione bus SPI
    
    bool sd_ok = false;
    
    // Tentativi multipli con timeout per evitare blocchi
    for (int retry = 0; retry < SD_RETRY_COUNT; retry++) {
        Serial.print(F("     Tentativo SD "));
        Serial.print(retry + 1);
        Serial.print(F("/"));
        Serial.print(SD_RETRY_COUNT);
        Serial.print(F(" (CS="));
        Serial.print(SD_CS);
        Serial.println(F(")"));
        
        // Assicura stato bus SPI prima del tentativo
        digitalWrite(TOUCH_CS, HIGH);  // Disabilita touch
        digitalWrite(SD_CS, HIGH);     // Reset SD CS
        delay(50);
        
        // Timeout per inizializzazione SD (max 3 secondi)
        unsigned long start_time = millis();
        bool init_success = false;
        
        // Prova inizializzazione con timeout
        wdt_reset(); // Reset watchdog prima dell'operazione
        if (SD.begin(SD_CS)) {
            init_success = true;
        }
        
        unsigned long elapsed = millis() - start_time;
        
        if (init_success && elapsed < 3000) {
            sd_ok = true;
            Serial.print(F("     SD Card: OK ("));
            Serial.print(elapsed);
            Serial.println(F("ms)"));
            break;
        } else {
            Serial.print(F("     SD Card: Fallito ("));
            Serial.print(elapsed);
            Serial.println(F("ms)"));
            
            // Delay ridotto per tentativi successivi
            if (retry < SD_RETRY_COUNT - 1) {
                delay(500); // Ridotto da 1000ms
                wdt_reset();
            }
        }
    }
    
    if (!sd_ok) {
        Serial.println(F("     ERRORE: SD Card non disponibile!"));
        Serial.println(F("     Sistema continuerà in modalità limitata"));
        Serial.println(F("     - Solo modalità manuale disponibile"));
        Serial.println(F("     - Nessun salvataggio programmi"));
        system_state.sd_available = false;
        
        // IMPORTANTE: Reset bus SPI dopo fallimento SD
        // Questo evita interferenze con TFT
        Serial.println(F("     🔄 Ripulitura bus SPI per TFT..."));
        digitalWrite(SD_CS, HIGH);    // Disabilita definitivamente SD
        digitalWrite(TOUCH_CS, HIGH); // Assicura touch disabilitato
        delay(100);
        
        // Reset SPI per TFT
        SPI.end();
        delay(50);
        SPI.begin();
        delay(50);
        
        Serial.println(F("     Bus SPI ripulito per TFT"));
        return;
    }
    
    system_state.sd_available = true;
    
    // Verifica informazioni SD (metodi semplificati per compatibilità)
    Serial.print(F("     Tipo SD: "));
    Serial.println(F("Compatibile"));
    
    // Verifica spazio disponibile tramite file system
    File root = SD.open("/");
    if (root) {
        Serial.println(F("     SD card inizializzata correttamente"));
        root.close();
    } else {
        Serial.println(F("     Errore accesso root directory"));
    }
    
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
    
    // MODALITÀ DEMO: Se modalità demo attiva, usa dati simulati
    if (isDemoModeActive()) {
        // Simula dati sensori con piccole variazioni per realismo
        static float temp_offset = 0.0;
        static float hum_offset = 0.0;
        static unsigned long last_change = 0;
        
        // Cambia i valori lentamente ogni 30 secondi
        if (millis() - last_change > 30000) {
            temp_offset += random(-20, 21) / 100.0;  // ±0.2°C
            hum_offset += random(-50, 51) / 100.0;   // ±0.5%
            
            // Limita le variazioni
            temp_offset = constrain(temp_offset, -2.0, 2.0);
            hum_offset = constrain(hum_offset, -5.0, 5.0);
            
            last_change = millis();
        }
        
        // Applica variazioni ai dati base
        sensors.temp_internal = 12.5 + temp_offset;
        sensors.hum_internal = 60.0 + hum_offset;
        sensors.temp_external = 15.0 + temp_offset * 0.8;
        sensors.hum_external = 55.0 + hum_offset * 0.9;
        
        // Forza validità per evitare emergenze
        sensors.internal_valid = true;
        sensors.external_valid = true;
        sensors.internal_error_count = 0;
        sensors.external_error_count = 0;
        
        // Aggiorna timestamp
        sensors.last_read_time = millis();
        
        return true;  // Successo simulato
    }
    
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
                    
                    // Applica filtri per rilevare spike e stabilizzare
                    if (!detectSensorSpike(temp_int, sensors.temp_internal_filtered) &&
                        !detectSensorSpike(hum_int, sensors.hum_internal_filtered)) {
                        
                        applySensorFilters(temp_int, hum_int);
                        sensors.temp_internal = sensors.temp_internal_filtered;
                        sensors.hum_internal = sensors.hum_internal_filtered;
                        sensors.internal_valid = true;
                        sensors.internal_error_count = 0;
                        am2315_success = true;
                        break;
                    } else {
                        Serial.println(F("SPIKE: Dati AM2315C scartati (spike rilevato)"));
                    }
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
    
    // MODALITÀ DEMO: Se modalità demo attiva, accetta sempre i dati simulati
    if (isDemoModeActive()) {
        return sensors.internal_valid && sensors.external_valid;
    }
    
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
    // MODALITÀ DEMO: Non gestire errori se in modalità demo
    if (isDemoModeActive()) {
        return;  // Skip gestione errori in modalità demo
    }
    
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
    // Non eseguire controlli in modalità emergenza (eccetto frigorifero)
    if (system_state.emergency_mode) {
        emergencyTemperatureControl();
        return;
    }
    
    // Verifica che abbiamo dati sensori validi
    if (!validateSensorData()) {
        Serial.println(F("CONTROLLO: Dati sensori non validi, skip controllo"));
        return;
    }
    
    // Controllo temperatura (priorità massima)
    controlTemperature();
    
    // Controllo umidità (solo se non in modalità solo temperatura)
    if (!control_system.temp_only_mode) {
        controlHumidity();
    }
    
    // Controllo ventilazione (se disponibile)
    if (control_system.ventilation_available) {
        controlVentilation();
    }
    
    // Aggiorna statistiche attuatori
    updateActuatorStatistics();
}

void emergencyTemperatureControl() {
    // Controllo temperatura di emergenza: mantieni ~4°C
    const float EMERGENCY_TARGET = 4.0;
    const float EMERGENCY_HYSTERESIS = 1.0;
    
    if (sensors.internal_valid) {
        // Temperature troppo alta - accendi frigorifero
        if (sensors.temp_internal > EMERGENCY_TARGET + EMERGENCY_HYSTERESIS) {
            if (canActivateActuator(&control_system.frigorifero, RELAY_FRIGORIFERO, 
                                   MIN_FRIDGE_ON_TIME, MIN_FRIDGE_OFF_TIME)) {
                activateActuator(&control_system.frigorifero, RELAY_FRIGORIFERO, true);
                Serial.println(F("EMERGENZA: Frigorifero ON"));
            }
        }
        // Temperatura OK - spegni frigorifero
        else if (sensors.temp_internal < EMERGENCY_TARGET - EMERGENCY_HYSTERESIS) {
            if (control_system.frigorifero.is_active) {
                if (canDeactivateActuator(&control_system.frigorifero, MIN_FRIDGE_ON_TIME)) {
                    activateActuator(&control_system.frigorifero, RELAY_FRIGORIFERO, false);
                    Serial.println(F("EMERGENZA: Frigorifero OFF"));
                }
            }
        }
    }
    
    // Se temperatura esterna < interna, attiva ventola immissione
    if (sensors.internal_valid && sensors.external_valid) {
        if (sensors.temp_external < sensors.temp_internal - 2.0) {
            if (canActivateActuator(&control_system.ventola_in, RELAY_VENTOLA_IN, 
                                   MIN_FAN_ON_TIME, MIN_FAN_OFF_TIME)) {
                activateActuator(&control_system.ventola_in, RELAY_VENTOLA_IN, true);
                Serial.println(F("EMERGENZA: Ventola immissione ON"));
            }
        } else {
            if (control_system.ventola_in.is_active) {
                if (canDeactivateActuator(&control_system.ventola_in, MIN_FAN_ON_TIME)) {
                    activateActuator(&control_system.ventola_in, RELAY_VENTOLA_IN, false);
                    Serial.println(F("EMERGENZA: Ventola immissione OFF"));
                }
            }
        }
    }
}

void checkEmergencyConditions() {
    unsigned long current_time = millis();
    emergency_system.last_check_time = current_time;
    
    // Controllo mute allarmi (gestione overflow millis)
    if (alarm_system.mute_active) {
        if (current_time < alarm_system.mute_start_time) {
            // Overflow millis rilevato
            alarm_system.mute_start_time = current_time;
        }
        
        if (current_time - alarm_system.mute_start_time >= alarm_system.mute_duration) {
            alarm_system.mute_active = false;
            Serial.println(F("Mute allarmi scaduto"));
        }
    }
    
    // Se già in emergenza, controlla condizioni di recovery
    if (emergency_system.is_active) {
        checkEmergencyRecovery();
        updateAlarmSystem();
        return;
    }
    
    // === CONTROLLI EMERGENZE MULTIPLE ===
    
    // 1. Sensore interno critico
    if (sensors.internal_error_count >= emergency_system.sensor_failure_threshold) {
        triggerEmergency(EMERGENCY_SENSOR_INTERNAL, 
                        "Sensore interno offline da troppo tempo");
        return;
    }
    
    // 2. Temperatura critica relativa al target
    if (sensors.internal_valid) {
        float target_avg = (control_system.target_temp_min + control_system.target_temp_max) / 2.0;
        if (sensors.temp_internal < target_avg - emergency_system.critical_temp_threshold) {
            snprintf(emergency_system.description, sizeof(emergency_system.description),
                    "Temperatura critica: %.1f°C (%.1f°C sotto target)", 
                    sensors.temp_internal, target_avg - sensors.temp_internal);
            triggerEmergency(EMERGENCY_TEMP_CRITICAL, emergency_system.description);
            return;
        }
    }
    
    // 3. Temperature estreme assolute (interno o esterno)
    if (sensors.internal_valid) {
        if (sensors.temp_internal < emergency_system.extreme_temp_low || 
            sensors.temp_internal > emergency_system.extreme_temp_high) {
            snprintf(emergency_system.description, sizeof(emergency_system.description),
                    "Temperatura interna estrema: %.1f°C", sensors.temp_internal);
            triggerEmergency(EMERGENCY_TEMP_EXTERNAL_EXTREME, emergency_system.description);
            return;
        }
    }
    
    if (sensors.external_valid) {
        if (sensors.temp_external < -20.0 || sensors.temp_external > 50.0) {
            snprintf(emergency_system.description, sizeof(emergency_system.description),
                    "Temperatura esterna estrema: %.1f°C", sensors.temp_external);
            triggerEmergency(EMERGENCY_TEMP_EXTERNAL_EXTREME, emergency_system.description);
            return;
        }
    }
    
    // 4. Fallimento SD critico durante programma automatico
    if (program_execution.is_running && !sd_manager.is_available && 
        sd_manager.retry_count >= SD_RETRY_COUNT) {
        triggerEmergency(EMERGENCY_SD_FAILURE, 
                        "SD card fallita durante esecuzione programma");
        return;
    }
    
    // 5. Problemi attuatori (esempio: tutti i relè non rispondono)
    // TODO: Implementare monitoraggio feedback attuatori se disponibile
    
    // Aggiorna pattern allarmi per condizioni borderline
    updateAlarmSystem();
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
    
    // Attiva modalità emergenza retroilluminazione (sempre al massimo)
    backlight_emergency_mode(true);
    
    // Forza cambio alla schermata di emergenza
    switchToScreen(SCREEN_EMERGENCY);
}

void triggerEmergency(EmergencyType type, const char* description) {
    // Se già in emergenza dello stesso tipo, ignora
    if (emergency_system.is_active && emergency_system.current_type == type) {
        emergency_system.trigger_count++;
        return;
    }
    
    // Nuova emergenza
    emergency_system.is_active = true;
    emergency_system.current_type = type;
    emergency_system.start_time = millis();
    emergency_system.trigger_count = 1;
    emergency_system.recovery_attempted = false;
    emergency_system.recovery_attempts = 0;
    emergency_system.emergency_episodes++;
    strncpy(emergency_system.description, description, sizeof(emergency_system.description) - 1);
    
    // Attiva modalità emergenza sistema
    system_state.emergency_mode = true;
    
    // Configura allarmi basati su priorità
    switch (type) {
        case EMERGENCY_SENSOR_INTERNAL:
        case EMERGENCY_TEMP_CRITICAL:
            alarm_system.critical_alarm = true;
            alarm_system.alarm_pattern = 2; // Continuo
            break;
            
        case EMERGENCY_TEMP_EXTERNAL_EXTREME:
        case EMERGENCY_SD_FAILURE:
            alarm_system.high_priority_alarm = true;
            alarm_system.alarm_pattern = 1; // Beep intermittente
            break;
            
        default:
            alarm_system.alarm_pattern = 1;
            break;
    }
    
    // Log emergenza
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║      🚨 EMERGENZA ATTIVATA 🚨       ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    Serial.print(F("Tipo: "));
    Serial.println(getEmergencyTypeName(type));
    Serial.print(F("Descrizione: "));
    Serial.println(description);
    Serial.println(F(""));
    
    // Entra in modalità emergenza sistema
    enterEmergencyMode();
}

void checkEmergencyRecovery() {
    if (!emergency_system.is_active) {
        return;
    }
    
    unsigned long current_time = millis();
    bool can_recover = false;
    
    // Controlli recovery specifici per tipo emergenza
    switch (emergency_system.current_type) {
        case EMERGENCY_SENSOR_INTERNAL:
            // Recovery se sensore torna funzionante
            can_recover = sensors.internal_valid && sensors.internal_error_count < 3;
            break;
            
        case EMERGENCY_TEMP_CRITICAL:
            // Recovery se temperatura torna in range accettabile
            if (sensors.internal_valid) {
                float target_avg = (control_system.target_temp_min + control_system.target_temp_max) / 2.0;
                can_recover = sensors.temp_internal > target_avg - (emergency_system.critical_temp_threshold / 2.0);
            }
            break;
            
        case EMERGENCY_TEMP_EXTERNAL_EXTREME:
            // Recovery se temperature tornano in range
            can_recover = true;
            if (sensors.internal_valid) {
                if (sensors.temp_internal < emergency_system.extreme_temp_low + 2.0 || 
                    sensors.temp_internal > emergency_system.extreme_temp_high - 2.0) {
                    can_recover = false;
                }
            }
            break;
            
        case EMERGENCY_SD_FAILURE:
            // Recovery se SD torna disponibile
            can_recover = sd_manager.is_available;
            break;
            
        default:
            can_recover = false;
            break;
    }
    
    // Tentativo recovery se condizioni soddisfatte
    if (can_recover) {
        // Attendi almeno 2 minuti prima del primo tentativo
        if (current_time - emergency_system.start_time >= 120000 && 
            current_time - emergency_system.last_recovery_time >= 60000) {
            
            attemptEmergencyRecovery();
        }
    }
    
    // Aggiorna statistiche tempo emergenza
    emergency_system.total_emergency_time = current_time - emergency_system.start_time;
}

void attemptEmergencyRecovery() {
    emergency_system.recovery_attempted = true;
    emergency_system.recovery_attempts++;
    emergency_system.last_recovery_time = millis();
    
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║      🔄 TENTATIVO RECOVERY 🔄       ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    Serial.print(F("Tentativo: "));
    Serial.print(emergency_system.recovery_attempts);
    Serial.print(F(" per "));
    Serial.println(getEmergencyTypeName(emergency_system.current_type));
    
    // Recovery specifico per tipo con controlli più rigorosi
    bool recovery_success = false;
    int validation_count = 0;
    
    switch (emergency_system.current_type) {
        case EMERGENCY_SENSOR_INTERNAL:
            // Verifica sensore con multiple letture per conferma
            for (int i = 0; i < 3; i++) {
                delay(1000);
                wdt_reset();
                readSensors(); // Rileggi sensori
                if (sensors.internal_valid && sensors.internal_error_count == 0) {
                    validation_count++;
                }
            }
            recovery_success = (validation_count >= 2); // 2 su 3 letture valide
            Serial.print(F("Validazioni sensore: "));
            Serial.print(validation_count);
            Serial.println(F("/3"));
            break;
            
        case EMERGENCY_TEMP_CRITICAL:
            // Verifica temperatura stabile e trend migliorativo
            if (sensors.internal_valid) {
                float target_avg = (control_system.target_temp_min + control_system.target_temp_max) / 2.0;
                float current_deviation = abs(sensors.temp_internal - target_avg);
                unsigned long emergency_duration = millis() - emergency_system.start_time;
                
                                 // Recovery se deviazione ridotta E tempo minimo trascorso
                 recovery_success = (current_deviation < emergency_system.critical_temp_threshold * 0.8) && 
                                  (emergency_duration >= 180000); // Min 3 minuti
                 
                 Serial.print(F("Temp deviation: "));
                 Serial.print(current_deviation, 1);
                 Serial.print(F("°C (soglia: "));
                 Serial.print(emergency_system.critical_temp_threshold * 0.8, 1);
                 Serial.println(F("°C) [5°C bilanciato]"));
            }
            break;
            
        case EMERGENCY_SD_FAILURE:
            // Test operazioni SD
            recovery_success = checkSDAvailability();
            if (recovery_success) {
                loadProgramList(); // Ricarica lista programmi
            }
            break;
            
        default:
            recovery_success = true; // Recovery generico
            break;
    }
    
    if (recovery_success) {
        exitEmergencyMode();
        Serial.println(F("✅ RECOVERY RIUSCITO"));
    } else {
        Serial.println(F("❌ Recovery fallito, continua emergenza"));
        
        // Dopo 3 tentativi falliti, accetta la situazione
        if (emergency_system.recovery_attempts >= 3) {
            Serial.println(F("⚠️  Recovery abbandonato dopo 3 tentativi"));
            // Rimane in emergenza ma smette di tentare recovery automatico
        }
    }
    Serial.println(F(""));
}

void exitEmergencyMode() {
    // Salva statistiche
    emergency_system.total_emergency_time = millis() - emergency_system.start_time;
    
    // Reset stato emergenza
    emergency_system.is_active = false;
    emergency_system.current_type = EMERGENCY_NONE;
    strcpy(emergency_system.description, "Sistema ripristinato");
    
    // Reset modalità sistema
    system_state.emergency_mode = false;
    
    // Reset allarmi
    alarm_system.critical_alarm = false;
    alarm_system.high_priority_alarm = false;
    alarm_system.alarm_pattern = 0;
    
    // Disattiva modalità emergenza retroilluminazione
    backlight_emergency_mode(false);
    
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║     ✅ USCITA MODALITÀ EMERGENZA    ║"));
    Serial.println(F("║    RIPRISTINO OPERAZIONI NORMALI    ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    Serial.print(F("Tempo totale emergenza: "));
    Serial.print(emergency_system.total_emergency_time / 60000);
    Serial.println(F(" minuti"));
    Serial.println(F(""));
    
    // Torna alla schermata precedente se era emergenza
    if (ui_state.current_screen == SCREEN_EMERGENCY) {
        switchToScreen(SCREEN_MAIN_DASHBOARD);
    }
}

const char* getEmergencyTypeName(EmergencyType type) {
    switch (type) {
        case EMERGENCY_NONE: return "Nessuna";
        case EMERGENCY_SENSOR_INTERNAL: return "Sensore Interno";
        case EMERGENCY_TEMP_CRITICAL: return "Temperatura Critica";
        case EMERGENCY_TEMP_EXTERNAL_EXTREME: return "Temperatura Estrema";
        case EMERGENCY_POWER_ISSUES: return "Problemi Alimentazione";
        case EMERGENCY_SD_FAILURE: return "Fallimento SD";
        case EMERGENCY_ACTUATOR_FAILURE: return "Fallimento Attuatori";
        default: return "Sconosciuta";
    }
}

void updateDisplay() {
    // Debug: conta chiamate updateDisplay
    static int update_count = 0;
    update_count++;
    
    // Controlla se è necessario aggiornare il display
    unsigned long current_time = millis();
    
    // Forza ridisegno se schermata cambiata o in modalità emergenza
    if (ui_state.screen_needs_redraw || system_state.emergency_mode) {
        ui_state.force_full_redraw = true;
        ui_state.screen_needs_redraw = false;
        Serial.print(F("updateDisplay #"));
        Serial.print(update_count);
        Serial.println(F(" - ridisegno necessario"));
    }
    
    // Ridisegno completo o aggiornamento incrementale
    if (ui_state.force_full_redraw) {
        Serial.print(F("Disegnando schermata: "));
        Serial.println(ui_state.current_screen);
        drawCurrentScreen();
        ui_state.force_full_redraw = false;
        ui_state.last_screen_update = current_time;
        Serial.println(F("Schermata disegnata"));
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
            
        case SCREEN_DIAGNOSTIC:
            drawDiagnosticScreen();
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
    Serial.println(F("DEBUG: Iniziando drawMainDashboard"));
    
    // Titolo principale
    tft.setTextColor(ui_state.text_color);
    tft.setTextSize(TEXT_SIZE_LARGE);
    tft.setCursor(10, 10);
    tft.println(F("STAGIONINO"));
    Serial.println(F("DEBUG: Titolo disegnato"));
    
    // Sottotitolo con indicatore modalità
    tft.setTextSize(TEXT_SIZE_SMALL);
    tft.setCursor(10, 40);
    if (isDemoModeActive()) {
        tft.setTextColor(COLOR_YELLOW);
        tft.println(F("Sistema Stagionatura Salumi - MODALITA' DEMO"));
        tft.setTextColor(ui_state.text_color);
        Serial.println(F("DEBUG: Sottotitolo DEMO disegnato"));
    } else {
        tft.println(F("Sistema Stagionatura Salumi"));
        Serial.println(F("DEBUG: Sottotitolo NORMALE disegnato"));
    }
    
    // Area dati sensori principali
    Serial.println(F("DEBUG: Chiamando drawSensorDataBox"));
    drawSensorDataBox(10, 70, SCREEN_WIDTH - 20, 120);
    Serial.println(F("DEBUG: drawSensorDataBox completato"));
    
    // Indicatori stato attuatori
    Serial.println(F("DEBUG: Chiamando drawActuatorStatus"));
    drawActuatorStatus(10, 200, SCREEN_WIDTH - 20, 60);
    Serial.println(F("DEBUG: drawActuatorStatus completato"));
    
    // Pulsanti di navigazione
    Serial.println(F("DEBUG: Chiamando drawNavigationButtons"));
    drawNavigationButtons();
    Serial.println(F("DEBUG: drawMainDashboard COMPLETATO"));
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
        // Colore diverso se in modalità demo
        if (isDemoModeActive()) {
            tft.setTextColor(COLOR_YELLOW);  // Giallo per dati simulati
        } else {
            tft.setTextColor(COLOR_GREEN);   // Verde per dati reali
        }
        tft.print(sensors.temp_internal, 1);
        tft.print(F("C "));
        tft.print(sensors.hum_internal, 1);
        tft.print(F("%"));
        if (isDemoModeActive()) {
            tft.setTextColor(COLOR_GRAY);
            tft.print(F(" SIM"));  // Indica dati simulati
        }
    } else {
        tft.setTextColor(COLOR_RED);
        tft.print(F("ERRORE"));
    }
    
    // Dati sensore esterno
    tft.setTextColor(ui_state.text_color);
    tft.setCursor(x + 10, y + 55);
    tft.print(F("Esterno: "));
    
    if (sensors.external_valid) {
        // Colore diverso se in modalità demo
        if (isDemoModeActive()) {
            tft.setTextColor(COLOR_YELLOW);  // Giallo per dati simulati
        } else {
            tft.setTextColor(COLOR_CYAN);    // Ciano per dati reali
        }
        tft.print(sensors.temp_external, 1);
        tft.print(F("C "));
        tft.print(sensors.hum_external, 1);
        tft.print(F("%"));
        if (isDemoModeActive()) {
            tft.setTextColor(COLOR_GRAY);
            tft.print(F(" SIM"));  // Indica dati simulati
        }
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
    
    drawButton(20, 80, SCREEN_WIDTH/2 - 30, 50, 
               F("CALIBRA"), COLOR_YELLOW);
    
    // Area diagnostica sistema
    tft.setCursor(10, 150);
    tft.println(F("Test Sistema:"));
    
    drawButton(20, 180, SCREEN_WIDTH/2 - 30, 50, 
               F("DIAGNOSTICA"), COLOR_CYAN);
    
    // Controlli retroilluminazione (lato destro)
    tft.setCursor(SCREEN_WIDTH/2 + 10, 50);
    tft.setTextSize(TEXT_SIZE_SMALL);
    tft.println(F("Retroilluminazione:"));
    
    // Pulsante toggle ON/OFF
    uint16_t toggle_color = backlight_system.is_enabled ? COLOR_GREEN : COLOR_RED;
    const __FlashStringHelper* toggle_text = backlight_system.is_enabled ? F("ON") : F("OFF");
    drawButton(SCREEN_WIDTH/2 + 20, 80, SCREEN_WIDTH/2 - 40, 30, toggle_text, toggle_color);
    
    // Livello corrente
    tft.setCursor(SCREEN_WIDTH/2 + 10, 120);
    tft.print(F("Livello: "));
    tft.print(backlight_system.current_level);
    tft.print(F("/255 ("));
    tft.print((backlight_system.current_level * 100) / 255);
    tft.println(F("%)"));
    
    // Pulsanti profili
    drawButton(SCREEN_WIDTH/2 + 20, 130, 90, 30, F("GIORNO"), COLOR_ORANGE);
    drawButton(SCREEN_WIDTH/2 + 120, 130, 90, 30, F("NOTTE"), COLOR_BLUE);
    
    // Stato auto-dim
    tft.setCursor(SCREEN_WIDTH/2 + 10, 170);
    tft.print(F("Auto-dim: "));
    tft.setTextColor(backlight_system.auto_dim_enabled ? COLOR_GREEN : COLOR_RED);
    tft.println(backlight_system.auto_dim_enabled ? F("SI") : F("NO"));
    tft.setTextColor(ui_state.text_color);
    
    // Timeout inattività
    tft.setCursor(SCREEN_WIDTH/2 + 10, 190);
    tft.print(F("Timeout: "));
    tft.print(BACKLIGHT_TIMEOUT / 1000);
    tft.println(F("s"));
    
    // Modalità Demo
    uint16_t demo_color = isDemoModeActive() ? COLOR_YELLOW : COLOR_GRAY;
    const __FlashStringHelper* demo_text = isDemoModeActive() ? F("DEMO ON") : F("DEMO OFF");
    drawButton(SCREEN_WIDTH/2 + 20, 200, SCREEN_WIDTH/2 - 40, 25, demo_text, demo_color);
    
    // Info sistema
    tft.setCursor(SCREEN_WIDTH/2 + 10, 240);
    tft.print(F("RAM: "));
    extern int __heap_start, *__brkval;
    int free_memory = ((int)&free_memory) - ((int)&__heap_start);
    tft.print(free_memory);
    tft.println(F(" bytes"));
    
    tft.setCursor(SCREEN_WIDTH/2 + 10, 260);
    tft.print(F("Uptime: "));
    tft.print(millis() / 3600000);
    tft.println(F("h"));
    
    // Indica tipo modalità demo se attiva
    if (isDemoModeActive()) {
        tft.setCursor(SCREEN_WIDTH/2 + 10, 280);
        tft.setTextColor(COLOR_YELLOW);
        if (system_state.demo_mode_forced) {
            tft.println(F("DEMO: FORZATA"));
        } else {
            tft.println(F("DEMO: AUTO"));
        }
        tft.setTextColor(ui_state.text_color);
    }
    
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
    tft.setCursor(30, 20);
    tft.println(F("🚨 EMERGENZA"));
    
    // Tipo emergenza
    tft.setTextSize(TEXT_SIZE_MEDIUM);
    tft.setCursor(10, 60);
    tft.print(F("Tipo: "));
    if (emergency_system.is_active) {
        tft.println(getEmergencyTypeName(emergency_system.current_type));
    } else {
        tft.println(F("Nessuna"));
    }
    
    // Descrizione emergenza
    tft.setTextSize(TEXT_SIZE_SMALL);
    tft.setCursor(10, 90);
    tft.println(F("Descrizione:"));
    tft.setCursor(10, 110);
    if (emergency_system.is_active) {
        // Spezza descrizione lunga su più righe
        char desc_copy[128];
        strncpy(desc_copy, emergency_system.description, 127);
        desc_copy[127] = '\0';
        
        char* line = strtok(desc_copy, " ");
        int line_length = 0;
        int y_pos = 110;
        
        while (line != NULL && y_pos < 180) {
            int word_length = strlen(line);
            if (line_length + word_length > 35) { // Max ~35 caratteri per riga
                y_pos += 15;
                line_length = 0;
                tft.setCursor(10, y_pos);
            }
            tft.print(line);
            tft.print(F(" "));
            line_length += word_length + 1;
            line = strtok(NULL, " ");
        }
    } else {
        tft.print(F("Sistema normale"));
    }
    
    // Informazioni recovery e timing
    if (emergency_system.is_active) {
        tft.setCursor(10, 200);
        tft.print(F("Tentativi recovery: "));
        tft.println(emergency_system.recovery_attempts);
        
        tft.setCursor(10, 220);
        tft.print(F("Tempo: "));
        unsigned long elapsed = millis() - emergency_system.start_time;
        tft.print(elapsed / 60000);
        tft.print(F("m "));
        tft.print((elapsed % 60000) / 1000);
        tft.println(F("s"));
    }
    
    // Status allarmi
    tft.setCursor(10, 240);
    tft.print(F("Allarmi: "));
    if (alarm_system.mute_active) {
        tft.print(F("MUTE ("));
        unsigned long remaining = alarm_system.mute_duration - (millis() - alarm_system.mute_start_time);
        tft.print(remaining / 60000);
        tft.print(F("m)"));
    } else if (alarm_system.buzzer_enabled) {
        tft.print(F("ATTIVI"));
    } else {
        tft.print(F("DISABILITATI"));
    }
    
    // Pulsanti azione
    drawButton(10, SCREEN_HEIGHT - 50, 100, 40, F("DASHBOARD"), COLOR_WHITE);
    drawButton(120, SCREEN_HEIGHT - 50, 80, 40, F("MUTE"), COLOR_YELLOW);
    drawButton(210, SCREEN_HEIGHT - 50, 100, 40, F("RECOVERY"), COLOR_GREEN);
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

void drawDiagnosticScreen() {
    static bool test_running = false;
    static unsigned long test_start_time = 0;
    static int test_phase = 0;
    
    tft.setTextColor(ui_state.text_color);
    tft.setTextSize(TEXT_SIZE_MEDIUM);
    tft.setCursor(10, 10);
    tft.println(F("DIAGNOSTICA SISTEMA"));
    
    // Area risultati test
    tft.setTextSize(TEXT_SIZE_SMALL);
    int y_pos = 50;
    
    // Test memoria RAM
    tft.setCursor(10, y_pos);
    tft.print(F("RAM libera: "));
    extern int __heap_start, *__brkval;
    int free_memory;
    if ((int)__brkval == 0) {
        free_memory = ((int)&free_memory) - ((int)&__heap_start);
    } else {
        free_memory = ((int)&free_memory) - ((int)__brkval);
    }
    
    tft.print(free_memory);
    tft.print(F(" bytes "));
    if (free_memory > 1500) {
        tft.setTextColor(COLOR_GREEN);
        tft.println(F("OK"));
    } else if (free_memory > 1000) {
        tft.setTextColor(COLOR_YELLOW);
        tft.println(F("LIMITE"));
    } else {
        tft.setTextColor(COLOR_RED);
        tft.println(F("CRITICO"));
    }
    tft.setTextColor(ui_state.text_color);
    y_pos += 20;
    
    // Status sensori
    tft.setCursor(10, y_pos);
    tft.print(F("Sensore interno: "));
    if (sensors.internal_valid) {
        tft.setTextColor(COLOR_GREEN);
        tft.print(F("OK ("));
        tft.print(sensors.temp_internal, 1);
        tft.print(F("C "));
        tft.print(sensors.hum_internal, 1);
        tft.println(F("%)"));
    } else {
        tft.setTextColor(COLOR_RED);
        tft.print(F("ERRORE ("));
        tft.print(sensors.internal_error_count);
        tft.println(F(" fallimenti)"));
    }
    tft.setTextColor(ui_state.text_color);
    y_pos += 20;
    
    tft.setCursor(10, y_pos);
    tft.print(F("Sensore esterno: "));
    if (sensors.external_valid) {
        tft.setTextColor(COLOR_GREEN);
        tft.print(F("OK ("));
        tft.print(sensors.temp_external, 1);
        tft.print(F("C)"));
    } else {
        tft.setTextColor(COLOR_YELLOW);
        tft.print(F("NON DISP"));
    }
    tft.setTextColor(ui_state.text_color);
    tft.println();
    y_pos += 20;
    
    // Status sistema
    tft.setCursor(10, y_pos);
    tft.print(F("SD Card: "));
    tft.setTextColor(system_state.sd_available ? COLOR_GREEN : COLOR_RED);
    tft.println(system_state.sd_available ? F("OK") : F("ERRORE"));
    tft.setTextColor(ui_state.text_color);
    y_pos += 20;
    
    tft.setCursor(10, y_pos);
    tft.print(F("RTC: "));
    tft.setTextColor(system_state.rtc_available ? COLOR_GREEN : COLOR_RED);
    tft.println(system_state.rtc_available ? F("OK") : F("ERRORE"));
    tft.setTextColor(ui_state.text_color);
    y_pos += 20;
    
    // Emergenze
    tft.setCursor(10, y_pos);
    tft.print(F("Emergenze totali: "));
    if (emergency_system.emergency_episodes == 0) {
        tft.setTextColor(COLOR_GREEN);
        tft.println(F("0 (PERFETTO)"));
    } else {
        tft.setTextColor(COLOR_YELLOW);
        tft.println(emergency_system.emergency_episodes);
    }
    tft.setTextColor(ui_state.text_color);
    y_pos += 20;
    
    // Uptime
    tft.setCursor(10, y_pos);
    tft.print(F("Uptime: "));
    unsigned long uptime_hours = millis() / 3600000;
    unsigned long uptime_minutes = (millis() % 3600000) / 60000;
    tft.print(uptime_hours);
    tft.print(F("h "));
    tft.print(uptime_minutes);
    tft.println(F("m"));
    y_pos += 20;
    
    // Valutazione sistema
    tft.setCursor(10, y_pos + 10);
    tft.setTextSize(TEXT_SIZE_MEDIUM);
    
    bool system_healthy = (free_memory > 1000) && 
                         sensors.internal_valid && 
                         (emergency_system.emergency_episodes < 3);
    
    if (system_healthy) {
        tft.setTextColor(COLOR_GREEN);
        tft.println(F("SISTEMA: STABILE"));
    } else {
        tft.setTextColor(COLOR_YELLOW);
        tft.println(F("SISTEMA: ATTENZIONE"));
    }
    
    // Pulsanti
    drawButton(10, SCREEN_HEIGHT - 90, 100, 35, F("TEST STRESS"), COLOR_ORANGE);
    drawButton(120, SCREEN_HEIGHT - 90, 100, 35, F("REFRESH"), COLOR_BLUE);
    drawButton(10, SCREEN_HEIGHT - 50, 80, 35, F("BACK"), ui_state.accent_color);
    drawButton(SCREEN_WIDTH - 120, SCREEN_HEIGHT - 50, 110, 35, F("REPORT"), COLOR_CYAN);
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
    
    // Usa la funzione helper che gestisce correttamente i pin condivisi
    // Basata sulla soluzione del forum Arduino MCUFRIEND
    touch_data.is_touched = Touch_getXY();
    
    if (touch_data.is_touched) {
        // Le coordinate sono già mappate dalla funzione helper
        touch_data.x = pixel_x;
        touch_data.y = pixel_y;
        
        // Notifica attività al sistema retroilluminazione
        backlight_activity_detected();
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
    
    // Registra attività per retroilluminazione
    backlight_activity_detected();
    
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
            
        case SCREEN_DIAGNOSTIC:
            handleDiagnosticTouch();
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
    
    // Pulsante Calibrazione (lato sinistro, alto)
    if (touch_data.x > 20 && touch_data.x < SCREEN_WIDTH/2 - 10 &&
        touch_data.y > 80 && touch_data.y < 130) {
        switchToScreen(SCREEN_CALIBRATION);
        return;
    }
    
    // Pulsante Diagnostica (lato sinistro, basso)
    if (touch_data.x > 20 && touch_data.x < SCREEN_WIDTH/2 - 10 &&
        touch_data.y > 180 && touch_data.y < 230) {
        switchToScreen(SCREEN_DIAGNOSTIC);
        return;
    }
    
    // Controlli retroilluminazione (lato destro)
    // Pulsante Toggle Backlight
    if (touch_data.x > SCREEN_WIDTH/2 + 20 && touch_data.x < SCREEN_WIDTH - 20 &&
        touch_data.y > 80 && touch_data.y < 120) {
        toggleBacklight();
        ui_state.screen_needs_redraw = true;
        return;
    }
    
    // Pulsante Profilo Diurno
    if (touch_data.x > SCREEN_WIDTH/2 + 20 && touch_data.x < SCREEN_WIDTH/2 + 120 &&
        touch_data.y > 130 && touch_data.y < 160) {
        setBacklightProfile(0); // Diurno
        ui_state.screen_needs_redraw = true;
        return;
    }
    
    // Pulsante Profilo Notturno
    if (touch_data.x > SCREEN_WIDTH/2 + 130 && touch_data.x < SCREEN_WIDTH - 20 &&
        touch_data.y > 130 && touch_data.y < 160) {
        setBacklightProfile(1); // Notturno
        ui_state.screen_needs_redraw = true;
        return;
    }
    
    // Pulsante Toggle Modalità Demo
    if (touch_data.x > SCREEN_WIDTH/2 + 20 && touch_data.x < SCREEN_WIDTH - 20 &&
        touch_data.y > 200 && touch_data.y < 230) {
        toggleDemoMode();
        ui_state.screen_needs_redraw = true;
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
    // Pulsante Dashboard
    if (touch_data.x >= 10 && touch_data.x <= 110 && 
        touch_data.y > SCREEN_HEIGHT - 60) {
        switchToScreen(SCREEN_MAIN_DASHBOARD);
        return;
    }
    
    // Pulsante Mute
    if (touch_data.x >= 120 && touch_data.x <= 200 && 
        touch_data.y > SCREEN_HEIGHT - 60) {
        muteAlarms(300000); // Mute 5 minuti
        ui_state.screen_needs_redraw = true;
        Serial.println(F("Touch: Allarmi silenziati da interfaccia"));
        return;
    }
    
    // Pulsante Recovery manuale
    if (touch_data.x >= 210 && touch_data.x <= 310 && 
        touch_data.y > SCREEN_HEIGHT - 60) {
        if (emergency_system.is_active) {
            Serial.println(F("Touch: Recovery manuale richiesto"));
            attemptEmergencyRecovery();
            ui_state.screen_needs_redraw = true;
        }
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

void handleDiagnosticTouch() {
    // Pulsante "Indietro"
    if (touch_data.x > 10 && touch_data.x < 90 && 
        touch_data.y > SCREEN_HEIGHT - 85 && touch_data.y < SCREEN_HEIGHT - 15) {
        switchToScreen(SCREEN_SETTINGS);
        return;
    }
    
    // Pulsante "Test Stress"
    if (touch_data.x > 10 && touch_data.x < 110 && 
        touch_data.y > SCREEN_HEIGHT - 125 && touch_data.y < SCREEN_HEIGHT - 90) {
        Serial.println(F("Touch: Avvio stress test..."));
        stressTestSensors();
        ui_state.screen_needs_redraw = true;
        return;
    }
    
    // Pulsante "Refresh"
    if (touch_data.x > 120 && touch_data.x < 220 && 
        touch_data.y > SCREEN_HEIGHT - 125 && touch_data.y < SCREEN_HEIGHT - 90) {
        Serial.println(F("Touch: Refresh diagnostica"));
        ui_state.screen_needs_redraw = true;
        return;
    }
    
    // Pulsante "Report"
    if (touch_data.x > SCREEN_WIDTH - 130 && touch_data.x < SCREEN_WIDTH - 10 && 
        touch_data.y > SCREEN_HEIGHT - 85 && touch_data.y < SCREEN_HEIGHT - 15) {
        Serial.println(F("Touch: Generazione report completo..."));
        runDiagnosticTests();
        ui_state.screen_needs_redraw = true;
        return;
    }
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
    if (!led_system.leds_enabled) {
        return;
    }
    
    unsigned long current_time = millis();
    
    // Aggiorna modalità LED in base allo stato sistema
    updateLEDMode();
    
    // Gestione blink per status attuatori
    if (current_time - led_system.last_blink_time >= 500) {
        led_system.blink_state = !led_system.blink_state;
        led_system.last_blink_time = current_time;
    }
    
    // Aggiorna LED in base alla modalità corrente
    switch (led_system.current_mode) {
        case LED_MODE_NORMAL:
            updateNormalLEDs();
            break;
            
        case LED_MODE_EMERGENCY:
            updateEmergencyLEDs();
            break;
            
        case LED_MODE_PROGRAM_RUNNING:
            updateProgramLEDs();
            break;
            
        case LED_MODE_ERROR:
            updateErrorLEDs();
            break;
            
        case LED_MODE_MAINTENANCE:
            updateMaintenanceLEDs();
            break;
            
        case LED_MODE_STARTUP:
            // Gestito da playStartupAnimation()
            break;
    }
    
    // Effetti speciali globali
    if (led_system.rainbow_effect) {
        applyRainbowEffect();
    }
    
    if (led_system.breathing_effect) {
        applyBreathingEffect();
    }
    
    // Aggiorna display LED
    FastLED.show();
    led_system.last_update = current_time;
}

void updateLEDMode() {
    LEDMode new_mode = LED_MODE_NORMAL;
    
    // Priorità modalità (più alta = più importante)
    if (emergency_system.is_active) {
        new_mode = LED_MODE_EMERGENCY;
    } else if (program_execution.is_running) {
        new_mode = LED_MODE_PROGRAM_RUNNING;
    } else if (!system_state.am2315_available && !system_state.dht11_available && 
               !isDemoModeActive()) {
        // SOLO se non in modalità demo
        new_mode = LED_MODE_ERROR;
    } else {
        new_mode = LED_MODE_NORMAL;
    }
    
    // Cambio modalità con animazione
    if (led_system.current_mode != new_mode) {
        led_system.current_mode = new_mode;
        led_system.animation_active = true;
        led_system.animation_start = millis();
        led_system.animation_step = 0;
        
        Serial.print(F("LED: Cambio modalità -> "));
        Serial.println(getLEDModeName(new_mode));
    }
}

const char* getLEDModeName(LEDMode mode) {
    switch (mode) {
        case LED_MODE_NORMAL: return "Normale";
        case LED_MODE_EMERGENCY: return "Emergenza";
        case LED_MODE_PROGRAM_RUNNING: return "Programma";
        case LED_MODE_STARTUP: return "Avvio";
        case LED_MODE_ERROR: return "Errore";
        case LED_MODE_MAINTENANCE: return "Manutenzione";
        default: return "Sconosciuta";
    }
}

void updateAlarmSystem() {
    unsigned long current_time = millis();
    
    // Se mute attivo, non suonare
    if (alarm_system.mute_active) {
        return;
    }
    
    // Se buzzer disabilitato, solo LED
    if (!alarm_system.buzzer_enabled) {
        return;
    }
    
    // Pattern allarmi basati su priorità
    switch (alarm_system.alarm_pattern) {
        case 0: // Nessun allarme
            // Silenzio
            break;
            
        case 1: // Beep intermittente (high priority)
            if (current_time - alarm_system.last_beep_time >= 2000) {
                playAlarmBeep(1); // 1 beep
                alarm_system.last_beep_time = current_time;
                alarm_system.beep_count++;
            }
            break;
            
        case 2: // Allarme continuo (critical)
            if (current_time - alarm_system.last_beep_time >= 500) {
                playAlarmBeep(3); // 3 beep rapidi
                alarm_system.last_beep_time = current_time;
                alarm_system.beep_count++;
            }
            break;
            
        case 3: // Allarme di emergenza estrema
            if (current_time - alarm_system.last_beep_time >= 200) {
                playAlarmBeep(5); // 5 beep molto rapidi
                alarm_system.last_beep_time = current_time;
                alarm_system.beep_count++;
            }
            break;
    }
    
    // Auto-mute dopo 100 beep per evitare disturbo eccessivo
    if (alarm_system.beep_count >= 100) {
        muteAlarms(300000); // Mute 5 minuti
        Serial.println(F("Auto-mute allarmi dopo 100 beep"));
    }
}

void playAlarmBeep(int beep_count) {
    for (int i = 0; i < beep_count; i++) {
        // Beep acuto per emergenza
        tone(BUZZER_PIN, 2000, 100);
        delay(120);
        
        if (i < beep_count - 1) {
            delay(50); // Pausa tra beep
        }
    }
}

void muteAlarms(unsigned long duration_ms) {
    alarm_system.mute_active = true;
    alarm_system.mute_start_time = millis();
    alarm_system.mute_duration = duration_ms;
    alarm_system.beep_count = 0; // Reset contatore
    
    Serial.print(F("Allarmi silenziati per "));
    Serial.print(duration_ms / 60000);
    Serial.println(F(" minuti"));
}

void toggleBuzzer() {
    alarm_system.buzzer_enabled = !alarm_system.buzzer_enabled;
    
    Serial.print(F("Buzzer "));
    Serial.println(alarm_system.buzzer_enabled ? F("ABILITATO") : F("DISABILITATO"));
    
    if (alarm_system.buzzer_enabled) {
        // Beep di conferma abilitazione
        tone(BUZZER_PIN, 1500, 200);
        delay(250);
        tone(BUZZER_PIN, 1800, 200);
    }
}

void handleBuzzer() {
    // Aggiorna sistema allarmi
    updateAlarmSystem();
    
    // Controlli aggiuntivi per condizioni borderline (warning)
    if (!emergency_system.is_active && alarm_system.alarm_pattern == 0) {
        // Allarmi preventivi per condizioni al limite
        bool warning_condition = false;
        
        // Temperatura vicina al limite
        if (sensors.internal_valid) {
            float target_avg = (control_system.target_temp_min + control_system.target_temp_max) / 2.0;
            float temp_diff = target_avg - sensors.temp_internal;
            
            if (temp_diff > emergency_system.critical_temp_threshold * 0.7) {
                warning_condition = true;
            }
        }
        
        // Sensore con errori intermittenti
        if (sensors.internal_error_count > 5 && sensors.internal_error_count < 10) {
            warning_condition = true;
        }
        
        // Attiva warning beep se necessario
        if (warning_condition) {
            static unsigned long last_warning = 0;
            if (millis() - last_warning >= 30000) { // Ogni 30 secondi
                tone(BUZZER_PIN, 1000, 150); // Beep basso warning
                last_warning = millis();
            }
        }
    }
}

void resetWatchdog() {
    wdt_reset();
}

void controlTemperature() {
    unsigned long current_time = millis();
    
    // Controllo adattivo: più frequente se temperatura critica
    unsigned long temp_interval = CONTROL_FRIDGE_INTERVAL;
    
    if (sensors.internal_valid) {
        float target_avg = (control_system.target_temp_min + control_system.target_temp_max) / 2.0;
        float temp_deviation = abs(sensors.temp_internal - target_avg);
        
        // Se temperatura molto fuori range, controllo più frequente
        if (temp_deviation > 3.0) {
            temp_interval = 30000;  // 30 secondi per situazioni critiche
        } else if (temp_deviation > 1.5) {
            temp_interval = 60000;  // 1 minuto per situazioni borderline
        }
    }
    
    // Controllo temporizzato adattivo
    if (current_time - control_system.last_temp_control < temp_interval) {
        return;
    }
    
    control_system.last_temp_control = current_time;
    
    float current_temp = sensors.temp_internal;
    float target_min = control_system.target_temp_min;
    float target_max = control_system.target_temp_max;
    float hysteresis = control_system.temp_hysteresis;
    
    // === CONTROLLO FRIGORIFERO ===
    // Temperatura troppo alta - accendi frigorifero
    if (current_temp > target_max + hysteresis) {
        if (canActivateActuator(&control_system.frigorifero, RELAY_FRIGORIFERO, 
                               MIN_FRIDGE_ON_TIME, MIN_FRIDGE_OFF_TIME)) {
            activateActuator(&control_system.frigorifero, RELAY_FRIGORIFERO, true);
            Serial.print(F("TEMP: Frigorifero ON ("));
            Serial.print(current_temp, 1);
            Serial.print(F("°C > "));
            Serial.print(target_max + hysteresis, 1);
            Serial.println(F("°C)"));
        }
    }
    // Temperatura raggiunta - spegni frigorifero
    else if (current_temp < target_max - hysteresis && control_system.frigorifero.is_active) {
        if (canDeactivateActuator(&control_system.frigorifero, MIN_FRIDGE_ON_TIME)) {
            activateActuator(&control_system.frigorifero, RELAY_FRIGORIFERO, false);
            Serial.print(F("TEMP: Frigorifero OFF ("));
            Serial.print(current_temp, 1);
            Serial.print(F("°C < "));
            Serial.print(target_max - hysteresis, 1);
            Serial.println(F("°C)"));
        }
    }
    
    // === CONTROLLO RISCALDATORE ===
    // Temperatura troppo bassa - accendi riscaldatore
    if (current_temp < target_min - hysteresis) {
        if (canActivateActuator(&control_system.riscaldatore, RELAY_RISCALDATORE, 
                               MIN_HEATER_ON_TIME, MIN_HEATER_OFF_TIME)) {
            activateActuator(&control_system.riscaldatore, RELAY_RISCALDATORE, true);
            Serial.print(F("TEMP: Riscaldatore ON ("));
            Serial.print(current_temp, 1);
            Serial.print(F("°C < "));
            Serial.print(target_min - hysteresis, 1);
            Serial.println(F("°C)"));
        }
    }
    // Temperatura raggiunta - spegni riscaldatore  
    else if (current_temp > target_min + hysteresis && control_system.riscaldatore.is_active) {
        if (canDeactivateActuator(&control_system.riscaldatore, MIN_HEATER_ON_TIME)) {
            activateActuator(&control_system.riscaldatore, RELAY_RISCALDATORE, false);
            Serial.print(F("TEMP: Riscaldatore OFF ("));
            Serial.print(current_temp, 1);
            Serial.print(F("°C > "));
            Serial.print(target_min + hysteresis, 1);
            Serial.println(F("°C)"));
        }
    }
}

void controlHumidity() {
    unsigned long current_time = millis();
    
    // Controllo temporizzato
    if (current_time - control_system.last_hum_control < CONTROL_HUM_INTERVAL) {
        return;
    }
    
    control_system.last_hum_control = current_time;
    
    float current_hum = sensors.hum_internal;
    float target_min = control_system.target_hum_min;
    float target_max = control_system.target_hum_max;
    float hysteresis = control_system.hum_hysteresis;
    
    // === CONTROLLO DEUMIDIFICATORE ===
    if (control_system.dehumidifier_available) {
        // Umidità troppo alta - accendi deumidificatore
        if (current_hum > target_max + hysteresis) {
            if (canActivateActuator(&control_system.deumidificatore, RELAY_DEUMIDIFICATORE, 
                                   MIN_DEHUM_ON_TIME, MIN_DEHUM_OFF_TIME)) {
                activateActuator(&control_system.deumidificatore, RELAY_DEUMIDIFICATORE, true);
                Serial.print(F("HUM: Deumidificatore ON ("));
                Serial.print(current_hum, 1);
                Serial.print(F("% > "));
                Serial.print(target_max + hysteresis, 1);
                Serial.println(F("%)"));
            }
        }
        // Umidità raggiunta - spegni deumidificatore
        else if (current_hum < target_max - hysteresis && control_system.deumidificatore.is_active) {
            if (canDeactivateActuator(&control_system.deumidificatore, MIN_DEHUM_ON_TIME)) {
                activateActuator(&control_system.deumidificatore, RELAY_DEUMIDIFICATORE, false);
                Serial.print(F("HUM: Deumidificatore OFF ("));
                Serial.print(current_hum, 1);
                Serial.print(F("% < "));
                Serial.print(target_max - hysteresis, 1);
                Serial.println(F("%)"));
            }
        }
    } else {
        // Se non c'è deumidificatore, usa ventola estrazione per umidità alta
        if (current_hum > target_max + hysteresis) {
            if (canActivateActuator(&control_system.ventola_out, RELAY_VENTOLA_OUT, 
                                   MIN_FAN_ON_TIME, MIN_FAN_OFF_TIME)) {
                activateActuator(&control_system.ventola_out, RELAY_VENTOLA_OUT, true);
                Serial.println(F("HUM: Ventola estrazione ON (no deumidificatore)"));
            }
        }
    }
    
    // === CONTROLLO UMIDIFICATORE ===
    if (control_system.humidifier_available) {
        // Umidità troppo bassa - accendi umidificatore
        if (current_hum < target_min - hysteresis) {
            if (canActivateActuator(&control_system.umidificatore, RELAY_UMIDIFICATORE, 
                                   MIN_HUM_ON_TIME, MIN_HUM_OFF_TIME)) {
                activateActuator(&control_system.umidificatore, RELAY_UMIDIFICATORE, true);
                Serial.print(F("HUM: Umidificatore ON ("));
                Serial.print(current_hum, 1);
                Serial.print(F("% < "));
                Serial.print(target_min - hysteresis, 1);
                Serial.println(F("%)"));
            }
        }
        // Umidità raggiunta - spegni umidificatore
        else if (current_hum > target_min + hysteresis && control_system.umidificatore.is_active) {
            if (canDeactivateActuator(&control_system.umidificatore, MIN_HUM_ON_TIME)) {
                activateActuator(&control_system.umidificatore, RELAY_UMIDIFICATORE, false);
                Serial.print(F("HUM: Umidificatore OFF ("));
                Serial.print(current_hum, 1);
                Serial.print(F("% > "));
                Serial.print(target_min + hysteresis, 1);
                Serial.println(F("%)"));
            }
        }
    } else {
        // Se non c'è umidificatore, usa ventola immissione + frigorifero/riscaldatore
        if (current_hum < target_min - hysteresis) {
            // Strategia adattiva: se possibile crea umidità con temperatura
            if (canActivateActuator(&control_system.ventola_in, RELAY_VENTOLA_IN, 
                                   MIN_FAN_ON_TIME, MIN_FAN_OFF_TIME)) {
                activateActuator(&control_system.ventola_in, RELAY_VENTOLA_IN, true);
                Serial.println(F("HUM: Ventola immissione ON (no umidificatore)"));
            }
        }
    }
}

void controlVentilation() {
    unsigned long current_time = millis();
    
    // Controllo temporizzato ventilazione
    if (current_time - control_system.last_vent_control < CONTROL_FAN_INTERVAL) {
        return;
    }
    
    control_system.last_vent_control = current_time;
    
    // Solo se entrambe le ventole sono disponibili
    if (!control_system.ventilation_available) {
        return;
    }
    
    // Controllo ventilazione intelligente basata su condizioni esterne
    if (sensors.internal_valid && sensors.external_valid) {
        float temp_diff = sensors.temp_external - sensors.temp_internal;
        float hum_diff = sensors.hum_external - sensors.hum_internal;
        
        // Ricircolo aria per migliorare condizioni
        bool should_ventilate = false;
        
        // Se aria esterna è più favorevole per temperatura
        if (abs(temp_diff) > 2.0) {
            float target_temp = (control_system.target_temp_min + control_system.target_temp_max) / 2.0;
            
            // Se interno troppo caldo e esterno più fresco
            if (sensors.temp_internal > target_temp && temp_diff < -1.0) {
                should_ventilate = true;
                Serial.println(F("VENT: Aria esterna più fresca"));
            }
            // Se interno troppo freddo e esterno più caldo
            else if (sensors.temp_internal < target_temp && temp_diff > 1.0) {
                should_ventilate = true;
                Serial.println(F("VENT: Aria esterna più calda"));
            }
        }
        
        // Attiva/disattiva ventilazione coordinata
        if (should_ventilate) {
            if (canActivateActuator(&control_system.ventola_in, RELAY_VENTOLA_IN, 
                                   MIN_FAN_ON_TIME, MIN_FAN_OFF_TIME) &&
                canActivateActuator(&control_system.ventola_out, RELAY_VENTOLA_OUT, 
                                   MIN_FAN_ON_TIME, MIN_FAN_OFF_TIME)) {
                
                activateActuator(&control_system.ventola_in, RELAY_VENTOLA_IN, true);
                activateActuator(&control_system.ventola_out, RELAY_VENTOLA_OUT, true);
                Serial.println(F("VENT: Ricircolo aria attivato"));
            }
        } else {
            // Spegni ventilazione se non necessaria
            if (control_system.ventola_in.is_active && control_system.ventola_out.is_active) {
                if (canDeactivateActuator(&control_system.ventola_in, MIN_FAN_ON_TIME) &&
                    canDeactivateActuator(&control_system.ventola_out, MIN_FAN_ON_TIME)) {
                    
                    activateActuator(&control_system.ventola_in, RELAY_VENTOLA_IN, false);
                    activateActuator(&control_system.ventola_out, RELAY_VENTOLA_OUT, false);
                    Serial.println(F("VENT: Ricircolo aria disattivato"));
                }
            }
        }
    }
}

// ===============================================================================
// FUNZIONI SUPPORTO CONTROLLO ATTUATORI
// ===============================================================================

bool canActivateActuator(ActuatorState* actuator, int relay_pin, 
                        unsigned long min_on_time, unsigned long min_off_time) {
    unsigned long current_time = millis();
    
    // Se già attivo, non serve riattivare
    if (actuator->is_active) {
        return false;
    }
    
    // Controlla tempo minimo OFF
    unsigned long time_since_off = current_time - actuator->last_change_time;
    if (time_since_off < min_off_time) {
        actuator->protection_active = true;
        return false;
    }
    
    actuator->protection_active = false;
    return true;
}

bool canDeactivateActuator(ActuatorState* actuator, unsigned long min_on_time) {
    unsigned long current_time = millis();
    
    // Se già spento, non serve riattivare
    if (!actuator->is_active) {
        return false;
    }
    
    // Controlla tempo minimo ON
    unsigned long time_since_on = current_time - actuator->last_change_time;
    if (time_since_on < min_on_time) {
        actuator->protection_active = true;
        return false;
    }
    
    actuator->protection_active = false;
    return true;
}

void activateActuator(ActuatorState* actuator, int relay_pin, bool activate) {
    unsigned long current_time = millis();
    
    // Aggiorna statistiche tempo
    if (actuator->is_active && !activate) {
        // Spegnimento: aggiungi tempo ON
        actuator->total_on_time += current_time - actuator->last_change_time;
    } else if (!actuator->is_active && activate) {
        // Accensione: aggiungi tempo OFF
        actuator->total_off_time += current_time - actuator->last_change_time;
    }
    
    // Cambia stato attuatore
    actuator->is_active = activate;
    actuator->last_change_time = current_time;
    actuator->protection_active = false;
    
    // Controlla relè (logica invertita: LOW = ON, HIGH = OFF)
    digitalWrite(relay_pin, activate ? LOW : HIGH);
    
    // Debug
    Serial.print(F("RELAY PIN "));
    Serial.print(relay_pin);
    Serial.print(F(": "));
    Serial.println(activate ? F("ON") : F("OFF"));
}

void updateActuatorStatistics() {
    // Aggiorna statistiche per tutti gli attuatori attivi
    unsigned long current_time = millis();
    
    // Solo log periodico per evitare spam
    static unsigned long last_stats_log = 0;
    if (current_time - last_stats_log > 300000) { // Ogni 5 minuti
        last_stats_log = current_time;
        
        Serial.println(F("=== STATISTICHE ATTUATORI (ultimi 5min) ==="));
        logActuatorStats(F("Frigorifero"), &control_system.frigorifero);
        logActuatorStats(F("Riscaldatore"), &control_system.riscaldatore);
        logActuatorStats(F("Deumidificatore"), &control_system.deumidificatore);
        logActuatorStats(F("Umidificatore"), &control_system.umidificatore);
        logActuatorStats(F("Ventola IN"), &control_system.ventola_in);
        logActuatorStats(F("Ventola OUT"), &control_system.ventola_out);
    }
}

void logActuatorStats(const __FlashStringHelper* name, ActuatorState* actuator) {
    unsigned long current_time = millis();
    unsigned long total_time = actuator->total_on_time + actuator->total_off_time;
    
    if (actuator->is_active) {
        total_time += current_time - actuator->last_change_time;
    }
    
    if (total_time > 0) {
        float duty_cycle = (float)actuator->total_on_time / total_time * 100.0;
        Serial.print(name);
        Serial.print(F(": "));
        Serial.print(duty_cycle, 1);
        Serial.print(F("% duty, "));
        Serial.print(actuator->is_active ? F("ON") : F("OFF"));
        if (actuator->protection_active) {
            Serial.print(F(" [PROTECTED]"));
        }
        Serial.println();
    }
}

// ===============================================================================
// GESTIONE SD CARD E PROGRAMMI
// ===============================================================================

bool checkSDAvailability() {
    // Verifica se SD è ancora disponibile
    if (!sd_manager.is_available) {
        return false;
    }
    
    // Test rapido di lettura
    File testFile = SD.open("/");
    if (!testFile) {
        sd_manager.is_available = false;
        strcpy(sd_manager.last_error, "SD card disconnessa");
        Serial.println(F("ERRORE SD: Card disconnessa"));
        return false;
    }
    testFile.close();
    
    return true;
}

bool retrySDOperation() {
    // Gestione retry automatico NON BLOCCANTE per operazioni SD
    unsigned long current_time = millis();
    
    if (!sd_manager.retry_needed) {
        return true;
    }
    
    // Attendi intervallo retry (ridotto per maggiore reattività)
    if (current_time - sd_manager.last_retry_time < 2000) {
        return false;
    }
    
    sd_manager.retry_count++;
    sd_manager.last_retry_time = current_time;
    
    Serial.print(F("SD Retry "));
    Serial.print(sd_manager.retry_count);
    Serial.print(F("/"));
    Serial.println(SD_RETRY_COUNT);
    
    // Reinizializza SD con timeout breve
    wdt_reset();
    unsigned long retry_start = millis();
    bool retry_success = false;
    
    // Tentativo rapido (max 1 secondo)
    if (SD.begin(SD_CS)) {
        unsigned long retry_time = millis() - retry_start;
        if (retry_time < 1000) {
            retry_success = true;
        }
    }
    
    if (retry_success) {
        sd_manager.is_available = true;
        sd_manager.retry_needed = false;
        sd_manager.retry_count = 0;
        strcpy(sd_manager.last_error, "Recupero OK");
        Serial.println(F("SD: Recupero riuscito"));
        return true;
    }
    
    // Se supera max retry, passa in modalità fallback
    if (sd_manager.retry_count >= SD_RETRY_COUNT) {
        sd_manager.retry_needed = false;
        sd_manager.is_available = false;
        strcpy(sd_manager.last_error, "Fallimento permanente");
        Serial.println(F("SD: Fallimento permanente - Modalità fallback"));
        
        // Se programma in esecuzione, continua in modalità manuale
        if (program_execution.is_running) {
            Serial.println(F("ATTENZIONE: Programma continua in modalità manuale"));
            // Non interrompe l'esecuzione, ma disabilita salvataggio stato
        }
    }
    
    return false;
}

void loadProgramList() {
    if (!checkSDAvailability()) {
        return;
    }
    
    sd_manager.programs_count = 0;
    
    File root = SD.open("/programs");
    if (!root) {
        strcpy(sd_manager.last_error, "Directory /programs non trovata");
        return;
    }
    
    while (true) {
        File entry = root.openNextFile();
        if (!entry) {
            break; // Fine file
        }
        
        // Solo file .txt
        String filename = entry.name();
        if (filename.endsWith(".txt") && sd_manager.programs_count < 20) {
            // Rimuovi estensione .txt
            filename.remove(filename.length() - 4);
            filename.toCharArray(sd_manager.program_list[sd_manager.programs_count], 64);
            sd_manager.programs_count++;
            
            Serial.print(F("Programma trovato: "));
            Serial.println(filename);
        }
        
        entry.close();
    }
    
    root.close();
    sd_manager.last_operation_time = millis();
}

bool loadProgram(const char* program_name) {
    if (!checkSDAvailability()) {
        if (retrySDOperation()) {
            return loadProgram(program_name); // Retry ricorsivo
        }
        return false;
    }
    
    // Costruisci path file
    char file_path[128];
    snprintf(file_path, sizeof(file_path), "/programs/%s.txt", program_name);
    
    File program_file = SD.open(file_path);
    if (!program_file) {
        snprintf(sd_manager.last_error, sizeof(sd_manager.last_error), 
                "File %s non trovato", program_name);
        Serial.print(F("ERRORE: "));
        Serial.println(sd_manager.last_error);
        return false;
    }
    
    // Reset programma corrente
    current_program.is_loaded = false;
    current_program.is_valid = false;
    current_program.total_phases = 0;
    
    // Leggi header programma
    String line = program_file.readStringUntil('\n');
    line.trim();
    
    if (!line.startsWith("STAGIONINO_PROGRAM")) {
        strcpy(sd_manager.last_error, "Formato file non valido");
        program_file.close();
        return false;
    }
    
    // Leggi nome programma
    line = program_file.readStringUntil('\n');
    line.trim();
    if (line.startsWith("NAME:")) {
        line.remove(0, 5);
        line.trim();
        line.toCharArray(current_program.name, sizeof(current_program.name));
    }
    
    // Leggi descrizione
    line = program_file.readStringUntil('\n');
    line.trim();
    if (line.startsWith("DESC:")) {
        line.remove(0, 5);
        line.trim();
        line.toCharArray(current_program.description, sizeof(current_program.description));
    }
    
    // Leggi fasi
    int phase_count = 0;
    while (program_file.available() && phase_count < 30) {
        line = program_file.readStringUntil('\n');
        line.trim();
        
        if (line.length() == 0 || line.startsWith("#")) {
            continue; // Riga vuota o commento
        }
        
        if (line.startsWith("PHASE:")) {
            if (parsePhase(line, &current_program.phases[phase_count])) {
                phase_count++;
            } else {
                snprintf(sd_manager.last_error, sizeof(sd_manager.last_error),
                        "Errore parsing fase %d", phase_count + 1);
                program_file.close();
                return false;
            }
        }
    }
    
    program_file.close();
    
    if (phase_count == 0) {
        strcpy(sd_manager.last_error, "Nessuna fase valida trovata");
        return false;
    }
    
    current_program.total_phases = phase_count;
    current_program.is_loaded = true;
    current_program.is_valid = true;
    
    Serial.print(F("Programma caricato: "));
    Serial.print(current_program.name);
    Serial.print(F(" ("));
    Serial.print(phase_count);
    Serial.println(F(" fasi)"));
    
    return true;
}

bool parsePhase(String& line, ProgramPhase* phase) {
    // Formato: PHASE:nome,temp_min,temp_max,hum_min,hum_max,duration_hours,is_final
    // Esempio: PHASE:Stufatura,16.0,24.0,-1,-1,24,0
    
    line.remove(0, 6); // Rimuovi "PHASE:"
    
    // Parse parametri separati da virgola
    int param_count = 0;
    int last_comma = -1;
    
    for (int i = 0; i <= line.length(); i++) {
        if (i == line.length() || line.charAt(i) == ',') {
            String param = line.substring(last_comma + 1, i);
            param.trim();
            
            switch (param_count) {
                case 0: // Nome fase
                    param.toCharArray(phase->name, sizeof(phase->name));
                    break;
                case 1: // Temp min
                    phase->temp_min = param.toFloat();
                    break;
                case 2: // Temp max
                    phase->temp_max = param.toFloat();
                    break;
                case 3: // Hum min (-1 = non controllata)
                    phase->hum_min = param.toFloat();
                    break;
                case 4: // Hum max (-1 = non controllata)
                    phase->hum_max = param.toFloat();
                    break;
                case 5: // Durata ore (0 = infinita)
                    phase->duration_hours = param.toInt();
                    break;
                case 6: // Fase finale
                    phase->is_final_phase = (param.toInt() == 1);
                    break;
            }
            
            param_count++;
            last_comma = i;
        }
    }
    
    // Validazione parametri
    if (param_count < 7) {
        return false;
    }
    
    if (phase->temp_min < -50 || phase->temp_max > 100 || 
        phase->temp_min >= phase->temp_max) {
        return false;
    }
    
    Serial.print(F("  Fase: "));
    Serial.print(phase->name);
    Serial.print(F(" T:"));
    Serial.print(phase->temp_min, 1);
    Serial.print(F("-"));
    Serial.print(phase->temp_max, 1);
    Serial.print(F("°C"));
    
    if (phase->hum_min >= 0 && phase->hum_max >= 0) {
        Serial.print(F(" H:"));
        Serial.print(phase->hum_min, 1);
        Serial.print(F("-"));
        Serial.print(phase->hum_max, 1);
        Serial.print(F("%"));
    } else {
        Serial.print(F(" H:NC"));
    }
    
    if (phase->duration_hours > 0) {
        Serial.print(F(" Durata:"));
        Serial.print(phase->duration_hours);
        Serial.print(F("h"));
    } else {
        Serial.print(F(" Durata:INF"));
    }
    
    if (phase->is_final_phase) {
        Serial.print(F(" [FINALE]"));
    }
    
    Serial.println();
    
    return true;
}

// ===============================================================================
// ESECUZIONE PROGRAMMI AUTOMATICI
// ===============================================================================

bool startProgram(const char* program_name) {
    // Carica il programma
    if (!loadProgram(program_name)) {
        return false;
    }
    
    // Inizializza esecuzione
    program_execution.is_running = true;
    program_execution.current_phase = 0;
    program_execution.phase_start_time = millis();
    program_execution.program_start_time = millis();
    program_execution.total_elapsed_hours = 0;
    program_execution.current_program = &current_program;
    program_execution.auto_advance = true;
    program_execution.phase_completed = false;
    
    // Applica parametri prima fase
    applyPhaseParameters(0);
    
    // Passa in modalità automatica
    control_system.manual_mode = false;
    control_system.auto_mode = true;
    control_system.temp_only_mode = false;
    
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║       PROGRAMMA AVVIATO              ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    Serial.print(F("Programma: "));
    Serial.println(current_program.name);
    Serial.print(F("Fase 1: "));
    Serial.println(current_program.phases[0].name);
    Serial.println(F(""));
    
    return true;
}

void stopProgram() {
    if (!program_execution.is_running) {
        return;
    }
    
    program_execution.is_running = false;
    program_execution.current_program = nullptr;
    
    // Torna in modalità manuale
    control_system.manual_mode = true;
    control_system.auto_mode = false;
    
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║       PROGRAMMA FERMATO              ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    Serial.println(F(""));
}

void updateProgramExecution() {
    if (!program_execution.is_running || !program_execution.current_program) {
        return;
    }
    
    unsigned long current_time = millis();
    unsigned long phase_elapsed = current_time - program_execution.phase_start_time;
    unsigned long phase_elapsed_hours = phase_elapsed / 3600000; // Converti in ore
    
    ProgramPhase* current_phase = &program_execution.current_program->phases[program_execution.current_phase];
    
    // Aggiorna tempo totale trascorso
    program_execution.total_elapsed_hours = (current_time - program_execution.program_start_time) / 3600000;
    
    // Controlla se fase completata (solo se ha durata limitata)
    if (current_phase->duration_hours > 0 && phase_elapsed_hours >= current_phase->duration_hours) {
        program_execution.phase_completed = true;
        
        if (program_execution.auto_advance) {
            advanceToNextPhase();
        } else {
            // Attendi comando manuale per avanzare
            Serial.println(F("Fase completata - Attendere avanzamento manuale"));
        }
    }
    
    // Log periodico stato programma (ogni ora)
    static unsigned long last_program_log = 0;
    if (current_time - last_program_log > 3600000) {
        last_program_log = current_time;
        logProgramStatus();
    }
}

void advanceToNextPhase() {
    if (!program_execution.is_running) {
        return;
    }
    
    int next_phase = program_execution.current_phase + 1;
    
    // Se ultima fase o fase finale
    if (next_phase >= program_execution.current_program->total_phases || 
        program_execution.current_program->phases[program_execution.current_phase].is_final_phase) {
        
        Serial.println(F(""));
        Serial.println(F("╔══════════════════════════════════════╗"));
        Serial.println(F("║    PROGRAMMA COMPLETATO              ║"));
        Serial.println(F("║    FASE DI STAGIONATURA              ║"));
        Serial.println(F("╚══════════════════════════════════════╝"));
        Serial.println(F("Continuazione fase finale infinita..."));
        Serial.println(F(""));
        
        // Resta in fase finale infinita
        program_execution.phase_completed = false;
        return;
    }
    
    // Avanza alla fase successiva
    program_execution.current_phase = next_phase;
    program_execution.phase_start_time = millis();
    program_execution.phase_completed = false;
    
    // Applica parametri nuova fase
    applyPhaseParameters(next_phase);
    
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║       AVANZAMENTO FASE               ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    Serial.print(F("Nuova fase "));
    Serial.print(next_phase + 1);
    Serial.print(F("/"));
    Serial.print(program_execution.current_program->total_phases);
    Serial.print(F(": "));
    Serial.println(program_execution.current_program->phases[next_phase].name);
    Serial.println(F(""));
}

void applyPhaseParameters(int phase_index) {
    if (phase_index >= program_execution.current_program->total_phases) {
        return;
    }
    
    ProgramPhase* phase = &program_execution.current_program->phases[phase_index];
    
    // Applica parametri temperatura
    control_system.target_temp_min = phase->temp_min;
    control_system.target_temp_max = phase->temp_max;
    
    // Applica parametri umidità (se controllata)
    if (phase->hum_min >= 0 && phase->hum_max >= 0) {
        control_system.target_hum_min = phase->hum_min;
        control_system.target_hum_max = phase->hum_max;
        control_system.temp_only_mode = false;
    } else {
        // Solo controllo temperatura
        control_system.temp_only_mode = true;
    }
    
    Serial.print(F("Parametri applicati - T: "));
    Serial.print(phase->temp_min, 1);
    Serial.print(F("-"));
    Serial.print(phase->temp_max, 1);
    Serial.print(F("°C"));
    
    if (!control_system.temp_only_mode) {
        Serial.print(F(", H: "));
        Serial.print(phase->hum_min, 1);
        Serial.print(F("-"));
        Serial.print(phase->hum_max, 1);
        Serial.print(F("%"));
    } else {
        Serial.print(F(", H: Non controllata"));
    }
    Serial.println();
}

void logProgramStatus() {
    Serial.println(F("=== STATUS PROGRAMMA ==="));
    Serial.print(F("Programma: "));
    Serial.println(program_execution.current_program->name);
    Serial.print(F("Fase: "));
    Serial.print(program_execution.current_phase + 1);
    Serial.print(F("/"));
    Serial.print(program_execution.current_program->total_phases);
    Serial.print(F(" - "));
    Serial.println(program_execution.current_program->phases[program_execution.current_phase].name);
    
    unsigned long phase_elapsed = millis() - program_execution.phase_start_time;
    Serial.print(F("Tempo fase: "));
    Serial.print(phase_elapsed / 3600000);
    Serial.print(F("h "));
    Serial.print((phase_elapsed % 3600000) / 60000);
    Serial.println(F("m"));
    
    Serial.print(F("Tempo totale: "));
    Serial.print(program_execution.total_elapsed_hours);
    Serial.println(F("h"));
    
    ProgramPhase* phase = &program_execution.current_program->phases[program_execution.current_phase];
    if (phase->duration_hours > 0) {
        unsigned long remaining = phase->duration_hours - (phase_elapsed / 3600000);
        Serial.print(F("Tempo rimanente fase: "));
        Serial.print(remaining);
        Serial.println(F("h"));
    } else {
        Serial.println(F("Fase infinita"));
    }
}

// ===============================================================================
// FUNZIONI LED SPECIFICHE PER MODALITÀ
// ===============================================================================

void playStartupAnimation() {
    // Animazione avvio semplificata per evitare blocchi durante init
    Serial.println(F("     Animazione LED avvio..."));
    
    // Animazione veloce e sicura
    for (int brightness = 0; brightness <= 255; brightness += 51) {
        fill_solid(leds_24, NUM_LEDS_24, CRGB(0, brightness, 0)); // Verde crescente
        fill_solid(leds_12, NUM_LEDS_12, CRGB(0, 0, brightness)); // Blu crescente
        FastLED.show();
        delay(100);
        wdt_reset(); // Reset watchdog durante animazione
    }
    
    // Flash finale
    fill_solid(leds_24, NUM_LEDS_24, CRGB::Green);
    fill_solid(leds_12, NUM_LEDS_12, CRGB::Blue);
    FastLED.show();
    delay(500);
    
    // Spegni
    fill_solid(leds_24, NUM_LEDS_24, CRGB::Black);
    fill_solid(leds_12, NUM_LEDS_12, CRGB::Black);
    FastLED.show();
    
    Serial.println(F("     Animazione LED completata"));
}

void updateNormalLEDs() {
    // Strip 24: Status attuatori (6 gruppi di 4 LED)
    int leds_per_actuator = NUM_LEDS_24 / 6;
    
    for (int actuator = 0; actuator < 6; actuator++) {
        CRGB color = CRGB::Black;
        bool is_active = false;
        
        // Determina stato attuatore
        switch (actuator) {
            case 0: is_active = control_system.frigorifero.is_active; break;
            case 1: is_active = control_system.riscaldatore.is_active; break;
            case 2: is_active = control_system.deumidificatore.is_active; break;
            case 3: is_active = control_system.umidificatore.is_active; break;
            case 4: is_active = control_system.ventola_in.is_active; break;
            case 5: is_active = control_system.ventola_out.is_active; break;
        }
        
        // Applica colore e blink se necessario
        if (is_active) {
            color = led_system.status_colors[actuator];
            
            // Blink se in protezione
            ActuatorState* state = getActuatorState(actuator);
            if (state && state->protection_active && led_system.blink_state) {
                color = CRGB::Black;
            }
        }
        
        // Applica colore al gruppo LED
        for (int i = 0; i < leds_per_actuator; i++) {
            int led_index = actuator * leds_per_actuator + i;
            if (led_index < NUM_LEDS_24) {
                leds_24[led_index] = color;
            }
        }
    }
    
    // Strip 12: Indicatori sistema
    updateSystemStatusLEDs();
}

void updateEmergencyLEDs() {
    unsigned long elapsed = millis() - led_system.animation_start;
    
    // Pattern lampeggiante rosso/giallo alternato
    CRGB emergency_color = (elapsed % 1000 < 500) ? CRGB::Red : CRGB::Yellow;
    
    // Strip 24: tutto del colore emergenza
    fill_solid(leds_24, NUM_LEDS_24, emergency_color);
    
    // Strip 12: pattern alternato
    for (int i = 0; i < NUM_LEDS_12; i++) {
        leds_12[i] = ((i + (elapsed / 200)) % 2 == 0) ? CRGB::Red : CRGB::Black;
    }
}

void updateProgramLEDs() {
    // Comportamento diverso per modalità manuale vs automatica
    if (control_system.manual_mode) {
        // MODALITÀ MANUALE: Ore funzionamento + Attuatori attivi
        updateManualModeLEDs();
    } else if (program_execution.is_running && program_execution.current_program) {
        // MODALITÀ AUTOMATICA: Ore rimanenti + Fasi programma
        updateAutomaticModeLEDs();
    } else {
        // MODALITÀ NORMALE: Sistema status
        updateNormalLEDs();
    }
}

void updateErrorLEDs() {
    // Protezione: se in modalità demo, usa modalità normale invece che errore
    if (isDemoModeActive()) {
        updateNormalLEDs();
        return;
    }
    
    unsigned long elapsed = millis() - led_system.animation_start;
    
    // Pattern errore: rosso lampeggiante lento
    bool error_state = (elapsed % 2000) < 1000;
    
    fill_solid(leds_24, NUM_LEDS_24, error_state ? CRGB::Red : CRGB::Black);
    fill_solid(leds_12, NUM_LEDS_12, error_state ? CRGB::Red : CRGB::Black);
}

void updateMaintenanceLEDs() {
    // Pattern manutenzione: arancione rotante
    unsigned long elapsed = millis() - led_system.animation_start;
    int rotation = (elapsed / 100) % NUM_LEDS_24;
    
    fill_solid(leds_24, NUM_LEDS_24, CRGB::Black);
    fill_solid(leds_12, NUM_LEDS_12, CRGB::Black);
    
    // LED rotanti
    for (int i = 0; i < 4; i++) {
        int pos = (rotation + i * 6) % NUM_LEDS_24;
        leds_24[pos] = CRGB::Orange;
    }
    
    // Strip 12 sincrono
    int pos_12 = (rotation / 2) % NUM_LEDS_12;
    leds_12[pos_12] = CRGB::Orange;
}

void updateSystemStatusLEDs() {
    // Strip 12: Indicatori sistema (sensori, SD, RTC, etc.)
    leds_12[0] = sensors.internal_valid ? CRGB::Green : CRGB::Red;        // Sensore interno
    leds_12[1] = sensors.external_valid ? CRGB::Green : CRGB::Yellow;     // Sensore esterno
    leds_12[2] = system_state.rtc_available ? CRGB::Green : CRGB::Red;    // RTC
    leds_12[3] = system_state.sd_available ? CRGB::Green : CRGB::Red;     // SD Card
    
    // Temperatura range (LED 4-7)
    if (sensors.internal_valid) {
        float temp_avg = (control_system.target_temp_min + control_system.target_temp_max) / 2.0;
        float temp_diff = sensors.temp_internal - temp_avg;
        
        if (abs(temp_diff) < 0.5) {
            leds_12[4] = CRGB::Green;    // Perfetto
        } else if (abs(temp_diff) < 2.0) {
            leds_12[4] = CRGB::Yellow;   // OK
        } else {
            leds_12[4] = CRGB::Red;      // Fuori range
        }
    } else {
        leds_12[4] = CRGB::Black;
    }
    
    // Umidità range (LED 5-7)
    if (sensors.internal_valid && !control_system.temp_only_mode) {
        float hum_avg = (control_system.target_hum_min + control_system.target_hum_max) / 2.0;
        float hum_diff = sensors.hum_internal - hum_avg;
        
        if (abs(hum_diff) < 2.0) {
            leds_12[5] = CRGB::Green;    // Perfetto
        } else if (abs(hum_diff) < 5.0) {
            leds_12[5] = CRGB::Yellow;   // OK
        } else {
            leds_12[5] = CRGB::Red;      // Fuori range
        }
    } else {
        leds_12[5] = control_system.temp_only_mode ? CRGB::Blue : CRGB::Black; // Blu se solo temp
    }
    
    // Modalità sistema (LED 6-11)
    leds_12[6] = control_system.manual_mode ? CRGB::Blue : CRGB::Black;
    leds_12[7] = control_system.auto_mode ? CRGB::Purple : CRGB::Black;
    leds_12[8] = emergency_system.is_active ? CRGB::Red : CRGB::Black;
    leds_12[9] = program_execution.is_running ? CRGB::Green : CRGB::Black;
    
    // Heartbeat (LED 10-11)
    bool heartbeat = (millis() % 2000) < 100;
    leds_12[10] = heartbeat ? CRGB::White : CRGB::Black;
    leds_12[11] = heartbeat ? CRGB::White : CRGB::Black;
}

ActuatorState* getActuatorState(int actuator_index) {
    switch (actuator_index) {
        case 0: return &control_system.frigorifero;
        case 1: return &control_system.riscaldatore;
        case 2: return &control_system.deumidificatore;
        case 3: return &control_system.umidificatore;
        case 4: return &control_system.ventola_in;
        case 5: return &control_system.ventola_out;
        default: return nullptr;
    }
}

// ===============================================================================
// EFFETTI SPECIALI LED
// ===============================================================================

void applyRainbowEffect() {
    led_system.rainbow_hue += 2;
    if (led_system.rainbow_hue >= 255) led_system.rainbow_hue = 0;
    
    // Applica arcobaleno solo alla strip 24
    for (int i = 0; i < NUM_LEDS_24; i++) {
        leds_24[i] = CHSV(led_system.rainbow_hue + (i * 255 / NUM_LEDS_24), 255, 200);
    }
}

void applyBreathingEffect() {
    // Effetto breathing sulla luminosità
    if (led_system.breathing_direction) {
        led_system.breathing_value += 5;
        if (led_system.breathing_value >= 255) {
            led_system.breathing_value = 255;
            led_system.breathing_direction = false;
        }
    } else {
        led_system.breathing_value -= 5;
        if (led_system.breathing_value <= 50) {
            led_system.breathing_value = 50;
            led_system.breathing_direction = true;
        }
    }
    
    FastLED.setBrightness(led_system.breathing_value);
}

void toggleLEDs() {
    led_system.leds_enabled = !led_system.leds_enabled;
    
    if (!led_system.leds_enabled) {
        fill_solid(leds_24, NUM_LEDS_24, CRGB::Black);
        fill_solid(leds_12, NUM_LEDS_12, CRGB::Black);
        FastLED.show();
    }
    
    Serial.print(F("LED "));
    Serial.println(led_system.leds_enabled ? F("ABILITATI") : F("DISABILITATI"));
}

void setBrightness(uint8_t brightness) {
    led_system.brightness = brightness;
    FastLED.setBrightness(brightness);
    
    Serial.print(F("LED luminosità: "));
    Serial.print((brightness * 100) / 255);
    Serial.println(F("%"));
}

// ===============================================================================
// SISTEMA LED RING CONCENTRICO PER PROGRAMMI
// ===============================================================================

void updateProgramRing24() {
    // Ring 24 (Esterno): ORE RIMANENTI - Ogni LED = 1 ora
    
    if (!program_execution.current_program) return;
    
    ProgramPhase* phase = &program_execution.current_program->phases[program_execution.current_phase];
    
    if (phase->duration_hours > 0) {
        // Fase a tempo: mostra ore rimanenti
        unsigned long elapsed_ms = millis() - program_execution.phase_start_time;
        unsigned long phase_duration_ms = phase->duration_hours * 3600000UL;
        unsigned long remaining_ms = (elapsed_ms < phase_duration_ms) ? 
                                   (phase_duration_ms - elapsed_ms) : 0;
        
        // Calcola ore rimanenti (arrotonda per eccesso)
        int hours_remaining = (remaining_ms + 3599999) / 3600000; // +59min59s per arrotondare
        if (hours_remaining > 24) hours_remaining = 24; // Max 24 LED
        
        // Colore basato su ore rimanenti
        CRGB hour_color;
        if (hours_remaining > 12) {
            hour_color = CRGB::Green;       // Molto tempo (>12h)
        } else if (hours_remaining > 6) {
            hour_color = CRGB::Yellow;      // Medio tempo (6-12h)
        } else if (hours_remaining > 2) {
            hour_color = CRGB::Orange;      // Poco tempo (2-6h)
        } else {
            hour_color = CRGB::Red;         // Urgente (<2h)
        }
        
        // Visualizza ore rimanenti (conto alla rovescia)
        for (int i = 0; i < NUM_LEDS_24; i++) {
            if (i < hours_remaining) {
                leds_24[i] = hour_color;
            } else {
                leds_24[i] = CRGB(10, 10, 10);  // Grigio molto scuro
            }
        }
        
        // LED lampeggiante per ora corrente
        if (hours_remaining > 0 && led_system.blink_state) {
            leds_24[hours_remaining - 1] = CRGB::White;
        }
        
    } else {
        // Fase infinita: effetto rotante
        unsigned long elapsed = millis() / 200;
        for (int i = 0; i < NUM_LEDS_24; i++) {
            int offset = (i + elapsed) % NUM_LEDS_24;
            if (offset < 4) {
                leds_24[i] = CRGB::Purple;
            } else {
                leds_24[i] = CRGB(20, 0, 20);  // Viola scuro
            }
        }
    }
}

void updatePhaseCountdownRing12() {
    // Ring 12 (Interno): FASI PROGRAMMA - Ogni LED = 1 fase
    
    if (!program_execution.current_program) return;
    
    int total_phases = program_execution.current_program->total_phases;
    int current_phase = program_execution.current_phase;
    
    // Spegni tutti i LED
    for (int i = 0; i < NUM_LEDS_12; i++) {
        leds_12[i] = CRGB::Black;
    }
    
    // Accendi LED per fasi del programma (max 12 fasi visualizzabili)
    int max_display_phases = min(total_phases, NUM_LEDS_12);
    
    for (int phase = 0; phase < max_display_phases; phase++) {
        CRGB phase_color;
        
        if (phase < current_phase) {
            // Fasi completate: Verde
            phase_color = CRGB::Green;
            
        } else if (phase == current_phase) {
            // Fase corrente: Lampeggiante o colore distintivo
            if (led_system.blink_state) {
                phase_color = CRGB::White;      // Lampeggio bianco
            } else {
                phase_color = CRGB::Blue;       // Blu per fase attiva
            }
            
        } else {
            // Fasi future: Grigio scuro
            phase_color = CRGB(30, 30, 30);
        }
        
        leds_12[phase] = phase_color;
    }
    
    // Se ci sono più di 12 fasi, indica overflow con ultimo LED rosso
    if (total_phases > NUM_LEDS_12) {
        leds_12[NUM_LEDS_12 - 1] = CRGB::Red;  // Indica "più fasi"
    }
    
    // Indica fase infinita con effetto speciale
    if (current_phase < total_phases) {
        ProgramPhase* phase = &program_execution.current_program->phases[current_phase];
        if (phase->is_final_phase) {
            // Fase infinita: breathing sull'LED della fase corrente
            uint8_t breathing = (sin(millis() / 1500.0) * 100) + 155;
            leds_12[current_phase] = CRGB(breathing, 0, breathing); // Viola pulsante
        }
    }
}

void updateManualModeLEDs() {
    // === MODALITÀ MANUALE ===
    
    // Ring 24: ORE DI FUNZIONAMENTO (conto in avanti)
    updateManualHoursRing24();
    
    // Ring 12: ATTUATORI ATTIVI (legenda colori)
    updateActuatorsRing12();
}

void updateAutomaticModeLEDs() {
    // === MODALITÀ AUTOMATICA ===
    
    // Ring 24: ORE RIMANENTI (conto alla rovescia)
    updateProgramRing24();
    
    // Ring 12: FASI PROGRAMMA
    updatePhaseCountdownRing12();
}

void updateManualHoursRing24() {
    // Ring 24: ORE DI FUNZIONAMENTO in modalità manuale
    
    // Calcola ore di funzionamento dall'avvio modalità manuale
    unsigned long elapsed_ms = millis() - system_state.manual_mode_start;
    int hours_running = elapsed_ms / 3600000; // Ore complete
    
    // Limita a 24 LED
    if (hours_running > 24) hours_running = 24;
    
    // Colore basato su ore di funzionamento
    CRGB hour_color;
    if (hours_running < 6) {
        hour_color = CRGB::Green;       // Funzionamento recente
    } else if (hours_running < 12) {
        hour_color = CRGB::Yellow;      // Funzionamento normale
    } else if (hours_running < 18) {
        hour_color = CRGB::Orange;      // Funzionamento prolungato
    } else {
        hour_color = CRGB::Red;         // Funzionamento molto lungo
    }
    
    // Visualizza ore di funzionamento (conto in avanti)
    for (int i = 0; i < NUM_LEDS_24; i++) {
        if (i < hours_running) {
            leds_24[i] = hour_color;
        } else {
            leds_24[i] = CRGB(5, 5, 5);  // Grigio molto scuro
        }
    }
    
    // LED lampeggiante per ora corrente
    if (hours_running < NUM_LEDS_24 && led_system.blink_state) {
        leds_24[hours_running] = CRGB::White;
    }
}

void updateActuatorsRing12() {
    // Ring 12: ATTUATORI ATTIVI con legenda colori
    
    // Spegni tutti i LED
    for (int i = 0; i < NUM_LEDS_12; i++) {
        leds_12[i] = CRGB::Black;
    }
    
    // Legenda colori attuatori (posizioni fisse)
    // LED 0-5: Attuatori principali
    // LED 6-11: Indicatori sistema/extra
    
    int led_pos = 0;
    
    // LED 0: Frigorifero
    if (control_system.frigorifero.is_active) {
        leds_12[0] = CRGB::Blue;
        if (control_system.frigorifero.protection_active && led_system.blink_state) {
            leds_12[0] = CRGB::Black;  // Blink se in protezione
        }
    } else {
        leds_12[0] = CRGB(10, 10, 30);  // Blu molto scuro (spento)
    }
    
    // LED 1: Riscaldatore
    if (control_system.riscaldatore.is_active) {
        leds_12[1] = CRGB::Red;
        if (control_system.riscaldatore.protection_active && led_system.blink_state) {
            leds_12[1] = CRGB::Black;
        }
    } else {
        leds_12[1] = CRGB(30, 10, 10);  // Rosso molto scuro
    }
    
    // LED 2: Deumidificatore
    if (control_system.deumidificatore.is_active) {
        leds_12[2] = CRGB::Orange;
        if (control_system.deumidificatore.protection_active && led_system.blink_state) {
            leds_12[2] = CRGB::Black;
        }
    } else {
        leds_12[2] = CRGB(30, 15, 5);   // Arancione molto scuro
    }
    
    // LED 3: Umidificatore
    if (control_system.umidificatore.is_active) {
        leds_12[3] = CRGB::Cyan;
        if (control_system.umidificatore.protection_active && led_system.blink_state) {
            leds_12[3] = CRGB::Black;
        }
    } else {
        leds_12[3] = CRGB(5, 30, 30);   // Ciano molto scuro
    }
    
    // LED 4: Ventola IN
    if (control_system.ventola_in.is_active) {
        leds_12[4] = CRGB::Green;
        if (control_system.ventola_in.protection_active && led_system.blink_state) {
            leds_12[4] = CRGB::Black;
        }
    } else {
        leds_12[4] = CRGB(10, 30, 10);  // Verde molto scuro
    }
    
    // LED 5: Ventola OUT
    if (control_system.ventola_out.is_active) {
        leds_12[5] = CRGB::Lime;
        if (control_system.ventola_out.protection_active && led_system.blink_state) {
            leds_12[5] = CRGB::Black;
        }
    } else {
        leds_12[5] = CRGB(20, 30, 5);   // Lime molto scuro
    }
    
    // LED 6-11: Indicatori sistema
    leds_12[6] = sensors.internal_valid ? CRGB::Green : CRGB::Red;      // Sensore interno
    leds_12[7] = sensors.external_valid ? CRGB::Green : CRGB::Yellow;   // Sensore esterno
    leds_12[8] = system_state.rtc_available ? CRGB::Green : CRGB::Red;  // RTC
    leds_12[9] = system_state.sd_available ? CRGB::Green : CRGB::Red;   // SD Card
    
    // LED 10-11: Heartbeat
    bool heartbeat = (millis() % 2000) < 100;
    leds_12[10] = heartbeat ? CRGB::White : CRGB::Black;
    leds_12[11] = heartbeat ? CRGB::White : CRGB::Black;
}

void handleMillisOverflow() {
    // Gestione overflow millis() per tutti i timer LED
    unsigned long current_time = millis();
    
    // Controlla se millis() ha fatto overflow (torna a 0)
    static unsigned long last_millis = 0;
    if (current_time < last_millis) {
        // Overflow rilevato - aggiorna tutti i timestamp LED
        led_system.animation_start = current_time;
        led_system.last_update = current_time;
        led_system.last_blink_time = current_time;
        
        Serial.println(F("Overflow millis() gestito - Timer LED aggiornati"));
    }
    last_millis = current_time;
}

// ===============================================================================
// FUNZIONI FILTRAGGIO SENSORI AVANZATO
// ===============================================================================

void applySensorFilters(float temp_raw, float hum_raw) {
    // Applica media mobile a 3 campioni per stabilizzare i dati
    
    // Aggiorna buffer circolare
    sensors.temp_buffer[sensors.buffer_index] = temp_raw;
    sensors.hum_buffer[sensors.buffer_index] = hum_raw;
    sensors.buffer_index = (sensors.buffer_index + 1) % 3;
    
    // Calcola media mobile se buffer inizializzato
    if (!sensors.filter_initialized) {
        // Prime letture: usa valore grezzo
        sensors.temp_internal_filtered = temp_raw;
        sensors.hum_internal_filtered = hum_raw;
        
        // Controlla se buffer pieno
        bool buffer_full = true;
        for (int i = 0; i < 3; i++) {
            if (isnan(sensors.temp_buffer[i]) || isnan(sensors.hum_buffer[i])) {
                buffer_full = false;
                break;
            }
        }
        sensors.filter_initialized = buffer_full;
    } else {
        // Calcola media mobile a 3 campioni
        float temp_sum = 0, hum_sum = 0;
        for (int i = 0; i < 3; i++) {
            temp_sum += sensors.temp_buffer[i];
            hum_sum += sensors.hum_buffer[i];
        }
        sensors.temp_internal_filtered = temp_sum / 3.0;
        sensors.hum_internal_filtered = hum_sum / 3.0;
    }
}

bool detectSensorSpike(float new_value, float filtered_value) {
    // Rileva spike anomali nei dati sensori
    if (isnan(filtered_value) || !sensors.filter_initialized) {
        return false; // Prima lettura, accetta sempre
    }
    
    float deviation = abs(new_value - filtered_value);
    
    // Soglie per rilevamento spike (basate su esperienza pratica)
    float temp_spike_threshold = 5.0;  // ±5°C in una lettura è sospetto
    float hum_spike_threshold = 15.0;  // ±15% in una lettura è sospetto
    
    // Controlla se il nuovo valore è troppo diverso dal valore filtrato
    if (deviation > temp_spike_threshold || deviation > hum_spike_threshold) {
        Serial.print(F("SPIKE rilevato: delta="));
        Serial.print(deviation, 2);
        Serial.print(F(" (soglia="));
        Serial.print(temp_spike_threshold, 1);
        Serial.println(F(")"));
        return true;
    }
    
    return false;
}

// ===============================================================================
// FUNZIONI DIAGNOSTICA E TEST PRODUZIONE
// ===============================================================================

void runDiagnosticTests() {
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║     DIAGNOSTICA SISTEMA PRODUZIONE  ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    
    // Test memoria
    validateMemoryUsage();
    
    // Test sensori
    stressTestSensors();
    
    // Test attuatori
    Serial.println(F("=== TEST ATTUATORI ==="));
    for (int i = 0; i < 6; i++) {
        ActuatorState* actuator = getActuatorState(i);
        if (actuator) {
            Serial.print(F("Attuatore "));
            Serial.print(i);
            Serial.print(F(": "));
            Serial.print(actuator->is_active ? F("ON") : F("OFF"));
            Serial.print(F(", Protezione: "));
            Serial.print(actuator->protection_active ? F("SI") : F("NO"));
            Serial.print(F(", Errori: "));
            Serial.println(actuator->error_count);
        }
    }
    
    printProductionReport();
}

void validateMemoryUsage() {
    Serial.println(F("=== ANALISI MEMORIA ==="));
    
    // Calcolo memoria libera approssimativo
    extern int __heap_start, *__brkval;
    int free_memory;
    
    if ((int)__brkval == 0) {
        free_memory = ((int)&free_memory) - ((int)&__heap_start);
    } else {
        free_memory = ((int)&free_memory) - ((int)__brkval);
    }
    
    Serial.print(F("RAM libera: "));
    Serial.print(free_memory);
    Serial.println(F(" bytes"));
    
    if (free_memory < 1000) {
        Serial.println(F("⚠️  ATTENZIONE: Memoria RAM critica!"));
    } else if (free_memory < 1500) {
        Serial.println(F("⚠️  Memoria RAM al limite"));
    } else {
        Serial.println(F("✅ Memoria RAM OK"));
    }
}

void stressTestSensors() {
    Serial.println(F("=== STRESS TEST SENSORI ==="));
    
    // Protezione: aggiorna display solo se disponibile e non durante inizializzazione
    static bool display_safe = false;
    if (millis() > 15000) { // Dopo 15 secondi dall'avvio
        display_safe = true;
        // Mostra messaggio di test in corso sul display
        tft.fillRect(10, SCREEN_HEIGHT - 160, SCREEN_WIDTH - 20, 50, COLOR_BLACK);
        tft.drawRect(10, SCREEN_HEIGHT - 160, SCREEN_WIDTH - 20, 50, COLOR_ORANGE);
        tft.setTextColor(COLOR_ORANGE);
        tft.setTextSize(TEXT_SIZE_SMALL);
        tft.setCursor(15, SCREEN_HEIGHT - 150);
        tft.println(F("STRESS TEST IN CORSO..."));
    }
    
    int successful_reads = 0;
    
    for (int i = 0; i < 5; i++) {
        // Aggiorna display con progresso (solo se sicuro)
        if (display_safe) {
            tft.setCursor(15, SCREEN_HEIGHT - 130);
            tft.setTextColor(COLOR_WHITE);
            tft.print(F("Test "));
            tft.print(i + 1);
            tft.print(F("/5..."));
        }
        
        Serial.print(F("Test "));
        Serial.print(i + 1);
        Serial.print(F("/5... "));
        
        bool read_success = readSensors();
        if (read_success && sensors.internal_valid) {
            successful_reads++;
            if (display_safe) {
                tft.setTextColor(COLOR_GREEN);
                tft.print(F(" OK"));
            }
            Serial.print(F("OK - T:"));
            Serial.print(sensors.temp_internal, 1);
            Serial.println(F("°C"));
        } else {
            if (display_safe) {
                tft.setTextColor(COLOR_RED);
                tft.print(F(" FAIL"));
            }
            Serial.println(F("FALLITO"));
        }
        
        delay(800);
        wdt_reset();
        
        // Cancella riga per prossimo test (solo se sicuro)
        if (i < 4 && display_safe) {
            tft.fillRect(15, SCREEN_HEIGHT - 130, 200, 15, COLOR_BLACK);
        }
    }
    
    float success_rate = (float)successful_reads / 5.0 * 100.0;
    
    // Mostra risultato finale (solo se sicuro)
    if (display_safe) {
        tft.fillRect(15, SCREEN_HEIGHT - 120, 300, 15, COLOR_BLACK);
        tft.setCursor(15, SCREEN_HEIGHT - 120);
        tft.setTextColor(COLOR_CYAN);
        tft.print(F("Successo: "));
        tft.print(success_rate, 1);
        tft.print(F("% ("));
        tft.print(successful_reads);
        tft.print(F("/5)"));
    }
    
    Serial.print(F("Tasso successo: "));
    Serial.print(success_rate, 1);
    Serial.println(F("%"));
    
    // Mantieni risultato per 3 secondi
    delay(3000);
}

void logSystemHealth() {
    // Log periodico salute sistema
    Serial.print(F("Uptime: "));
    Serial.print(millis() / 3600000);
    Serial.print(F("h, Sensori: "));
    Serial.print(sensors.internal_valid ? F("OK") : F("ERR"));
    Serial.print(F(", Emergenze: "));
    Serial.println(emergency_system.emergency_episodes);
    wdt_reset();
}

void printProductionReport() {
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║        REPORT PRODUZIONE             ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    
    unsigned long uptime_hours = millis() / 3600000;
    Serial.print(F("Uptime: "));
    Serial.print(uptime_hours);
    Serial.println(F(" ore"));
    
    if (emergency_system.emergency_episodes == 0) {
        Serial.println(F("✅ Nessuna emergenza"));
    } else {
        Serial.print(F("⚠️  Emergenze: "));
        Serial.println(emergency_system.emergency_episodes);
    }
    
    Serial.println(sensors.internal_valid ? 
                   F("🟢 SISTEMA: STABILE") : 
                   F("🟡 SISTEMA: VERIFICARE SENSORI"));
}

// ===============================================================================
// SISTEMA RETROILLUMINAZIONE DISPLAY
// ===============================================================================

void initializeBacklight() {
    // Configura pin PWM per controllo retroilluminazione
    pinMode(BACKLIGHT_PIN, OUTPUT);
    
    // Imposta livello iniziale massimo
    analogWrite(BACKLIGHT_PIN, BACKLIGHT_MAX_LEVEL);
    
    Serial.print(F("     Retroilluminazione: Pin "));
    Serial.print(BACKLIGHT_PIN);
    Serial.print(F(" - Livello iniziale: "));
    Serial.print(BACKLIGHT_MAX_LEVEL);
    Serial.println(F("/255"));
}

void updateBacklight() {
    unsigned long current_time = millis();
    
    // Evita aggiornamenti troppo frequenti (ogni 50ms minimo)
    if (current_time - backlight_system.last_update < 50) {
        return;
    }
    backlight_system.last_update = current_time;
    
    // Se disabilitato, spegni completamente
    if (!backlight_system.is_enabled) {
        analogWrite(BACKLIGHT_PIN, 0);
        return;
    }
    
    // Modalità emergenza: sempre acceso al massimo
    if (backlight_system.emergency_mode || system_state.emergency_mode) {
        setBacklightLevel(BACKLIGHT_MAX_LEVEL);
        return;
    }
    
    // Gestione fade progressivo
    if (backlight_system.fade_in_progress) {
        unsigned long fade_elapsed = current_time - backlight_system.fade_start_time;
        
        if (fade_elapsed >= backlight_system.fade_duration) {
            // Fade completato
            backlight_system.fade_in_progress = false;
            setBacklightLevel(backlight_system.fade_target_level);
        } else {
            // Calcola livello intermedio del fade
            float progress = (float)fade_elapsed / backlight_system.fade_duration;
            int current_fade_level = backlight_system.fade_start_level + 
                                   (backlight_system.fade_target_level - backlight_system.fade_start_level) * progress;
            
            setBacklightLevel(current_fade_level);
        }
        return;
    }
    
    // Auto-dimming basato su inattività
    if (backlight_system.auto_dim_enabled && !backlight_system.manual_control) {
        unsigned long inactive_time = current_time - backlight_system.last_activity;
        
        // Gestione overflow millis()
        if (current_time < backlight_system.last_activity) {
            backlight_system.last_activity = current_time;
            inactive_time = 0;
        }
        
        if (inactive_time >= BACKLIGHT_TIMEOUT) {
            // Passa a modalità standby
            if (backlight_system.target_level != backlight_system.standby_level) {
                fadeBacklightTo(backlight_system.standby_level, 2000); // Fade 2s
                // Serial.println(F("Backlight: Auto-dim attivo")); // Debug rimosso
            }
        } else {
            // Mantieni livello normale
            if (backlight_system.target_level == backlight_system.standby_level) {
                fadeBacklightTo(backlight_system.day_level, 1000); // Fade 1s
            }
        }
    }
    
    // Applica livello corrente
    if (backlight_system.current_level != backlight_system.target_level && 
        !backlight_system.fade_in_progress) {
        setBacklightLevel(backlight_system.target_level);
    }
}

void setBacklightLevel(uint8_t level) {
    // Limita il range
    level = constrain(level, 0, 255);
    
    backlight_system.current_level = level;
    backlight_system.target_level = level;
    
    // Applica PWM al pin
    analogWrite(BACKLIGHT_PIN, level);
    
    // Debug rimosso per ridurre spam seriale
}

void fadeBacklightTo(uint8_t target_level, unsigned long duration_ms) {
    // Limita il range
    target_level = constrain(target_level, 0, 255);
    
    // Evita fade inutili
    if (target_level == backlight_system.current_level) {
        return;
    }
    
    // Imposta parametri fade
    backlight_system.fade_in_progress = true;
    backlight_system.fade_start_time = millis();
    backlight_system.fade_start_level = backlight_system.current_level;
    backlight_system.fade_target_level = target_level;
    backlight_system.fade_duration = duration_ms;
    
    // Debug fade rimosso per ridurre spam seriale
}

void backlight_activity_detected() {
    // Registra timestamp ultima attività
    backlight_system.last_activity = millis();
    
    // Se in standby, riattiva immediatamente
    if (backlight_system.current_level == backlight_system.standby_level) {
        fadeBacklightTo(backlight_system.day_level, 500); // Fade rapido 0.5s
        Serial.println(F("Backlight: Attività rilevata - Riattivazione"));
    }
}

void toggleBacklight() {
    if (backlight_system.is_enabled) {
        // Spegni
        backlight_system.is_enabled = false;
        fadeBacklightTo(0, 1000);
        Serial.println(F("Backlight: Spento"));
    } else {
        // Accendi
        backlight_system.is_enabled = true;
        fadeBacklightTo(backlight_system.day_level, 1000);
        Serial.println(F("Backlight: Acceso"));
    }
}

void setBacklightProfile(int profile) {
    uint8_t new_level;
    
    switch (profile) {
        case 0: // Profilo diurno
            new_level = backlight_system.day_level;
            Serial.println(F("Backlight: Profilo DIURNO"));
            break;
            
        case 1: // Profilo notturno
            new_level = backlight_system.night_level;
            Serial.println(F("Backlight: Profilo NOTTURNO"));
            break;
            
        case 2: // Profilo standby
            new_level = backlight_system.standby_level;
            Serial.println(F("Backlight: Profilo STANDBY"));
            break;
            
        default:
            new_level = backlight_system.day_level;
            Serial.println(F("Backlight: Profilo DEFAULT"));
            break;
    }
    
    // Disabilita controllo manuale per permettere auto-dim
    backlight_system.manual_control = false;
    
    // Applica nuovo profilo con fade
    fadeBacklightTo(new_level, 1500);
}

void backlight_emergency_mode(bool enable) {
    backlight_system.emergency_mode = enable;
    
    if (enable) {
        // Modalità emergenza: massima luminosità
        setBacklightLevel(BACKLIGHT_MAX_LEVEL);
        Serial.println(F("Backlight: MODALITÀ EMERGENZA attivata"));
    } else {
        // Torna al controllo normale
        backlight_system.emergency_mode = false;
        fadeBacklightTo(backlight_system.day_level, 1000);
        Serial.println(F("Backlight: Modalità emergenza disattivata"));
    }
}

// ===============================================================================
// MODALITÀ DEMO
// ===============================================================================

void toggleDemoMode() {
    system_state.demo_mode_forced = !system_state.demo_mode_forced;
    updateDemoModeStatus();
    
    if (system_state.demo_mode_forced) {
        Serial.println(F(""));
        Serial.println(F("╔══════════════════════════════════════╗"));
        Serial.println(F("║   MODALITÀ DEMO ATTIVATA MANUALMENTE ║"));
        Serial.println(F("║   DATI SENSORI SIMULATI              ║"));
        Serial.println(F("╚══════════════════════════════════════╝"));
        
        // Inizializza dati simulati immediatamente
        sensors.temp_internal = 12.5;
        sensors.hum_internal = 60.0;
        sensors.temp_external = 15.0;
        sensors.hum_external = 55.0;
        sensors.internal_valid = true;
        sensors.external_valid = true;
        sensors.internal_error_count = 0;
        sensors.external_error_count = 0;
        
    } else {
        Serial.println(F(""));
        Serial.println(F("╔══════════════════════════════════════╗"));
        Serial.println(F("║   MODALITÀ DEMO DISATTIVATA          ║"));
        Serial.println(F("║   RITORNO AI SENSORI REALI           ║"));
        Serial.println(F("╚══════════════════════════════════════╝"));
        
        // Reset validità sensori per forzare una nuova lettura
        sensors.internal_valid = false;
        sensors.external_valid = false;
    }
    
    // Forza aggiornamento display
    ui_state.screen_needs_redraw = true;
    ui_state.force_full_redraw = true;
}

bool isDemoModeActive() {
    return system_state.demo_mode_active;
}

void updateDemoModeStatus() {
    // La modalità demo è attiva se:
    // 1. È forzata manualmente, OPPURE
    // 2. Non ci sono sensori disponibili automaticamente
    bool was_demo_active = system_state.demo_mode_active;
    
    system_state.demo_mode_active = system_state.demo_mode_forced || 
                                   (!system_state.am2315_available && !system_state.dht11_available);
    
    // Log cambio stato modalità demo
    if (was_demo_active != system_state.demo_mode_active) {
        if (system_state.demo_mode_active) {
            Serial.print(F("Demo mode ATTIVATA - Tipo: "));
            if (system_state.demo_mode_forced) {
                Serial.println(F("FORZATA"));
            } else {
                Serial.println(F("AUTOMATICA (no sensori)"));
            }
        } else {
            Serial.println(F("Demo mode DISATTIVATA"));
        }
    }
}

// ===============================================================================
// COMANDI SERIALI
// ===============================================================================

void handleSerialCommands() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toLowerCase();
        
        if (command == "demo") {
            if (!system_state.demo_mode_forced) {
                system_state.demo_mode_forced = true;
                updateDemoModeStatus();
                Serial.println(F(""));
                Serial.println(F("✅ MODALITÀ DEMO ATTIVATA DA SERIALE"));
                Serial.println(F("   Sistema ora usa dati simulati"));
                Serial.println(F("   Digita 'nodemo' per disattivare"));
                
                // Inizializza dati demo immediatamente
                sensors.temp_internal = 12.5;
                sensors.hum_internal = 60.0;
                sensors.temp_external = 15.0;
                sensors.hum_external = 55.0;
                sensors.internal_valid = true;
                sensors.external_valid = true;
                sensors.internal_error_count = 0;
                sensors.external_error_count = 0;
                sensors.last_read_time = millis();
                
                // Forza ridisegno display e cambio schermata
                ui_state.current_screen = SCREEN_MAIN_DASHBOARD;
                ui_state.screen_needs_redraw = true;
                ui_state.force_full_redraw = true;
                
                Serial.println(F("   🖥️  Aggiornamento display forzato..."));
            } else {
                Serial.println(F("⚠️  Modalità demo già attiva"));
            }
            
        } else if (command == "nodemo") {
            if (system_state.demo_mode_forced) {
                system_state.demo_mode_forced = false;
                updateDemoModeStatus();
                Serial.println(F(""));
                Serial.println(F("✅ MODALITÀ DEMO DISATTIVATA DA SERIALE"));
                Serial.println(F("   Sistema torna ai sensori reali"));
                
                // Reset validità sensori
                sensors.internal_valid = false;
                sensors.external_valid = false;
                
                // Forza ridisegno display
                ui_state.screen_needs_redraw = true;
                ui_state.force_full_redraw = true;
            } else {
                Serial.println(F("⚠️  Modalità demo non era attiva"));
            }
            
        } else if (command == "status") {
            Serial.println(F(""));
            Serial.println(F("📊 STATUS SISTEMA STAGIONINO"));
            Serial.println(F("════════════════════════════"));
            Serial.print(F("Demo Mode: "));
            if (isDemoModeActive()) {
                if (system_state.demo_mode_forced) {
                    Serial.println(F("ATTIVA (FORZATA)"));
                } else {
                    Serial.println(F("ATTIVA (AUTO)"));
                }
            } else {
                Serial.println(F("DISATTIVA"));
            }
            
            Serial.print(F("Sensori AM2315: "));
            Serial.println(system_state.am2315_available ? F("OK") : F("NON RILEVATO"));
            
            Serial.print(F("Sensore DHT11: "));
            Serial.println(system_state.dht11_available ? F("OK") : F("NON RILEVATO"));
            
            Serial.print(F("SD Card: "));
            Serial.println(system_state.sd_available ? F("OK") : F("NON RILEVATA"));
            
            Serial.print(F("RTC: "));
            Serial.println(system_state.rtc_available ? F("OK") : F("NON RILEVATO"));
            
            if (sensors.internal_valid) {
                Serial.print(F("Temp/Hum Int: "));
                Serial.print(sensors.temp_internal, 1);
                Serial.print(F("°C / "));
                Serial.print(sensors.hum_internal, 1);
                Serial.println(F("%"));
            }
            
            if (sensors.external_valid) {
                Serial.print(F("Temp/Hum Ext: "));
                Serial.print(sensors.temp_external, 1);
                Serial.print(F("°C / "));
                Serial.print(sensors.hum_external, 1);
                Serial.println(F("%"));
            }
            
            Serial.print(F("Uptime: "));
            Serial.print(millis() / 3600000);
            Serial.println(F(" ore"));
            
            extern int __heap_start, *__brkval;
            int free_memory = (int) &free_memory - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
            Serial.print(F("RAM libera: "));
            Serial.print(free_memory);
            Serial.println(F(" bytes"));
            Serial.println(F(""));
            
        } else if (command == "backlight") {
            Serial.println(F(""));
            Serial.println(F("🔆 TEST BACKLIGHT - Pin "));
            Serial.print(BACKLIGHT_PIN);
            Serial.println(F(""));
            Serial.println(F("Test sequenza luminosità..."));
            
            // Test sequenza backlight
            for (int level = 0; level <= 255; level += 51) {
                analogWrite(BACKLIGHT_PIN, level);
                Serial.print(F("  Livello: "));
                Serial.print(level);
                Serial.print(F("/255 ("));
                Serial.print((level * 100) / 255);
                Serial.println(F("%)"));
                delay(1000);
            }
            
            // Torna al livello normale
            analogWrite(BACKLIGHT_PIN, BACKLIGHT_MAX_LEVEL);
            Serial.println(F("Test completato - ritorno livello normale"));
            Serial.println(F(""));
            
        } else if (command == "refresh") {
            Serial.println(F(""));
            Serial.println(F("🔄 FORZA AGGIORNAMENTO DISPLAY"));
            
            // Cancella tutto lo schermo
            tft.fillScreen(0x0000); // Schermo nero
            delay(500);
            
            // Forza ridisegno completo
            ui_state.current_screen = SCREEN_MAIN_DASHBOARD;
            ui_state.screen_needs_redraw = true;
            ui_state.force_full_redraw = true;
            
            // Aggiorna immediatamente
            updateDisplay();
            
            Serial.println(F("   Display pulito e ridisegnato"));
            Serial.println(F(""));
            
        } else if (command == "testdisplay") {
            Serial.println(F(""));
            Serial.println(F("🖥️  TEST DIAGNOSTICO DISPLAY"));
            Serial.println(F("════════════════════════════"));
            
            // Rilegge ID display
            uint16_t ID = tft.readID();
            Serial.print(F("Controller ID: 0x"));
            Serial.println(ID, HEX);
            
            // Informazioni display
            Serial.print(F("Risoluzione: "));
            Serial.print(tft.width());
            Serial.print(F("x"));
            Serial.println(tft.height());
            
            // Test pattern colorati (basato su forum Arduino)
            Serial.println(F("Esecuzione test pattern..."));
            
            // Test rosso
            tft.fillScreen(0xF800);
            Serial.println(F("  - Schermo ROSSO (2s)"));
            delay(2000);
            
            // Test verde
            tft.fillScreen(0x07E0);
            Serial.println(F("  - Schermo VERDE (2s)"));
            delay(2000);
            
            // Test blu
            tft.fillScreen(0x001F);
            Serial.println(F("  - Schermo BLU (2s)"));
            delay(2000);
            
            // Test bianco
            tft.fillScreen(0xFFFF);
            Serial.println(F("  - Schermo BIANCO (2s)"));
            delay(2000);
            
            // Test nero
            tft.fillScreen(0x0000);
            Serial.println(F("  - Schermo NERO (2s)"));
            delay(2000);
            
            // Ripristina display normale
            ui_state.screen_needs_redraw = true;
            ui_state.force_full_redraw = true;
            Serial.println(F("Test completato - display ripristinato"));
            Serial.println(F(""));
            
        } else if (command == "testtouch") {
            Serial.println(F(""));
            Serial.println(F("👆 TEST DIAGNOSTICO TOUCHSCREEN"));
            Serial.println(F("═══════════════════════════════════"));
            Serial.println(F("Tocca lo schermo per 10 secondi..."));
            Serial.println(F("(soluzione pin condivisi implementata)"));
            
            unsigned long test_start = millis();
            int touch_count = 0;
            
            while (millis() - test_start < 10000) {
                if (Touch_getXY()) {
                    touch_count++;
                    Serial.print(F("Touch #"));
                    Serial.print(touch_count);
                    Serial.print(F(" - X: "));
                    Serial.print(pixel_x);
                    Serial.print(F(", Y: "));
                    Serial.print(pixel_y);
                    Serial.println(F(" (pin ripristinati)"));
                    
                    // Test visivo: disegna un punto sullo schermo
                    tft.fillCircle(pixel_x, pixel_y, 3, 0xFFE0); // Punto giallo
                    
                    delay(200); // Evita spam
                }
                delay(50);
            }
            
            Serial.print(F("Test completato - "));
            Serial.print(touch_count);
            Serial.println(F(" tocchi rilevati"));
            
            // Ripristina display
            ui_state.screen_needs_redraw = true;
            ui_state.force_full_redraw = true;
            Serial.println(F("Display ripristinato"));
            Serial.println(F(""));
            
        } else if (command == "dashboard") {
            Serial.println(F(""));
            Serial.println(F("🖥️  FORZA DASHBOARD PRINCIPALE"));
            
            // Assicura modalità demo attiva
            if (!isDemoModeActive()) {
                Serial.println(F("Attivando modalità demo..."));
                system_state.demo_mode_forced = true;
                system_state.demo_mode_active = true;
                updateDemoModeStatus();
            }
            
            // SOLUZIONE HARDWARE: Reset completo pin condivisi
            Serial.println(F("Reset pin condivisi TFT/Touch/SD..."));
            spi_deselect_all();
            delay(100);
            
            // Reset display driver
            Serial.println(F("Re-inizializzazione display driver..."));
            uint16_t ID = tft.readID();
            Serial.print(F("Display ID: 0x"));
            Serial.println(ID, HEX);
            tft.begin(ID);
            tft.setRotation(1);
            
            // Test pattern per verificare display
            Serial.println(F("Test pattern colori..."));
            tft.fillScreen(0xF800); // Rosso
            delay(500);
            tft.fillScreen(0x07E0); // Verde  
            delay(500);
            tft.fillScreen(0x001F); // Blu
            delay(500);
            tft.fillScreen(0x0000); // Nero
            delay(500);
            
            // Forza ridisegno dashboard
            ui_state.current_screen = SCREEN_MAIN_DASHBOARD;
            ui_state.screen_needs_redraw = true;
            ui_state.force_full_redraw = true;
            
            Serial.println(F("Disegnando dashboard..."));
            drawMainDashboard();
            Serial.println(F("Dashboard forzata completata"));
            Serial.println(F(""));
            
        } else if (command == "tfttest") {
            Serial.println(F(""));
            Serial.println(F("🎨 TEST TFT BASE - Solo Colori"));
            
            // Reset pin condivisi prima del test
            spi_deselect_all();
            delay(100);
            
            // Test colori di base
            Serial.println(F("Rosso..."));
            tft.fillScreen(0xF800);
            delay(2000);
            
            Serial.println(F("Verde..."));
            tft.fillScreen(0x07E0);
            delay(2000);
            
            Serial.println(F("Blu..."));
            tft.fillScreen(0x001F);
            delay(2000);
            
            Serial.println(F("Bianco..."));
            tft.fillScreen(0xFFFF);
            delay(2000);
            
            Serial.println(F("Nero..."));
            tft.fillScreen(0x0000);
            delay(1000);
            
            Serial.println(F("Test TFT completato"));
            Serial.println(F(""));
            
        } else if (command == "uiinfo") {
            Serial.println(F(""));
            Serial.println(F("📱 STATO INTERFACCIA UTENTE"));
            Serial.println(F("═══════════════════════════"));
            Serial.print(F("Schermata corrente: "));
            Serial.println(ui_state.current_screen);
            Serial.print(F("Screen needs redraw: "));
            Serial.println(ui_state.screen_needs_redraw ? F("SI") : F("NO"));
            Serial.print(F("Force full redraw: "));
            Serial.println(ui_state.force_full_redraw ? F("SI") : F("NO"));
            Serial.print(F("Ultimo aggiornamento: "));
            Serial.print(ui_state.last_screen_update);
            Serial.println(F("ms"));
            Serial.print(F("Background color: 0x"));
            Serial.println(ui_state.background_color, HEX);
            Serial.print(F("Text color: 0x"));
            Serial.println(ui_state.text_color, HEX);
            Serial.println(F(""));
            
        } else if (command == "displayreset") {
            Serial.println(F(""));
            Serial.println(F("🔄 RESET COMPLETO DISPLAY"));
            
            // Reset hardware aggressivo
            Serial.println(F("1. Reset pin CS e deseleziona tutto..."));
            spi_deselect_all();
            delay(200);
            
            // Re-inizializza display completamente
            Serial.println(F("2. Re-inizializzazione completa display..."));
            uint16_t ID = tft.readID();
            Serial.print(F("   ID rilevato: 0x"));
            Serial.println(ID, HEX);
            
            // Forza ID se non rilevato
            if (ID == 0xFFFF || ID == 0x0000) {
                Serial.println(F("   ID non valido, forzando ILI9486..."));
                ID = 0x9486;
            }
            
            tft.begin(ID);
            tft.setRotation(1);
            Serial.print(F("   Risoluzione: "));
            Serial.print(tft.width());
            Serial.print(F("x"));
            Serial.println(tft.height());
            
            // Test scrittura diretta
            Serial.println(F("3. Test scrittura diretta pixel..."));
            for (int y = 0; y < 10; y++) {
                for (int x = 0; x < 10; x++) {
                    tft.drawPixel(x, y, 0xFFFF); // Quadrato bianco 10x10
                }
            }
            
            Serial.println(F("4. Test riempimento schermo..."));
            tft.fillScreen(0xF800); // Rosso pieno
            delay(1000);
            
            Serial.println(F("Reset display completato"));
            Serial.println(F(""));
            
        } else if (command == "sdtest") {
            Serial.println(F(""));
            Serial.println(F("💾 DIAGNOSI SD CARD E BUS SPI"));
            
            // 1. Stato attuale
            Serial.print(F("Stato SD attuale: "));
            Serial.println(system_state.sd_available ? F("DISPONIBILE") : F("NON DISPONIBILE"));
            
            // 2. Reset completo bus SPI
            Serial.println(F(""));
            Serial.println(F("🔄 RESET BUS SPI COMPLETO"));
            Serial.println(F("  1. Disabilitazione tutti i dispositivi SPI..."));
            
            // Forza disabilitazione di TUTTI i device SPI
            pinMode(SD_CS, OUTPUT);
            pinMode(TOUCH_CS, OUTPUT);
            digitalWrite(SD_CS, HIGH);
            digitalWrite(TOUCH_CS, HIGH);
            delay(200);
            
            Serial.println(F("  2. Reset pin SPI e inizializzazione..."));
            // Reset SPI
            SPI.end();
            delay(100);
            SPI.begin();
            SPI.setClockDivider(SPI_CLOCK_DIV2);
            delay(100);
            
            // 3. Test SD isolato
            Serial.println(F(""));
            Serial.println(F("🧪 TEST SD ISOLATO"));
            bool sd_test_ok = false;
            
            for (int test = 1; test <= 3; test++) {
                Serial.print(F("  Test "));
                Serial.print(test);
                Serial.print(F("/3: "));
                
                // Assicura isolamento
                digitalWrite(TOUCH_CS, HIGH);
                digitalWrite(SD_CS, HIGH);
                delay(100);
                
                unsigned long start = millis();
                if (SD.begin(SD_CS)) {
                    unsigned long elapsed = millis() - start;
                    Serial.print(F("✅ OK ("));
                    Serial.print(elapsed);
                    Serial.println(F("ms)"));
                    sd_test_ok = true;
                    break;
                } else {
                    unsigned long elapsed = millis() - start;
                    Serial.print(F("❌ FAIL ("));
                    Serial.print(elapsed);
                    Serial.println(F("ms)"));
                    delay(500);
                }
            }
            
            // 4. Se SD OK, test file system
            if (sd_test_ok) {
                Serial.println(F(""));
                Serial.println(F("📁 TEST FILE SYSTEM"));
                
                File root = SD.open("/");
                if (root) {
                    Serial.println(F("  ✅ Root directory accessibile"));
                    
                    // Lista files
                    Serial.println(F("  📄 Files trovati:"));
                    File entry = root.openNextFile();
                    int file_count = 0;
                    while (entry && file_count < 5) {
                        Serial.print(F("    - "));
                        Serial.print(entry.name());
                        if (entry.isDirectory()) {
                            Serial.println(F("/ (directory)"));
                        } else {
                            Serial.print(F(" ("));
                            Serial.print(entry.size());
                            Serial.println(F(" bytes)"));
                        }
                        entry.close();
                        entry = root.openNextFile();
                        file_count++;
                    }
                    root.close();
                    
                    // Test scrittura
                    Serial.println(F("  ✏️  Test scrittura..."));
                    File test = SD.open("/spi_test.txt", FILE_WRITE);
                    if (test) {
                        test.println("SPI Test OK");
                        test.close();
                        Serial.println(F("  ✅ Scrittura OK"));
                        SD.remove("/spi_test.txt");
                    } else {
                        Serial.println(F("  ❌ Scrittura FAIL"));
                    }
                } else {
                    Serial.println(F("  ❌ Root directory inaccessibile"));
                }
            }
            
            // 5. Aggiorna stato sistema
            system_state.sd_available = sd_test_ok;
            
            // 6. Reset finale bus per TFT
            Serial.println(F(""));
            Serial.println(F("🖥️  RIPRISTINO TFT DOPO TEST SD"));
            spi_deselect_all();
            delay(200);
            
            // Re-init TFT
            uint16_t tft_id = tft.readID();
            Serial.print(F("  TFT ID dopo test SD: 0x"));
            Serial.println(tft_id, HEX);
            
            if (tft_id == 0xFFFF || tft_id == 0x0000) {
                Serial.println(F("  ⚠️  TFT compromesso, re-inizializzazione..."));
                tft.begin(0x9486);
                tft.setRotation(1);
            }
            
            // Test finale TFT
            tft.fillScreen(0x07E0); // Verde
            delay(500);
            tft.fillScreen(0x0000); // Nero
            
            Serial.println(F(""));
            Serial.println(F("✅ Diagnosi SD completata"));
            Serial.print(F("   SD Card: "));
            Serial.println(sd_test_ok ? F("FUNZIONANTE") : F("PROBLEMA"));
            Serial.println(F(""));
            
        } else if (command.length() > 0) {
            Serial.println(F(""));
            Serial.println(F("❌ Comando non riconosciuto"));
            Serial.println(F("Comandi disponibili:"));
            Serial.println(F("  demo      → Attiva modalità demo"));
            Serial.println(F("  nodemo    → Disattiva modalità demo"));
            Serial.println(F("  status    → Mostra stato sistema"));
            Serial.println(F("  backlight → Test controllo retroilluminazione"));
            Serial.println(F("  refresh   → Forza aggiornamento display"));
            Serial.println(F(""));
        }
    }
}

// ===============================================================================
// GESTIONE PIN SPI CONDIVISI (TOUCH + SD)
// ===============================================================================

void spi_select_touch() {
    // Disabilita SD e abilita touch
    digitalWrite(SD_CS, HIGH);
    delayMicroseconds(10);
    digitalWrite(TOUCH_CS, LOW);
    delayMicroseconds(10);
}

void spi_select_sd() {
    // Disabilita touch e abilita SD
    digitalWrite(TOUCH_CS, HIGH);
    delayMicroseconds(10);
    digitalWrite(SD_CS, LOW);
    delayMicroseconds(10);
}

void spi_deselect_all() {
    // Disabilita entrambi
    digitalWrite(TOUCH_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
    delayMicroseconds(10);
}

// ===============================================================================
// GESTIONE TOUCH CON PIN CONDIVISI (SOLUZIONE FORUM ARDUINO)
// ===============================================================================

bool Touch_getXY(void) {
    // Funzione basata sulla soluzione del forum Arduino MCUFRIEND
    // https://forum.arduino.cc/t/3-5-tft-mcufriend-touch-screen-ili9486/612104
    
    TS_Point p = touch.getPoint();
    
    // CRITICO: Ripristina pin condivisi con il display TFT
    // I pin touch sono condivisi con i pin TFT nel shield MCUFRIEND
    pinMode(TOUCH_IRQ, INPUT);    // Ripristina pin IRQ
    pinMode(TOUCH_CS, OUTPUT);    // Ripristina pin CS
    digitalWrite(TOUCH_CS, HIGH); // Disabilita touch per permettere al TFT di funzionare
    
    // Controlla se il tocco è valido
    bool pressed = (p.z > 250 && p.z < 4000);  // Valori tipici per XPT2046
    
    if (pressed) {
        // Mappa le coordinate touch alle coordinate dello schermo
        pixel_x = map(p.x, 200, 3700, 0, tft.width());   
        pixel_y = map(p.y, 200, 3700, 0, tft.height());
        
        // Limita le coordinate ai bordi dello schermo
        pixel_x = constrain(pixel_x, 0, tft.width() - 1);
        pixel_y = constrain(pixel_y, 0, tft.height() - 1);
    }
    
    return pressed;
}

// ===============================================================================
// FINE FILE PRINCIPALE
// =============================================================================== 