/**
 * ===============================================================================
 * STAGIONINO v1.2 - Implementazione Comunicazione Nextion Display
 * ===============================================================================
 * 
 * Implementazione della classe NextionDisplay per gestire la comunicazione
 * UART con il display Nextion NX4832K035
 * 
 * ===============================================================================
 */

#include "nextion_protocol.h"

// Istanza globale Nextion
NextionDisplay nextion;

// Variabile per gestione eventi
static NextionEvent last_event = {0, 0, 0, false};
static bool new_event_available = false;

// ===============================================================================
// COSTRUTTORE E INIZIALIZZAZIONE
// ===============================================================================

NextionDisplay::NextionDisplay(HardwareSerial* serial) {
    serial_port = serial;
    is_initialized = false;
    current_page = PAGE_DASHBOARD;
    last_command_time = 0;
    last_error_code = 0;
    retry_count = 0;
    
    // Pulisce buffer
    memset(command_buffer, 0, NEXTION_BUFFER_SIZE);
    memset(receive_buffer, 0, NEXTION_BUFFER_SIZE);
}

bool NextionDisplay::begin(unsigned long baud_rate) {
    Serial.println(F("-> Inizializzazione Display Nextion NX4832K035"));
    
    // Inizializza porta seriale
    serial_port->begin(baud_rate);
    delay(500); // Attende stabilizzazione
    
    // Pulisce buffer seriale
    flushSerial();
    
    // Test connessione
    Serial.println(F("   Test connessione display..."));
    if (!testConnection()) {
        Serial.println(F("   ERRORE: Display non risponde"));
        return false;
    }
    
    // Reset display
    Serial.println(F("   Reset display..."));
    reset();
    delay(1000);
    
    // Imposta luminosità iniziale
    Serial.println(F("   Configurazione iniziale..."));
    setBrightness(80);
    
    // Vai alla pagina dashboard
    setPage(PAGE_DASHBOARD);
    
    // Mostra messaggio di avvio
    setText("t0", "STAGIONINO V1.2");
    setText("t1", "Sistema Stagionatura Salumi");
    setText("t2", "Inizializzazione...");
    
    is_initialized = true;
    Serial.println(F("   Display Nextion: OK"));
    
    return true;
}

bool NextionDisplay::isReady() {
    return is_initialized;
}

void NextionDisplay::reset() {
    sendCommand("rest");
    delay(1000);
    flushSerial();
}

bool NextionDisplay::testConnection() {
    // Invia comando "connect" per test connessione
    if (!sendCommand("connect")) {
        return false;
    }
    
    // Attende risposta
    unsigned long start = millis();
    while (millis() - start < NEXTION_TIMEOUT) {
        if (serial_port->available()) {
            String response = serial_port->readString();
            if (response.indexOf("comok") >= 0) {
                return true;
            }
        }
        delay(10);
    }
    
    return false;
}

// ===============================================================================
// GESTIONE PAGINE
// ===============================================================================

bool NextionDisplay::setPage(NextionPage page) {
    snprintf(command_buffer, NEXTION_BUFFER_SIZE, "page %d", (int)page);
    
    if (sendCommand(command_buffer)) {
        current_page = page;
        Serial.print(F("Nextion: Cambio pagina -> "));
        Serial.println((int)page);
        return true;
    }
    
    return false;
}

NextionPage NextionDisplay::getCurrentPage() {
    return current_page;
}

bool NextionDisplay::refreshCurrentPage() {
    return setPage(current_page);
}

// ===============================================================================
// AGGIORNAMENTO COMPONENTI
// ===============================================================================

bool NextionDisplay::setText(const char* component, const char* text) {
    snprintf(command_buffer, NEXTION_BUFFER_SIZE, "%s.txt=\"%s\"", component, text);
    return sendCommand(command_buffer);
}

