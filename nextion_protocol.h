/**
 * ===============================================================================
 * STAGIONINO v1.2 - Protocollo Comunicazione Nextion Display NX4832K035
 * ===============================================================================
 * 
 * Sistema di stagionatura salumi con interfaccia Nextion
 * Hardware: Arduino Mega 2560 + Display Nextion NX4832K035 3.5"
 * 
 * Comunicazione: UART 9600 baud (Serial1: TX1=18, RX1=19)
 * 
 * Author: Stagionino Development Team
 * Version: 1.2
 * Date: 2024
 * 
 * ===============================================================================
 */

#ifndef NEXTION_PROTOCOL_H
#define NEXTION_PROTOCOL_H

#include <Arduino.h>

// ===============================================================================
// CONFIGURAZIONE NEXTION
// ===============================================================================

#define NEXTION_BAUD_RATE        9600      // Velocità comunicazione UART
#define NEXTION_TIMEOUT          1000      // Timeout comandi (ms)
#define NEXTION_BUFFER_SIZE      64        // Buffer ricezione
#define NEXTION_RETRY_COUNT      3         // Tentativi reinvio comando

// Terminatori comandi Nextion
#define NEXTION_END_CMD          "\xFF\xFF\xFF"

// ===============================================================================
// PAGINE NEXTION
// ===============================================================================

enum NextionPage {
    PAGE_DASHBOARD = 0,                    // Dashboard principale
    PAGE_SENSORS = 1,                      // Dati sensori dettagliati
    PAGE_SETTINGS = 2,                     // Impostazioni sistema
    PAGE_PROGRAMS = 3,                     // Gestione programmi
    PAGE_EMERGENCY = 4,                    // Schermata emergenza
    PAGE_DIAGNOSTIC = 5                    // Test diagnostici
};

// ===============================================================================
// CODICI EVENTI NEXTION
// ===============================================================================

// Struttura evento touch Nextion
struct NextionEvent {
    uint8_t page_id;                       // ID pagina
    uint8_t component_id;                  // ID componente
    uint8_t event_type;                    // Tipo evento (press/release)
    bool is_valid;                         // Flag validità evento
};

// Tipi eventi
#define NEXTION_EVENT_TOUCH_PRESS    0x65
#define NEXTION_EVENT_TOUCH_RELEASE  0x66
#define NEXTION_EVENT_PAGE_CHANGE    0x67

// IDs componenti pagina DASHBOARD (0) - Layout Professionale
#define DASHBOARD_BTN_TEMP_UP       0      // Pulsante SU temperatura/range [🔺]
#define DASHBOARD_BTN_TEMP_DOWN     1      // Pulsante GIÙ temperatura/range [🔻]
#define DASHBOARD_BTN_SENSORS       10     // Pulsante "SENSORI"
#define DASHBOARD_BTN_PROGRAMS      11     // Pulsante "PROGRAMMI"
#define DASHBOARD_BTN_SETTINGS      12     // Pulsante "SETTINGS"

// IDs range controls (modalità manuale) - NUOVO LAYOUT FOTO
#define DASHBOARD_BTN_TMIN          10     // Button b10 - Tmin (16°C)
#define DASHBOARD_BTN_TMAX          11     // Button b11 - Tmax (19°C)  
#define DASHBOARD_BTN_UMIN          12     // Button b12 - Umin (50%)
#define DASHBOARD_BTN_UMAX          13     // Button b13 - Umax (75%)

// IDs indicatori circolari range
#define DASHBOARD_CIRCLE_TMIN       10     // Circle c10 - Indicatore Tmin
#define DASHBOARD_CIRCLE_TMAX       11     // Circle c11 - Indicatore Tmax
#define DASHBOARD_CIRCLE_UMIN       12     // Circle c12 - Indicatore Umin
#define DASHBOARD_CIRCLE_UMAX       13     // Circle c13 - Indicatore Umax

