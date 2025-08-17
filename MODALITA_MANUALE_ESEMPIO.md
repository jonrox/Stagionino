# 🎛️ MODALITÀ MANUALE - Controllo Diretto Attuatori

## 🎯 **Comportamento Dashboard in Modalità Manuale**

Quando **nessun programma è attivo**, la dashboard si trasforma in un **pannello di controllo manuale** che permette all'utente di controllare direttamente tutti gli attuatori.

---

## 🖥️ **Differenze Visive: Programma vs Manuale**

### **📊 CON PROGRAMMA ATTIVO:**
```
┌─ 🎯 PROGRAMMA ATTIVO ───────────────────────────────────┐
│                                                         │
│ SALAME TRADIZIONALE           Status: ▶️ RUNNING       │
│ FASE 2/4 | 60% ████████████▒▒▒▒▒▒▒▒ | 12g 4h left    │
│                                                         │
│ Target: 🌡️15°C(±1) 💧75%(±5) | [⏸️PAUSA] [⏹️STOP]   │
└─────────────────────────────────────────────────────────┘

ATTUATORI: ❄️🔵 🔥⚫ 💨⚫ 💧🟢 🌀🟢 🌀⚫ (Touch: DISABILITATO)
```

### **🎛️ SENZA PROGRAMMA (MANUALE):**
```
┌─ 🎛️ CONTROLLO MANUALE ─────────────────────────────────┐
│                                                         │
│ 👤 Utente controlla tutti gli attuatori                │
│ ⚙️ Tap PROGRAMMI per avviare automatico                │
│ 🔄 Override: Attivo                                    │
│                                                         │
└─────────────────────────────────────────────────────────┘

ATTUATORI: ❄️🔵⭕ 🔥⚫⭕ 💨⚫⭕ 💧🟢⭕ 🌀🟢⭕ 🌀⚫⭕ (Touch: ABILITATO)
          (bordi arancioni = tap per toggle)
```

---

## 🛠️ **Implementazione Codice Arduino**

### **Aggiornamento Dashboard Completo:**
```cpp
void updateDashboardCompleteModeAware() {
    bool programma_attivo = system_state.current_program_active;
    bool manual_mode = !programma_attivo;
    
    // Aggiorna tutti i controlli ambientali e programma
    nextion.updateDashboardProfessional(
        temperatura_attuale,    // 18.6
        temperatura_setpoint,   // 15.0
        umidita_attuale,       // 75.2
        temp_min, temp_max,    // 16.0, 19.0
        umid_min, umid_max,    // 50.0, 75.0
        progress_percentuale,   // 60.0 o 0.0 se manuale
        nome_programma.c_str(),// "SALAME TRADIZIONALE" o ""
        fase_corrente,         // 2 o 0
        fasi_totali,          // 4 o 0
        tempo_rimanente.c_str(), // "12g 4h rimanenti" o ""
        programma_attivo       // true/false
    );
    
    // Aggiorna visualizzazione attuatori con modalità
    nextion.updateActuatorsVisualMode(
        frigorifero_on,        
        riscaldatore_on,       
        deumidificatore_on,    
        umidificatore_on,      
        ventola1_on,          
        ventola2_on,
        manual_mode            // true = bordi arancioni + touch abilitato
    );
    
    Serial.println(manual_mode ? 
        "Dashboard: MODALITÀ MANUALE attiva" : 
        "Dashboard: MODALITÀ PROGRAMMA attiva");
}
```