bool NextionDisplay::setText(const char* component, float value, int decimals) {
    char temp_buffer[16];
    dtostrf(value, 0, decimals, temp_buffer);
    return setText(component, temp_buffer);
}

bool NextionDisplay::setText(const char* component, int value) {
    char temp_buffer[16];
    snprintf(temp_buffer, sizeof(temp_buffer), "%d", value);
    return setText(component, temp_buffer);
}

bool NextionDisplay::setTextColor(const char* component, uint16_t color) {
    snprintf(command_buffer, NEXTION_BUFFER_SIZE, "%s.pco=%u", component, color);
    return sendCommand(command_buffer);
}

bool NextionDisplay::setBackgroundColor(const char* component, uint16_t color) {
    snprintf(command_buffer, NEXTION_BUFFER_SIZE, "%s.bco=%u", component, color);
    return sendCommand(command_buffer);
}

bool NextionDisplay::setValue(const char* component, int value) {
    snprintf(command_buffer, NEXTION_BUFFER_SIZE, "%s.val=%d", component, value);
    return sendCommand(command_buffer);
}

bool NextionDisplay::getValue(const char* component, int* value) {
    snprintf(command_buffer, NEXTION_BUFFER_SIZE, "get %s.val", component);
    
    if (!sendCommand(command_buffer)) {
        return false;
    }
    
    // Legge risposta (formato: 0x71 + 4 bytes valore + 0xFF 0xFF 0xFF)
    if (waitForResponse()) {
        // Implementa parsing risposta numerica
        // TODO: Parsing completo risposta Nextion
        return true;
    }
    
    return false;
}

bool NextionDisplay::setVisible(const char* component, bool visible) {
    snprintf(command_buffer, NEXTION_BUFFER_SIZE, "vis %s,%d", component, visible ? 1 : 0);
    return sendCommand(command_buffer);
}

// ===============================================================================
// AGGIORNAMENTO DASHBOARD
// ===============================================================================

bool NextionDisplay::updateSensorData(float temp_int, float hum_int, bool int_valid,
                                     float temp_ext, float hum_ext, bool ext_valid) {
    bool success = true;
    
    // Aggiorna sensore interno
    if (int_valid) {
        success &= setText("t3", temp_int, 1);
        success &= setText("t4", hum_int, 1);
        success &= setTextColor("t3", NEXTION_GREEN);
        success &= setTextColor("t4", NEXTION_GREEN);
    } else {
        success &= setText("t3", "ERRORE");
        success &= setText("t4", "ERRORE");
        success &= setTextColor("t3", NEXTION_RED);
        success &= setTextColor("t4", NEXTION_RED);
    }
    
    // Aggiorna sensore esterno
    if (ext_valid) {
        success &= setText("t5", temp_ext, 1);
        success &= setText("t6", hum_ext, 1);
        success &= setTextColor("t5", NEXTION_CYAN);
        success &= setTextColor("t6", NEXTION_CYAN);
    } else {
        success &= setText("t5", "NON DISP");
        success &= setText("t6", "NON DISP");
        success &= setTextColor("t5", NEXTION_YELLOW);
        success &= setTextColor("t6", NEXTION_YELLOW);
    }
    
    return success;
}

bool NextionDisplay::updateActuatorStatus(bool frigorifero, bool riscaldatore, 
                                         bool deumidificatore, bool umidificatore,
                                         bool ventola_in, bool ventola_out) {
    bool success = true;
    
    // Aggiorna colori cerchi attuatori (c0-c5)
    success &= setBackgroundColor("c0", frigorifero ? NEXTION_BLUE : NEXTION_GRAY);
    success &= setBackgroundColor("c1", riscaldatore ? NEXTION_RED : NEXTION_GRAY);
    success &= setBackgroundColor("c2", deumidificatore ? NEXTION_ORANGE : NEXTION_GRAY);
    success &= setBackgroundColor("c3", umidificatore ? NEXTION_CYAN : NEXTION_GRAY);
    success &= setBackgroundColor("c4", ventola_in ? NEXTION_GREEN : NEXTION_GRAY);
    success &= setBackgroundColor("c5", ventola_out ? NEXTION_GREEN : NEXTION_GRAY);
    
    return success;
}

