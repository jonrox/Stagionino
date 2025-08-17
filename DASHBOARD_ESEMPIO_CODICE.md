# 🎛️ DASHBOARD PROFESSIONALE - Esempio Implementazione

## 📋 Come Utilizzare il Nuovo Dashboard

Ecco un esempio pratico di come utilizzare le nuove funzioni del dashboard professionale:

## 🔧 **Codice Arduino di Esempio**

### **Setup Iniziale:**
```cpp
#include "nextion_protocol.h"

// Variabili sistema
float temperatura_attuale = 18.6;
float temperatura_setpoint = 15.0;
float umidita_attuale = 75.2;
float temp_min = 16.0, temp_max = 19.0;
float umid_min = 50.0, umid_max = 75.0;

// Programma
bool programma_attivo = true;
String nome_programma = "SALAME TRADIZIONALE";
int fase_corrente = 2;
int fasi_totali = 4;
float progress_percentuale = 60.0;
String tempo_rimanente = "12g 4h rimanenti";

// Stati attuatori
bool frigorifero_on = true;
bool riscaldatore_on = false;
bool deumidificatore_on = false;
bool umidificatore_on = true;
bool ventola1_on = true;
bool ventola2_on = false;

void setup() {
    // Inizializza Nextion
    nextion.begin(9600);
    nextion.setPage(PAGE_DASHBOARD);
    
    // Prima configurazione dashboard
    updateDashboardComplete();
}
```

### **Funzione Aggiornamento Completo:**
```cpp
void updateDashboardComplete() {
    // Aggiorna tutti i controlli ambientali e programma
    nextion.updateDashboardProfessional(
        temperatura_attuale,    // 18.6
        temperatura_setpoint,   // 15.0
        umidita_attuale,       // 75.2
        temp_min, temp_max,    // 16.0, 19.0
        umid_min, umid_max,    // 50.0, 75.0
        progress_percentuale,   // 60.0
        nome_programma.c_str(),// "SALAME TRADIZIONALE"
        fase_corrente,         // 2
        fasi_totali,          // 4
        tempo_rimanente.c_str(), // "12g 4h rimanenti"
        programma_attivo       // true
    );
    
    // Aggiorna visualizzazione attuatori
    nextion.updateActuatorsVisual(
        frigorifero_on,        // true  -> 🔵 BLU
        riscaldatore_on,       // false -> ⚫ GRIGIO
        deumidificatore_on,    // false -> ⚫ GRIGIO  
        umidificatore_on,      // true  -> 🟢 VERDE
        ventola1_on,          // true  -> 🟢 VERDE
        ventola2_on           // false -> ⚫ GRIGIO
    );
}
```

### **Gestione Eventi Touch:**
```cpp
void handleNextionEvents() {
    if (nextion.pollEvents()) {
        NextionEvent event = nextion.getLastEvent();
        
        if (event.page_id == PAGE_DASHBOARD) {
            switch (event.component_id) {
                
                case DASHBOARD_BTN_TEMP_UP:
                    // Pulsante SU temperatura [🔺]
                    temperatura_setpoint += 0.5;
                    if (temperatura_setpoint > 25.0) temperatura_setpoint = 25.0;
                    
                    // Aggiorna solo il setpoint
                    nextion.setText("t1", String((int)temperatura_setpoint));
                    
                    Serial.print("Setpoint aumentato a: ");
                    Serial.println(temperatura_setpoint);
                    break;
                    
                case DASHBOARD_BTN_TEMP_DOWN:
                    // Pulsante GIÙ temperatura [🔻]
                    temperatura_setpoint -= 0.5;
                    if (temperatura_setpoint < 10.0) temperatura_setpoint = 10.0;
                    
                    // Aggiorna solo il setpoint
                    nextion.setText("t1", String((int)temperatura_setpoint));
                    
                    Serial.print("Setpoint diminuito a: ");
                    Serial.println(temperatura_setpoint);
                    break;
                    
                case DASHBOARD_BTN_SENSORS:
                    // Vai a pagina sensori
                    nextion.setPage(PAGE_SENSORS);
                    break;
                    
                case DASHBOARD_BTN_PROGRAMS:
                    // Vai a pagina programmi
                    nextion.setPage(PAGE_PROGRAMS);
                    break;
                    
                case DASHBOARD_BTN_SETTINGS:
                    // Vai a pagina impostazioni
                    nextion.setPage(PAGE_SETTINGS);
                    break;
            }
        }
    }
}
```

