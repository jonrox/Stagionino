# 🎛️ CONTROLLI RANGE INTERATTIVI - Modalità Manuale

## 🎯 **Funzionalità: Touch Range + Frecce**

In modalità manuale, l'utente può modificare direttamente i valori di range toccando i box **Tmin, Tmax, Umin, Umax** e usando le frecce [🔺][🔻] per regolare il valore selezionato.

---

## 🖥️ **Interfaccia Visiva**

### **Dashboard con Range Interattivi:**
```
┌─ CONTROLLI AMBIENTALI ───────────────────────────────────┐
│                                                           │
│  📊 18.6°C    🎯 15°C [🔺]    💧 75.2%                   │
│      Temp      Target [🔻]      Umid                     │
│                                                           │
│  ┌───────┐  ┌───────┐  ┌───────┐  ┌───────┐  ┌───────┐ │
│  │  16   │  │  19   │  │  📊   │  │ 6.1   │  │  50   │ │
│  │ Tmin  │  │ Tmax  │  │ LOG   │  │ Prog  │  │ Umin  │ │
│  │ 📦BLU │  │ 📦GRY │  │       │  │       │  │ 📦GRY │ │  
│  └───────┘  └───────┘  └───────┘  └───────┘  └───────┘ │
│               ↑                                ↑        │
│            SELECTED                        TOUCHABLE    │
└─────────────────────────────────────────────────────────┘

Stati Visivi:
📦BLU  = Temperatura selezionata (bordo blu)
📦VERDE = Umidità selezionata (bordo verde)  
📦GRY  = Non selezionato ma toccabile (bordo grigio)
📦OFF  = Non toccabile (modalità programma)
```

---

## 🎮 **Interazione Utente**

### **Workflow Controllo Range:**

1. **👆 TOCCA** un range box (Tmin, Tmax, Umin, Umax)
2. **🔵 BORDO** cambia colore per indicare selezione
3. **🔺🔻 FRECCE** modificano il valore selezionato
4. **💾 SALVA** automaticamente il nuovo valore
5. **👆 TOCCA** altro range per cambiare selezione

### **Esempio Sequenza:**
```
Step 1: Touch "16" (Tmin)
   → Bordo diventa BLU
   → Frecce controllano Tmin

Step 2: Press [🔺] 3 volte  
   → Tmin: 16 → 17 → 18 → 19

Step 3: Touch "50" (Umin)
   → Bordo Tmin torna GRIGIO
   → Bordo Umin diventa VERDE
   → Frecce ora controllano Umin

Step 4: Press [🔻] 2 volte
   → Umin: 50 → 49 → 48
```

---

## 🛠️ **Implementazione Arduino**

### **Variabili Globali:**
```cpp
// Range values
float temp_min = 16.0;
float temp_max = 19.0; 
float umid_min = 50.0;
float umid_max = 75.0;

// Selezione corrente
RangeSelected current_range_selected = RANGE_NONE;

// Modalità
bool manual_mode = false;
bool programma_attivo = false;
```

### **Setup Iniziale:**
```cpp
void setup() {
    // ... altri setup ...
    
    // Inizializza range controls
    nextion.updateRangeValues(temp_min, temp_max, umid_min, umid_max);
    nextion.enableRangeControls(false);  // Disabilitato finché non in modalità manuale
    nextion.updateRangeSelection(RANGE_NONE);
    
    Serial.println("Range controls inizializzati");
}
```

### **Aggiornamento Modalità:**
```cpp
void updateSystemMode() {
    manual_mode = !programma_attivo;
    
    // Abilita/disabilita controlli range in base alla modalità
    nextion.enableRangeControls(manual_mode);
    
    if (manual_mode) {
        Serial.println("MODALITÀ MANUALE: Range controls abilitati");
        // Reset selezione quando si entra in modalità manuale
        current_range_selected = RANGE_NONE;
        nextion.updateRangeSelection(RANGE_NONE);
    } else {
        Serial.println("MODALITÀ PROGRAMMA: Range controls disabilitati");
        current_range_selected = RANGE_NONE;
    }
}
```