bool NextionDisplay::updateDemoMode(bool demo_active) {
    if (demo_active) {
        setText("t2", "MODALITA' DEMO");
        setTextColor("t2", NEXTION_YELLOW);
    } else {
        setText("t2", "Sistema Stagionatura Salumi");
        setTextColor("t2", NEXTION_WHITE);
    }
    
    return true;
}

// ===============================================================================
// FUNZIONI DASHBOARD PROFESSIONALE
// ===============================================================================

bool NextionDisplay::updateDashboardProfessional(float temp_attuale, float temp_setpoint, 
                                                  float umidita_attuale, float temp_min, float temp_max,
                                                  float umid_min, float umid_max, float progress_value,
                                                  const char* nome_programma, int fase_corrente, int fasi_totali,
                                                  const char* tempo_rimanente, bool programma_attivo) {
    bool success = true;
    
    // === AGGIORNA DISPLAY PRINCIPALI COMPATTI (NUOVO LAYOUT) ===
    // Valori principali LCD-style  
    success &= updateMainValues(temp_attuale, umidita_attuale, temp_setpoint);
    
    // Range values (layout compatto)
    success &= updateRangeValues(temp_min, temp_max, umid_min, umid_max);
    
    // === GESTIONE PROGRAMMA VS MANUALE ===
    if (programma_attivo) {
        // ==========================================
        // MODALITÀ PROGRAMMA AUTOMATICO
        // ==========================================
        
        // Nome e fase programma
        success &= setText("t60", nome_programma);  // Nome programma principale
        char fase_text[32];
        snprintf(fase_text, sizeof(fase_text), "FASE %d/%d", fase_corrente, fasi_totali);
        success &= setText("t61", fase_text);  // Fase corrente
        
        // Progress bar e percentuale
        success &= setValue("j0", (int)(progress_value));  // Progress bar 0-100
        char progress_text[8];
        snprintf(progress_text, sizeof(progress_text), "%d%%", (int)progress_value);
        success &= setText("t62", progress_text);  // Percentuale
        
        // Tempo rimanente
        success &= setText("t63", tempo_rimanente);
        
        // Indicatore modalità
        success &= setText("t64", "🤖 AUTOMATICO");
        success &= setTextColor("t64", NEXTION_GREEN);
        
        // Mostra progress bar
        success &= setVisible("j0", true);
        
        // Disabilita controlli manuali
        success &= enableRangeControls(false);  // Range non toccabili
        
    } else {
        // ==========================================
        // MODALITÀ CONTROLLO MANUALE
        // ==========================================
        
        // Testi modalità manuale
        success &= setText("t60", "🎛️ CONTROLLO MANUALE");
        success &= setText("t61", "👤 Controllo Utente");
        success &= setText("t62", "");  // Nasconde percentuale
        success &= setText("t63", "⚙️ Tap PROGRAMMI per avviare automatico");
        
        // Indicatore modalità
        success &= setText("t64", "🎛️ MANUALE");
        success &= setTextColor("t64", NEXTION_ORANGE);
        
        // Nasconde progress bar
        success &= setValue("j0", 0);
        success &= setVisible("j0", false);
        
        // Abilita controlli manuali
        success &= enableRangeControls(true);   // Range toccabili
    }
    
    return success;
}

bool NextionDisplay::updateActuatorsVisual(bool frigorifero, bool riscaldatore, 
                                          bool deumidificatore, bool umidificatore,
                                          bool ventola1, bool ventola2) {
    return updateActuatorsVisualMode(frigorifero, riscaldatore, deumidificatore, 
                                   umidificatore, ventola1, ventola2, false);
}