### **Gestione Touch Eventi in Modalità Manuale:**
```cpp
void handleNextionEvents() {
    if (nextion.pollEvents()) {
        NextionEvent event = nextion.getLastEvent();
        
        if (event.page_id == PAGE_DASHBOARD) {
            
            // Controlli standard (sempre attivi)
            if (event.component_id == DASHBOARD_BTN_TEMP_UP) {
                temperatura_setpoint += 0.5;
                nextion.setText("t1", String((int)temperatura_setpoint));
                Serial.println("Setpoint aumentato: " + String(temperatura_setpoint));
                return;
            }
            
            // Touch attuatori (solo in modalità manuale)
            if (!system_state.current_program_active) {
                switch (event.component_id) {
                    
                    case DASHBOARD_CIRCLE_FRIGO:
                        frigorifero_on = !frigorifero_on;
                        digitalWrite(RELAY_FRIGORIFERO, frigorifero_on ? LOW : HIGH);
                        
                        // Feedback visivo immediato
                        nextion.updateActuatorsVisualMode(
                            frigorifero_on, riscaldatore_on, deumidificatore_on,
                            umidificatore_on, ventola1_on, ventola2_on, true
                        );
                        
                        Serial.println("MANUALE: Frigorifero " + 
                            String(frigorifero_on ? "ACCESO" : "SPENTO"));
                        break;
                        
                    case DASHBOARD_CIRCLE_RISC:
                        riscaldatore_on = !riscaldatore_on;
                        digitalWrite(RELAY_RISCALDATORE, riscaldatore_on ? LOW : HIGH);
                        
                        nextion.updateActuatorsVisualMode(
                            frigorifero_on, riscaldatore_on, deumidificatore_on,
                            umidificatore_on, ventola1_on, ventola2_on, true
                        );
                        
                        Serial.println("MANUALE: Riscaldatore " + 
                            String(riscaldatore_on ? "ACCESO" : "SPENTO"));
                        break;
                        
                    case DASHBOARD_CIRCLE_DEUM:
                        deumidificatore_on = !deumidificatore_on;
                        digitalWrite(RELAY_DEUMIDIFICATORE, deumidificatore_on ? LOW : HIGH);
                        
                        nextion.updateActuatorsVisualMode(
                            frigorifero_on, riscaldatore_on, deumidificatore_on,
                            umidificatore_on, ventola1_on, ventola2_on, true
                        );
                        
                        Serial.println("MANUALE: Deumidificatore " + 
                            String(deumidificatore_on ? "ACCESO" : "SPENTO"));
                        break;
                        
                    case DASHBOARD_CIRCLE_UMID:
                        umidificatore_on = !umidificatore_on;
                        digitalWrite(RELAY_UMIDIFICATORE, umidificatore_on ? LOW : HIGH);
                        
                        nextion.updateActuatorsVisualMode(
                            frigorifero_on, riscaldatore_on, deumidificatore_on,
                            umidificatore_on, ventola1_on, ventola2_on, true
                        );
                        
                        Serial.println("MANUALE: Umidificatore " + 
                            String(umidificatore_on ? "ACCESO" : "SPENTO"));
                        break;
                        
                    case DASHBOARD_CIRCLE_VENT1:
                        ventola1_on = !ventola1_on;
                        digitalWrite(RELAY_VENTOLA_IN, ventola1_on ? LOW : HIGH);
                        
                        nextion.updateActuatorsVisualMode(
                            frigorifero_on, riscaldatore_on, deumidificatore_on,
                            umidificatore_on, ventola1_on, ventola2_on, true
                        );
                        
                        Serial.println("MANUALE: Ventola 1 " + 
                            String(ventola1_on ? "ACCESA" : "SPENTA"));
                        break;
                        
                    case DASHBOARD_CIRCLE_VENT2:
                        ventola2_on = !ventola2_on;
                        digitalWrite(RELAY_VENTOLA_OUT, ventola2_on ? LOW : HIGH);
                        
                        nextion.updateActuatorsVisualMode(
                            frigorifero_on, riscaldatore_on, deumidificatore_on,
                            umidificatore_on, ventola1_on, ventola2_on, true
                        );
                        
                        Serial.println("MANUALE: Ventola 2 " + 
                            String(ventola2_on ? "ACCESA" : "SPENTA"));
                        break;
                }
            } else {
                // In modalità programma, touch attuatori ignorato
                Serial.println("Touch attuatore ignorato: modalità programma attiva");
            }
        }
    }
}
```