// IDs cerchi attuatori (per toggle manuale) - LAYOUT COMPLETO
#define DASHBOARD_CIRCLE_FRIGO      0      // Circle c0 - Frigorifero
#define DASHBOARD_CIRCLE_RISC       1      // Circle c1 - Riscaldatore  
#define DASHBOARD_CIRCLE_DEUM       2      // Circle c2 - Deumidificatore
#define DASHBOARD_CIRCLE_UMID       3      // Circle c3 - Umidificatore
#define DASHBOARD_CIRCLE_VENT1      4      // Circle c4 - Ventola 1
#define DASHBOARD_CIRCLE_VENT2      5      // Circle c5 - Ventola 2

// IDs navigation buttons
#define DASHBOARD_BTN_SENSORS       20     // Button b20 - "SENSORI"
#define DASHBOARD_BTN_PROGRAMS      21     // Button b21 - "PROGRAMMI"
#define DASHBOARD_BTN_SETTINGS      22     // Button b22 - "SETTINGS"
#define DASHBOARD_BTN_EMERGENCY     23     // Button b23 - "EMERGENZA"

// Enumerazione per range selezionato
enum RangeSelected {
    RANGE_NONE = 0,                // Nessun range selezionato
    RANGE_TMIN = 1,                // Temperatura minima
    RANGE_TMAX = 2,                // Temperatura massima
    RANGE_UMIN = 3,                // Umidità minima
    RANGE_UMAX = 4                 // Umidità massima
};

// IDs componenti pagina SETTINGS (2)
#define SETTINGS_SWITCH_DEMO        0      // Switch modalità demo
#define SETTINGS_SWITCH_BACKLIGHT   1      // Switch retroilluminazione
#define SETTINGS_SLIDER_BRIGHTNESS  0      // Slider luminosità
#define SETTINGS_BTN_CALIBRATE      0      // Pulsante calibrazione
#define SETTINGS_BTN_DIAGNOSTIC     1      // Pulsante diagnostica
#define SETTINGS_BTN_DAY_PROFILE    2      // Pulsante profilo giorno
#define SETTINGS_BTN_NIGHT_PROFILE  3      // Pulsante profilo notte
#define SETTINGS_BTN_BACK           4      // Pulsante indietro

// ===============================================================================
// COLORI NEXTION (RGB565)
// ===============================================================================

#define NEXTION_BLACK               0       // Nero
#define NEXTION_WHITE               65535   // Bianco
#define NEXTION_RED                 63488   // Rosso
#define NEXTION_GREEN               2016    // Verde
#define NEXTION_BLUE                31      // Blu
#define NEXTION_YELLOW              65504   // Giallo
#define NEXTION_CYAN                2047    // Ciano
#define NEXTION_ORANGE              64512   // Arancione
#define NEXTION_GRAY                33808   // Grigio
#define NEXTION_EMERGENCY           63488   // Rosso emergenza

// ===============================================================================
// CLASSE GESTIONE NEXTION
// ===============================================================================

class NextionDisplay {
private:
    HardwareSerial* serial_port;           // Porta seriale utilizzata
    char command_buffer[NEXTION_BUFFER_SIZE]; // Buffer comandi
    char receive_buffer[NEXTION_BUFFER_SIZE]; // Buffer ricezione
    unsigned long last_command_time;       // Timestamp ultimo comando
    bool is_initialized;                   // Flag inizializzazione
    NextionPage current_page;              // Pagina corrente
    
    // Gestione errori e retry
    int last_error_code;                   // Ultimo codice errore
    int retry_count;                       // Contatore tentativi
    
public:
    // Costruttore
    NextionDisplay(HardwareSerial* serial = &Serial1);
    
    // ===============================================================================
    // INIZIALIZZAZIONE E SETUP
    // ===============================================================================
    
    bool begin(unsigned long baud_rate = NEXTION_BAUD_RATE);
    bool isReady();
    void reset();
    bool testConnection();
    
    // ===============================================================================
    // GESTIONE PAGINE
    // ===============================================================================
    
    bool setPage(NextionPage page);
    NextionPage getCurrentPage();
    bool refreshCurrentPage();
    
    // ===============================================================================
    // AGGIORNAMENTO COMPONENTI
    // ===============================================================================
    
    // Testo
    bool setText(const char* component, const char* text);
    bool setText(const char* component, float value, int decimals = 1);
    bool setText(const char* component, int value);
    
    // Colori
    bool setTextColor(const char* component, uint16_t color);
    bool setBackgroundColor(const char* component, uint16_t color);
    