bool NextionDisplay::updateActuatorsVisualMode(bool frigorifero, bool riscaldatore, 
                                              bool deumidificatore, bool umidificatore,
                                              bool ventola1, bool ventola2, bool manual_mode) {
    bool success = true;
    
    // Colori RGB565: ON = colore specifico, OFF = grigio scuro
    uint16_t color_off = 0x4208;     // Grigio scuro #404040
    uint16_t color_blue = 0x4D9F;    // Blu freddo #2196F3  
    uint16_t color_red = 0xF800;     // Rosso caldo #F44336
    uint16_t color_orange = 0xFD20;  // Arancione #FF9800
    uint16_t color_green = 0x07E0;   // Verde #4CAF50
    uint16_t color_manual_border = 0xFD20;  // Arancione per bordo modalità manuale
    
    // Aggiorna colori di sfondo
    success &= setBackgroundColor("c0", frigorifero ? color_blue : color_off);
    success &= setBackgroundColor("c1", riscaldatore ? color_red : color_off);       
    success &= setBackgroundColor("c2", deumidificatore ? color_orange : color_off);    
    success &= setBackgroundColor("c3", umidificatore ? color_green : color_off);      
    success &= setBackgroundColor("c4", ventola1 ? color_green : color_off);          
    success &= setBackgroundColor("c5", ventola2 ? color_green : color_off);          
    
    if (manual_mode) {
        // MODALITÀ MANUALE: Aggiungi bordi arancioni per indicare interattività
        success &= sendCommand("c0.bco2=" + String(color_manual_border));  // Bordo arancione
        success &= sendCommand("c1.bco2=" + String(color_manual_border));
        success &= sendCommand("c2.bco2=" + String(color_manual_border));
        success &= sendCommand("c3.bco2=" + String(color_manual_border));
        success &= sendCommand("c4.bco2=" + String(color_manual_border));
        success &= sendCommand("c5.bco2=" + String(color_manual_border));
        
        // Abilita eventi touch sui cerchi
        success &= sendCommand("c0.touch=1");
        success &= sendCommand("c1.touch=1");
        success &= sendCommand("c2.touch=1");
        success &= sendCommand("c3.touch=1");
        success &= sendCommand("c4.touch=1");
        success &= sendCommand("c5.touch=1");
    } else {
        // MODALITÀ AUTOMATICA: Rimuovi bordi e disabilita touch
        success &= sendCommand("c0.bco2=" + String(color_off));  // Nessun bordo
        success &= sendCommand("c1.bco2=" + String(color_off));
        success &= sendCommand("c2.bco2=" + String(color_off));
        success &= sendCommand("c3.bco2=" + String(color_off));
        success &= sendCommand("c4.bco2=" + String(color_off));
        success &= sendCommand("c5.bco2=" + String(color_off));
        
        // Disabilita eventi touch sui cerchi
        success &= sendCommand("c0.touch=0");
        success &= sendCommand("c1.touch=0");
        success &= sendCommand("c2.touch=0");
        success &= sendCommand("c3.touch=0");
        success &= sendCommand("c4.touch=0");
        success &= sendCommand("c5.touch=0");
    }
    
    return success;
}

// ===============================================================================
// GESTIONE RANGE INTERATTIVI (MODALITÀ MANUALE)
// ===============================================================================