### **Transizione Programma → Manuale:**
```cpp
void stopProgram() {
    system_state.current_program_active = false;
    nome_programma = "";
    fase_corrente = 0;
    fasi_totali = 0;
    progress_percentuale = 0.0;
    
    // Aggiorna dashboard per mostrare controllo manuale
    updateDashboardCompleteModeAware();
    
    Serial.println("=== MODALITÀ MANUALE ATTIVATA ===");
    Serial.println("- Touch attuatori: ABILITATO");
    Serial.println("- Controllo utente: ATTIVO");
    Serial.println("- Setpoint: Modificabile");
    Serial.println("=====================================");
}
```

### **Transizione Manuale → Programma:**
```cpp
void startProgram(String nome, int fasi) {
    system_state.current_program_active = true;
    nome_programma = nome;
    fase_corrente = 1;
    fasi_totali = fasi;
    progress_percentuale = 0.0;
    
    // Aggiorna dashboard per mostrare programma
    updateDashboardCompleteModeAware();
    
    Serial.println("=== MODALITÀ PROGRAMMA ATTIVATA ===");
    Serial.println("- Programma: " + nome);
    Serial.println("- Touch attuatori: DISABILITATO");
    Serial.println("- Controllo automatico: ATTIVO");
    Serial.println("===================================");
}
```

---

## 🎨 **Feedback Visivo in Modalità Manuale**

### **🔵 Indicatori Bordo Arancione:**
- **Bordi arancioni** attorno agli attuatori = "Touch abilitato"
- **Lampeggio leggero** per attirare attenzione
- **Animazione press** quando toccato

### **📱 Messaggi di Sistema:**
```cpp
void showManualModeHints() {
    // Mostra suggerimenti periodici
    static unsigned long last_hint = 0;
    
    if (millis() - last_hint >= 30000 && !system_state.current_program_active) {
        nextion.setText("t12", "💡 Tip: Tocca gli attuatori per controllarli");
        delay(3000);
        nextion.setText("t12", "⚙️ Tap PROGRAMMI per avviare automatico");
        last_hint = millis();
    }
}
```

### **🚦 Log Attività Manuale:**
```cpp
void logManualActivity(String attuatore, bool stato) {
    String action = stato ? "ACCESO" : "SPENTO";
    String log_message = "MANUALE: " + attuatore + " " + action + " da utente";
    
    Serial.println(log_message);
    
    // Opzionale: salva su SD per storico
    if (SD.begin(SD_CS)) {
        File logFile = SD.open("manual_log.txt", FILE_WRITE);
        if (logFile) {
            logFile.println(getTimestamp() + " - " + log_message);
            logFile.close();
        }
    }
}
```

---

## 🎯 **Vantaggi Modalità Manuale**

### **👤 Per l'Utente:**
✅ **Controllo immediato** su ogni attuatore  
✅ **Feedback visivo chiaro** su cosa è toccabile  
✅ **Setpoint modificabili** in tempo reale  
✅ **Passaggio programma** con un tap  

### **🛠️ Per il Sistema:**
✅ **Safety**: Controllo diretto senza algoritmi  
✅ **Test**: Verifica manuale ogni componente  
✅ **Debug**: Isolamento problemi hardware  
✅ **Flessibilità**: Situazioni non standard  

### **📊 Per lo Sviluppo:**
✅ **Testing**: Verifica funzionamento attuatori  
✅ **Calibrazione**: Setup iniziale sistema  
✅ **Manutenzione**: Diagnosi e riparazione  
✅ **Demo**: Presentazione funzionalità  

La modalità manuale trasforma il sistema in un **pannello di controllo interattivo professionale** mantenendo la stessa interfaccia elegante! 🎉