### **Gestione Touch Eventi:**
```cpp
void handleNextionEvents() {
    if (nextion.pollEvents()) {
        NextionEvent event = nextion.getLastEvent();
        
        if (event.page_id == PAGE_DASHBOARD) {
            
            // === TOUCH RANGE BOXES (solo in modalità manuale) ===
            if (manual_mode) {
                switch (event.component_id) {
                    
                    case DASHBOARD_BTN_TMIN:
                        current_range_selected = RANGE_TMIN;
                        nextion.updateRangeSelection(RANGE_TMIN);
                        Serial.println("Range selezionto: Tmin (" + String(temp_min) + "°C)");
                        break;
                        
                    case DASHBOARD_BTN_TMAX:
                        current_range_selected = RANGE_TMAX;
                        nextion.updateRangeSelection(RANGE_TMAX);
                        Serial.println("Range selezionto: Tmax (" + String(temp_max) + "°C)");
                        break;
                        
                    case DASHBOARD_BTN_UMIN:
                        current_range_selected = RANGE_UMIN;
                        nextion.updateRangeSelection(RANGE_UMIN);
                        Serial.println("Range selezionto: Umin (" + String(umid_min) + "%)");
                        break;
                        
                    case DASHBOARD_BTN_UMAX:
                        current_range_selected = RANGE_UMAX;
                        nextion.updateRangeSelection(RANGE_UMAX);
                        Serial.println("Range selezionto: Umax (" + String(umid_max) + "%)");
                        break;
                }
            }
            
            // === TOUCH FRECCE (controllano range selezionato) ===
            switch (event.component_id) {
                
                case DASHBOARD_BTN_TEMP_UP:
                    if (current_range_selected == RANGE_NONE) {
                        // Comportamento normale: aumenta setpoint temperatura
                        temperatura_setpoint += 0.5;
                        nextion.setText("t1", String((int)temperatura_setpoint));
                        Serial.println("Setpoint aumentato: " + String(temperatura_setpoint));
                    } else {
                        // Aumenta range selezionato
                        adjustSelectedRange(+1);
                    }
                    break;
                    
                case DASHBOARD_BTN_TEMP_DOWN:
                    if (current_range_selected == RANGE_NONE) {
                        // Comportamento normale: diminuisce setpoint temperatura
                        temperatura_setpoint -= 0.5;
                        nextion.setText("t1", String((int)temperatura_setpoint));
                        Serial.println("Setpoint diminuito: " + String(temperatura_setpoint));
                    } else {
                        // Diminuisce range selezionato
                        adjustSelectedRange(-1);
                    }
                    break;
            }
        }
    }
}
```

### **Funzione Regolazione Range:**
```cpp
void adjustSelectedRange(int delta) {
    bool changed = false;
    String range_name;
    String new_value;
    
    switch (current_range_selected) {
        
        case RANGE_TMIN:
            temp_min += delta;
            temp_min = constrain(temp_min, 5.0, temp_max - 1.0);  // Min 5°C, max Tmax-1
            nextion.setText("b3", String((int)temp_min));
            range_name = "Tmin";
            new_value = String(temp_min) + "°C";
            changed = true;
            break;
            
        case RANGE_TMAX:
            temp_max += delta;
            temp_max = constrain(temp_max, temp_min + 1.0, 30.0);  // Min Tmin+1, max 30°C
            nextion.setText("b4", String((int)temp_max));
            range_name = "Tmax";
            new_value = String(temp_max) + "°C";
            changed = true;
            break;
            
        case RANGE_UMIN:
            umid_min += (delta * 5);  // Incrementi di 5% per umidità
            umid_min = constrain(umid_min, 20.0, umid_max - 5.0);  // Min 20%, max Umax-5
            nextion.setText("b5", String((int)umid_min));
            range_name = "Umin";
            new_value = String(umid_min) + "%";
            changed = true;
            break;
            
        case RANGE_UMAX:
            umid_max += (delta * 5);  // Incrementi di 5% per umidità
            umid_max = constrain(umid_max, umid_min + 5.0, 95.0);  // Min Umin+5, max 95%
            nextion.setText("b6", String((int)umid_max));
            range_name = "Umax";
            new_value = String(umid_max) + "%";
            changed = true;
            break;
            
        case RANGE_NONE:
        default:
            // Nessun range selezionato, non fare nulla
            break;
    }
    
    if (changed) {
        Serial.println("Range " + range_name + " aggiornato: " + new_value);
        
        // Salva in EEPROM per persistenza
        saveRangeValuesToEEPROM();
        
        // Mostra feedback visivo temporaneo
        showRangeUpdateFeedback(range_name, new_value);
    }
}
```