bool NextionDisplay::updateRangeSelection(RangeSelected selected_range) {
    bool success = true;
    
    // Colori per bordi e indicatori
    uint16_t color_invisible = 0x0000;      // Nero (invisibile su sfondo scuro)
    uint16_t color_temp_selected = 0x4D9F;  // Blu per temperatura
    uint16_t color_umid_selected = 0x07E0;  // Verde per umidità
    uint16_t color_gray = 0x8410;           // Grigio per indicatori non selezionati
    
    // === RESET TUTTI I BORDI A INVISIBILE ===
    success &= sendCommand("b10.bco=" + String(color_invisible));  // Tmin border
    success &= sendCommand("b11.bco=" + String(color_invisible));  // Tmax border
    success &= sendCommand("b12.bco=" + String(color_invisible));  // Umin border
    success &= sendCommand("b13.bco=" + String(color_invisible));  // Umax border
    
    // === RESET TUTTI GLI INDICATORI A GRIGIO ===
    success &= setBackgroundColor("c10", color_gray);  // Indicatore Tmin
    success &= setBackgroundColor("c11", color_gray);  // Indicatore Tmax
    success &= setBackgroundColor("c12", color_gray);  // Indicatore Umin
    success &= setBackgroundColor("c13", color_gray);  // Indicatore Umax
    
    // === EVIDENZIA IL RANGE SELEZIONATO ===
    switch (selected_range) {
        case RANGE_TMIN:
            success &= sendCommand("b10.bco=" + String(color_temp_selected));  // Bordo blu
            success &= sendCommand("b10.bco2=" + String(color_temp_selected)); // Bordo interno
            success &= setBackgroundColor("c10", color_temp_selected);         // Pallino blu
            break;
            
        case RANGE_TMAX:
            success &= sendCommand("b11.bco=" + String(color_temp_selected));  // Bordo blu
            success &= sendCommand("b11.bco2=" + String(color_temp_selected)); // Bordo interno
            success &= setBackgroundColor("c11", color_temp_selected);         // Pallino blu
            break;
            
        case RANGE_UMIN:
            success &= sendCommand("b12.bco=" + String(color_umid_selected));  // Bordo verde
            success &= sendCommand("b12.bco2=" + String(color_umid_selected)); // Bordo interno
            success &= setBackgroundColor("c12", color_umid_selected);         // Pallino verde
            break;
            
        case RANGE_UMAX:
            success &= sendCommand("b13.bco=" + String(color_umid_selected));  // Bordo verde
            success &= sendCommand("b13.bco2=" + String(color_umid_selected)); // Bordo interno
            success &= setBackgroundColor("c13", color_umid_selected);         // Pallino verde
            break;
            
        case RANGE_NONE:
        default:
            // Tutti invisibili e grigi, già fatto sopra
            break;
    }
    
    return success;
}

bool NextionDisplay::updateRangeValues(float temp_min, float temp_max, 
                                      float umid_min, float umid_max) {
    bool success = true;
    
    // Aggiorna i valori sui pulsanti NUOVO LAYOUT
    success &= setText("b10", String((int)temp_min));  // Tmin button
    success &= setText("b11", String((int)temp_max));  // Tmax button
    success &= setText("b12", String((int)umid_min));  // Umin button
    success &= setText("b13", String((int)umid_max));  // Umax button
    
    return success;
}

bool NextionDisplay::enableRangeControls(bool enable) {
    bool success = true;
    
    if (enable) {
        // === MODALITÀ MANUALE: Abilita touch sui range boxes ===
        success &= sendCommand("b10.touch=1");  // Tmin
        success &= sendCommand("b11.touch=1");  // Tmax
        success &= sendCommand("b12.touch=1");  // Umin
        success &= sendCommand("b13.touch=1");  // Umax
        
        // Mantieni bordi invisibili inizialmente (saranno attivati solo quando selezionati)
        success &= sendCommand("b10.bco=" + String(0x0000));  // Invisibile
        success &= sendCommand("b11.bco=" + String(0x0000));
        success &= sendCommand("b12.bco=" + String(0x0000));
        success &= sendCommand("b13.bco=" + String(0x0000));
        
        // Indicatori grigi (disponibili ma non selezionati)
        success &= setBackgroundColor("c10", 0x8410);  // Grigio
        success &= setBackgroundColor("c11", 0x8410);
        success &= setBackgroundColor("c12", 0x8410);
        success &= setBackgroundColor("c13", 0x8410);
        
    } else {
        // === MODALITÀ PROGRAMMA: Disabilita touch ===
        success &= sendCommand("b10.touch=0");  // Tmin
        success &= sendCommand("b11.touch=0");  // Tmax
        success &= sendCommand("b12.touch=0");  // Umin
        success &= sendCommand("b13.touch=0");  // Umax
        
        // Nascondi completamente bordi e indicatori
        success &= sendCommand("b10.bco=" + String(0x0000));  // Bordi invisibili
        success &= sendCommand("b11.bco=" + String(0x0000));
        success &= sendCommand("b12.bco=" + String(0x0000));
        success &= sendCommand("b13.bco=" + String(0x0000));
        
        // Indicatori invisibili (stesso colore dello sfondo)
        success &= setBackgroundColor("c10", 0x0000);  // Nero invisibile
        success &= setBackgroundColor("c11", 0x0000);
        success &= setBackgroundColor("c12", 0x0000);
        success &= setBackgroundColor("c13", 0x0000);
    }
    
    return success;
}