### **Loop Principale:**
```cpp
void loop() {
    static unsigned long last_update = 0;
    static unsigned long last_sensor_read = 0;
    
    // Gestione eventi touch
    handleNextionEvents();
    
    // Lettura sensori ogni 2 secondi
    if (millis() - last_sensor_read >= 2000) {
        readSensors();  // Legge temperatura_attuale, umidita_attuale
        last_sensor_read = millis();
    }
    
    // Aggiornamento display ogni 1 secondo
    if (millis() - last_update >= 1000) {
        
        // Aggiorna solo valori che cambiano
        nextion.setText("t0", String(temperatura_attuale, 1));
        nextion.setText("t2", String(umidita_attuale, 1));
        
        // Aggiorna attuatori se cambiati
        updateActuatorsIfChanged();
        
        // Aggiorna progress programma se attivo
        if (programma_attivo) {
            updateProgramProgress();
        }
        
        last_update = millis();
    }
    
    delay(50);
}
```

### **Funzioni di Supporto:**
```cpp
void updateActuatorsIfChanged() {
    static bool last_states[6] = {false, false, false, false, false, false};
    bool current_states[6] = {frigorifero_on, riscaldatore_on, deumidificatore_on, 
                             umidificatore_on, ventola1_on, ventola2_on};
    
    bool changed = false;
    for (int i = 0; i < 6; i++) {
        if (last_states[i] != current_states[i]) {
            changed = true;
            last_states[i] = current_states[i];
        }
    }
    
    if (changed) {
        nextion.updateActuatorsVisual(
            frigorifero_on, riscaldatore_on, deumidificatore_on,
            umidificatore_on, ventola1_on, ventola2_on
        );
        
        Serial.println("Attuatori aggiornati sul display");
    }
}

void updateProgramProgress() {
    // Calcola progress in base al tempo trascorso
    static unsigned long program_start_time = millis();
    unsigned long elapsed = millis() - program_start_time;
    
    // Esempio: programma dura 21 giorni = 1814400000 ms
    float total_duration = 21 * 24 * 60 * 60 * 1000UL;
    progress_percentuale = (elapsed / total_duration) * 100.0;
    
    if (progress_percentuale > 100.0) progress_percentuale = 100.0;
    
    // Aggiorna progress bar
    nextion.setValue("j0", (int)progress_percentuale);
    
    // Calcola tempo rimanente
    if (progress_percentuale < 100.0) {
        unsigned long remaining_ms = total_duration - elapsed;
        int days = remaining_ms / (24 * 60 * 60 * 1000UL);
        int hours = (remaining_ms % (24 * 60 * 60 * 1000UL)) / (60 * 60 * 1000UL);
        
        char time_text[32];
        snprintf(time_text, sizeof(time_text), "%dg %dh rimanenti", days, hours);
        tempo_rimanente = String(time_text);
        
        nextion.setText("t12", tempo_rimanente.c_str());
    }
}

void startProgram(String nome, int fasi) {
    programma_attivo = true;
    nome_programma = nome;
    fase_corrente = 1;
    fasi_totali = fasi;
    progress_percentuale = 0.0;
    
    // Aggiorna dashboard completo
    updateDashboardComplete();
    
    Serial.println("Programma '" + nome + "' avviato!");
}

void stopProgram() {
    programma_attivo = false;
    nome_programma = "";
    
    // Aggiorna dashboard per mostrare controllo manuale
    updateDashboardComplete();
    
    Serial.println("Programma fermato - modalità manuale");
}
```

## 🎯 **Risultato Visivo**

Con questo codice ottieni:

### **🌡️ Controlli Ambientali Dinamici:**
- **Temperatura**: 18.6°C (aggiornata in tempo reale)
- **Setpoint**: 15°C (modificabile con [🔺][🔻])
- **Umidità**: 75.2% (aggiornata in tempo reale)
- **Range**: Visualizzazione min/max configurabili

### **📊 Programma Intelligente:**
- **Con programma**: Nome, fase, progress bar, tempo rimanente
- **Senza programma**: "CONTROLLO MANUALE" + suggerimento

### **🎛️ Attuatori Visivi:**
- **ON**: Colori vivaci (🔵🔴🟠🟢)
- **OFF**: Grigio uniforme (⚫)
- **Feedback istantaneo** su ogni cambio stato

### **📱 Navigazione Intuitiva:**
- **3 pulsanti principali** sempre visibili
- **Touch setpoint** per regolazioni rapide
- **Feedback visivo** su ogni interazione

Questo crea un'interfaccia **professionale come un prodotto industriale** ma **semplice come uno smartphone**! 🚀