    // Valori numerici
    bool setValue(const char* component, int value);
    bool getValue(const char* component, int* value);
    
    // Visibilità
    bool setVisible(const char* component, bool visible);
    
    // ===============================================================================
    // AGGIORNAMENTO DASHBOARD
    // ===============================================================================
    
    bool updateSensorData(float temp_int, float hum_int, bool int_valid,
                         float temp_ext, float hum_ext, bool ext_valid);
    bool updateActuatorStatus(bool frigorifero, bool riscaldatore, 
                             bool deumidificatore, bool umidificatore,
                             bool ventola_in, bool ventola_out);
    bool updateDemoMode(bool demo_active);
    bool updateTimestamp(unsigned long seconds_ago);
    
    // ===============================================================================
    // DASHBOARD PROFESSIONALE
    // ===============================================================================
    
    bool updateDashboardProfessional(float temp_attuale, float temp_setpoint, 
                                    float umidita_attuale, float temp_min, float temp_max,
                                    float umid_min, float umid_max, float progress_value,
                                    const char* nome_programma, int fase_corrente, int fasi_totali,
                                    const char* tempo_rimanente, bool programma_attivo);
    
    bool updateActuatorsVisual(bool frigorifero, bool riscaldatore, 
                              bool deumidificatore, bool umidificatore,
                              bool ventola1, bool ventola2);
    
    bool updateActuatorsVisualMode(bool frigorifero, bool riscaldatore, 
                                  bool deumidificatore, bool umidificatore,
                                  bool ventola1, bool ventola2, bool manual_mode);
    
    // ===============================================================================
    // CONTROLLI RANGE INTERATTIVI (MODALITÀ MANUALE)
    // ===============================================================================
    
    bool updateRangeSelection(RangeSelected selected_range);
    bool updateRangeValues(float temp_min, float temp_max, float umid_min, float umid_max);
    bool enableRangeControls(bool enable);
    bool updateMainValues(float temperatura, float umidita, float setpoint);
    
    // ===============================================================================
    // GESTIONE EVENTI
    // ===============================================================================
    
    bool pollEvents();
    NextionEvent getLastEvent();
    bool hasNewEvent();
    
    // ===============================================================================
    // COMANDI SPECIALI
    // ===============================================================================
    
    bool setBrightness(uint8_t brightness); // 0-100
    bool setBacklight(bool enabled);
    bool playSound(int sound_id, int volume);
    bool showMessage(const char* title, const char* message);
    
    // ===============================================================================
    // UTILITÀ E DEBUG
    // ===============================================================================
    
    bool sendCommand(const char* command);
    bool sendRawCommand(const char* command);
    int getLastError();
    void clearBuffer();
    void printDebugInfo();
    
private:
    // ===============================================================================
    // FUNZIONI INTERNE
    // ===============================================================================
    
    bool waitForResponse(unsigned long timeout = NEXTION_TIMEOUT);
    bool parseEvent(NextionEvent* event);
    void flushSerial();
    bool validateResponse();
    void logError(int error_code, const char* message);
};

// ===============================================================================
// ISTANZA GLOBALE
// ===============================================================================

extern NextionDisplay nextion;

// ===============================================================================
// MACRO HELPER PER AGGIORNAMENTI RAPIDI
// ===============================================================================

// Aggiornamento testo con controllo errori
#define NEXTION_SET_TEXT(comp, text) \
    do { \
        if (!nextion.setText(comp, text)) { \
            Serial.print(F("Errore aggiornamento testo: ")); \
            Serial.println(comp); \
        } \
    } while(0)

// Aggiornamento valore numerico
#define NEXTION_SET_VALUE(comp, value) \
    do { \
        if (!nextion.setValue(comp, value)) { \
            Serial.print(F("Errore aggiornamento valore: ")); \
            Serial.println(comp); \
        } \
    } while(0)

// Cambio pagina con verifica
#define NEXTION_GOTO_PAGE(page) \
    do { \
        if (!nextion.setPage(page)) { \
            Serial.print(F("Errore cambio pagina: ")); \
            Serial.println(page); \
        } \
    } while(0)

#endif // NEXTION_PROTOCOL_H