// ===============================================================================
// AGGIORNAMENTO VALORI PRINCIPALI (NUOVO LAYOUT COMPATTO)
// ===============================================================================

bool NextionDisplay::updateMainValues(float temperatura, float umidita, float setpoint) {
    bool success = true;
    
    // === AGGIORNA VALORI PRINCIPALI GRANDI (stile LCD) ===
    // Temperatura attuale (giallo su viola scuro)
    char temp_str[8];
    snprintf(temp_str, sizeof(temp_str), "%.1f", temperatura);
    success &= setText("t21", temp_str);  // Display temperatura principale
    
    // Umidità attuale (verde su viola scuro)
    char umid_str[8];
    snprintf(umid_str, sizeof(umid_str), "%.1f", umidita);
    success &= setText("t31", umid_str);  // Display umidità principale
    
    // Setpoint temperatura (bianco su cyan)
    char setpoint_str[8];
    snprintf(setpoint_str, sizeof(setpoint_str), "%.1f", setpoint);
    success &= setText("t25", setpoint_str);  // Display setpoint centrale
    
    return success;
}

bool NextionDisplay::updateTimestamp(unsigned long seconds_ago) {
    char timestamp_text[32];
    snprintf(timestamp_text, sizeof(timestamp_text), "Agg: %lu s fa", seconds_ago);
    return setText("t7", timestamp_text);
}

// ===============================================================================
// GESTIONE EVENTI
// ===============================================================================

bool NextionDisplay::pollEvents() {
    if (!serial_port->available()) {
        return false;
    }
    
    // Legge dati seriali
    int bytes_available = serial_port->available();
    if (bytes_available >= 7) { // Evento touch completo
        uint8_t event_data[7];
        
        for (int i = 0; i < 7; i++) {
            event_data[i] = serial_port->read();
        }
        
        // Verifica terminatori
        if (event_data[4] == 0xFF && event_data[5] == 0xFF && event_data[6] == 0xFF) {
            NextionEvent event;
            event.page_id = event_data[1];
            event.component_id = event_data[2];
            event.event_type = event_data[3];
            event.is_valid = true;
            
            last_event = event;
            new_event_available = true;
            
            // Debug evento
            Serial.print(F("Nextion Event - Page: "));
            Serial.print(event.page_id);
            Serial.print(F(", Component: "));
            Serial.print(event.component_id);
            Serial.print(F(", Type: "));
            Serial.println(event.event_type);
            
            return true;
        }
    }
    
    // Pulisce buffer se dati non validi
    flushSerial();
    return false;
}

NextionEvent NextionDisplay::getLastEvent() {
    new_event_available = false;
    return last_event;
}

bool NextionDisplay::hasNewEvent() {
    return new_event_available;
}

// ===============================================================================
// COMANDI SPECIALI
// ===============================================================================