### **Feedback Visivo:**
```cpp
void showRangeUpdateFeedback(String range_name, String new_value) {
    // Mostra messaggio temporaneo
    nextion.setText("t12", range_name + ": " + new_value);
    
    // Timer per nascondere messaggio dopo 2 secondi
    static unsigned long feedback_timer = 0;
    feedback_timer = millis();
    
    // Nel loop principale controlla se nascondere il messaggio
    // if (millis() - feedback_timer > 2000) {
    //     nextion.setText("t12", "");
    // }
}
```

### **Persistenza EEPROM:**
```cpp
void saveRangeValuesToEEPROM() {
    // Salva i range in EEPROM per persistenza tra riavvi
    EEPROM.put(0, temp_min);
    EEPROM.put(4, temp_max);
    EEPROM.put(8, umid_min);
    EEPROM.put(12, umid_max);
    
    Serial.println("Range salvati in EEPROM");
}

void loadRangeValuesFromEEPROM() {
    // Carica i range dall'EEPROM all'avvio
    EEPROM.get(0, temp_min);
    EEPROM.get(4, temp_max);
    EEPROM.get(8, umid_min);
    EEPROM.get(12, umid_max);
    
    // Valida i valori caricati
    if (temp_min < 5 || temp_min > 25) temp_min = 16.0;
    if (temp_max < 10 || temp_max > 30) temp_max = 19.0;
    if (umid_min < 20 || umid_min > 80) umid_min = 50.0;
    if (umid_max < 30 || umid_max > 95) umid_max = 75.0;
    
    Serial.println("Range caricati da EEPROM:");
    Serial.println("  Temp: " + String(temp_min) + "°C - " + String(temp_max) + "°C");
    Serial.println("  Umid: " + String(umid_min) + "% - " + String(umid_max) + "%");
}
```

---

## 🎯 **Vantaggi Implementazione**

### **👤 Per l'Utente:**
✅ **Controllo intuitivo**: Touch + frecce familiar  
✅ **Feedback immediato**: Bordi colorati e valori aggiornati  
✅ **Persistenza**: Valori salvati tra riavvi  
✅ **Sicurezza**: Limiti automatici per evitare valori invalidi  

### **🛠️ Per il Sistema:**
✅ **Modalità aware**: Solo in modalità manuale  
✅ **Validazione**: Controlli automatici sui range  
✅ **Logging**: Traccia tutte le modifiche  
✅ **EEPROM**: Persistenza configurazione  

### **💻 Per lo Sviluppo:**
✅ **Modulare**: Funzioni separate e riutilizzabili  
✅ **Debuggabile**: Log completo delle operazioni  
✅ **Estendibile**: Facile aggiungere nuovi range  
✅ **Manutenibile**: Codice chiaro e documentato  

Il sistema permette un **controllo fine e professionale** dei parametri ambientali mantenendo semplicità d'uso e sicurezza! 🎉