bool NextionDisplay::setBrightness(uint8_t brightness) {
    // Nextion brightness range: 0-100
    brightness = constrain(brightness, 0, 100);
    snprintf(command_buffer, NEXTION_BUFFER_SIZE, "dim=%d", brightness);
    return sendCommand(command_buffer);
}

bool NextionDisplay::setBacklight(bool enabled) {
    snprintf(command_buffer, NEXTION_BUFFER_SIZE, "sleep=%d", enabled ? 0 : 1);
    return sendCommand(command_buffer);
}

bool NextionDisplay::playSound(int sound_id, int volume) {
    // TODO: Implementa riproduzione suoni se supportata
    return true;
}

bool NextionDisplay::showMessage(const char* title, const char* message) {
    // Implementa finestra di dialogo custom
    // TODO: Crea pagina popup per messaggi
    return true;
}

// ===============================================================================
// UTILITÀ E DEBUG
// ===============================================================================

bool NextionDisplay::sendCommand(const char* command) {
    if (!is_initialized) {
        return false;
    }
    
    // Aggiunge terminatori Nextion
    serial_port->print(command);
    serial_port->print(NEXTION_END_CMD);
    
    last_command_time = millis();
    
    // Debug comando inviato
    Serial.print(F("Nextion CMD: "));
    Serial.println(command);
    
    return waitForResponse();
}

bool NextionDisplay::sendRawCommand(const char* command) {
    serial_port->print(command);
    return true;
}

int NextionDisplay::getLastError() {
    return last_error_code;
}

void NextionDisplay::clearBuffer() {
    flushSerial();
    memset(command_buffer, 0, NEXTION_BUFFER_SIZE);
    memset(receive_buffer, 0, NEXTION_BUFFER_SIZE);
}

void NextionDisplay::printDebugInfo() {
    Serial.println(F("=== NEXTION DEBUG INFO ==="));
    Serial.print(F("Inizializzato: "));
    Serial.println(is_initialized ? F("SI") : F("NO"));
    Serial.print(F("Pagina corrente: "));
    Serial.println((int)current_page);
    Serial.print(F("Ultimo errore: "));
    Serial.println(last_error_code);
    Serial.print(F("Ultimo comando: "));
    Serial.print((millis() - last_command_time) / 1000);
    Serial.println(F(" s fa"));
    Serial.println(F("========================"));
}

// ===============================================================================
// FUNZIONI INTERNE
// ===============================================================================

bool NextionDisplay::waitForResponse(unsigned long timeout) {
    unsigned long start = millis();
    
    while (millis() - start < timeout) {
        if (serial_port->available()) {
            // Legge risposta
            delay(10); // Attende ricezione completa
            
            int available = serial_port->available();
            if (available > 0) {
                // Cerca terminatori 0xFF 0xFF 0xFF
                bool found_terminator = false;
                int terminator_count = 0;
                
                while (serial_port->available() && !found_terminator) {
                    uint8_t byte = serial_port->read();
                    
                    if (byte == 0xFF) {
                        terminator_count++;
                        if (terminator_count >= 3) {
                            found_terminator = true;
                        }
                    } else {
                        terminator_count = 0;
                    }
                }
                
                return found_terminator;
            }
        }
        
        delay(1);
    }
    
    // Timeout - comando fallito
    last_error_code = -1;
    return false;
}

bool NextionDisplay::parseEvent(NextionEvent* event) {
    // Implementazione parsing eventi complessi
    // Già gestito in pollEvents() per eventi base
    return true;
}

void NextionDisplay::flushSerial() {
    while (serial_port->available()) {
        serial_port->read();
        delay(1);
    }
}

bool NextionDisplay::validateResponse() {
    // TODO: Validazione risposta in base al tipo di comando
    return true;
}

void NextionDisplay::logError(int error_code, const char* message) {
    last_error_code = error_code;
    Serial.print(F("Nextion Error "));
    Serial.print(error_code);
    Serial.print(F(": "));
    Serial.println(message);
}
