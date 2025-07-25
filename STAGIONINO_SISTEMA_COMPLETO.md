# 🥩 STAGIONINO V1.0 - Sistema Completo

## 📋 Panoramica Sistema Implementato

**Stagionino** è un sistema di controllo ambientale avanzato per la stagionatura artigianale dei salumi, basato su Arduino Mega 2560 con funzionalità di livello industriale.

### ✅ Funzionalità Completamente Implementate

## 🎛️ Sistema di Controllo Intelligente

### Controllo Attuatori
- **6 Attuatori**: Frigorifero, Riscaldatore, Deumidificatore, Umidificatore, Ventola IN/OUT
- **Protezioni Cicli Minimi**: Prevenzione danni meccanici con timing forzati
- **Logica Adattiva**: Sistema modulare per dispositivi disponibili/non disponibili
- **Statistiche Real-time**: Duty cycle e monitoraggio utilizzo ogni 5 minuti

### Modalità Operative
- **🔧 Modalità Manuale**: Controllo parametri fissi configurabili dall'utente
- **🤖 Modalità Automatica**: Esecuzione programmi con avanzamento fasi automatico
- **🌡️ Solo Temperatura**: Controllo temperatura senza gestione umidità

## 📊 Sistema Sensori Avanzato

### Sensori Supportati
- **AM2315C (I2C)**: Sensore interno alta precisione (±0.3°C, ±2% RH)
- **DHT11**: Sensore esterno economico (±2°C, ±5% RH)
- **DS1307**: RTC con batteria backup per timestamp

### Validazione e Protezioni
- **Range Validation**: Controllo automatico range specifici per ogni sensore
- **Retry Automatico**: 3 tentativi con delay per letture fallite
- **Error Counting**: Disabilitazione automatica dopo 10 errori consecutivi
- **Graceful Degradation**: Continuità operativa anche con sensori offline

## 💾 Sistema Programmi e SD Card

### Gestione Programmi
- **Formato Strutturato**: File .txt con sintassi professionale
- **Max 30 Fasi**: Per programma con parametri temperatura/umidità
- **Fase Finale Infinita**: Stagionatura continua fino a stop manuale
- **Avanzamento Automatico**: Transizioni basate su durata temporale

### Gestione SD Card Robusta
- **Retry Automatico**: 3 tentativi inizializzazione con recovery intelligente
- **Fallback Graceful**: Modalità manuale se SD non disponibile
- **Verifica Continua**: Test disponibilità prima di ogni operazione
- **Directory Strutturate**: `/programs/` e `/logs/` auto-create

### Esempi Programmi Inclusi
```
📄 Salame_Felino.txt (3 fasi)
📄 Salame_Milanese.txt (4 fasi)  
📄 README_programmi.md (documentazione)
```

## 🚨 Sistema Emergenze Multi-Livello

### Tipi Emergenza Gestiti
- **CRITICAL**: Sensore interno offline, Temperatura critica (allarme continuo)
- **HIGH PRIORITY**: Temperature estreme, SD failure (beep intermittente)
- **WARNING**: Condizioni borderline (beep preventivi)

### Recovery Automatico
- **Temporizzazioni Smart**: 2min attesa + 1min intervalli + max 3 tentativi
- **Recovery Specifico**: Logica dedicata per ogni tipo emergenza
- **Fallback Intelligente**: Accettazione situazione dopo 3 tentativi falliti

### Statistiche e Log
- **Tempo Emergenza**: Conteggio preciso durata episodi
- **Contatori Recovery**: Tentativi effettuati per tipo
- **Log Dettagliati**: Motivazioni attivazione/disattivazione

## 🔔 Sistema Allarmi Configurabile

### Pattern Sonori Multi-Livello
- **Pattern 0**: Silenzio (normale)
- **Pattern 1**: 1 beep / 2s (high priority)
- **Pattern 2**: 3 beep / 500ms (critical)
- **Pattern 3**: 5 beep / 200ms (emergenza estrema)

### Protezioni Audio
- **Auto-mute**: Dopo 100 beep per evitare disturbo eccessivo
- **Mute Manuale**: Configurabile (default 5 minuti)
- **Toggle Buzzer**: Abilitazione/disabilitazione globale
- **Gestione Overflow**: Correzione automatica timer

## 💡 Sistema LED WS2812B Avanzato

### Strip LED Dual (24+12 LED)
- **Strip 24**: Indicatori attuatori (6 gruppi da 4 LED) + progress programmi
- **Strip 12**: Status sistema (sensori, RTC, SD, modalità, heartbeat)

### Modalità LED Automatiche
- **🟢 NORMAL**: Status attuatori + indicatori sistema
- **🔴 EMERGENCY**: Pattern rosso/giallo lampeggiante  
- **🟣 PROGRAM**: Progress bar fasi + status attuatori
- **🟠 ERROR**: Rosso lampeggiante lento
- **🟤 MAINTENANCE**: Pattern arancione rotante
- **🌈 STARTUP**: Animazione wave colorata

### Effetti Speciali
- **Rainbow Effect**: Arcobaleno rotante
- **Breathing Effect**: Luminosità pulsante
- **Blink Protection**: LED lampeggianti per protezioni attive
- **Heartbeat**: LED bianco pulsante per sistema vivo

## 🖱️ Interfaccia Touch Professionale

### Schermate Complete (6 schermate)
- **🏠 MAIN_DASHBOARD**: Overview sensori + attuatori + navigazione
- **📊 SENSOR_DATA**: Dati dettagliati con contatori errori
- **⚙️ SETTINGS**: Configurazioni e calibrazione touch
- **📋 PROGRAMS**: Gestione programmi (lista + controlli)
- **🚨 EMERGENCY**: Interfaccia emergenza con controlli avanzati
- **🎯 CALIBRATION**: Sistema calibrazione touchscreen

### Controlli Touch Avanzati
- **Anti-Bounce**: 300ms debounce per eliminare tocchi multipli
- **Mappatura Coordinate**: Conversione touch → coordinate schermo
- **Areas Logiche**: Zone touch intuitive per ogni schermata
- **Feedback Visivo**: Ridisegno immediato per responsività

## 🛡️ Protezioni e Sicurezza

### Watchdog e Overflow
- **Watchdog 8s**: Protezione blocchi sistema con auto-riavvio
- **Overflow millis()**: Gestione automatica dopo 50 giorni funzionamento
- **Timer Asincroni**: Intervalli ottimizzati (30s, 15s, 8min, 5min, etc.)

### Validazioni Multiple
- **Dati Sensori**: Range specifici + NaN check + retry
- **Operazioni SD**: Availability check + retry + fallback
- **Controlli Attuatori**: Validation prima di ogni comando
- **Input Touch**: Coordinate validation + debounce

### Cicli Minimi Dispositivi
| Dispositivo | ON Minimo | OFF Minimo | Isteresi |
|-------------|-----------|------------|----------|
| Frigorifero | 5 min | 3 min | ±1.0°C |
| Riscaldatore | 4 min | 2 min | ±0.8°C |
| Deumidificatore | 10 min | 5 min | ±3.0% |
| Umidificatore | 5 min | 3 min | ±2.5% |
| Ventole | 1 min | 30s | ±2.0°C/5.0% |

## 📡 Monitoraggio e Diagnostica

### Output Seriale Completo (115200 baud)
- **Inizializzazione**: Status dettagliato ogni componente
- **Dati Sensori**: Log ogni 5 letture con timestamp
- **Controlli Attuatori**: Motivazioni accensione/spegnimento
- **Emergenze**: Log completo attivazione/recovery/exit
- **Statistiche**: Duty cycle attuatori ogni 5 minuti
- **Touch Debug**: Coordinate e azioni per calibrazione

### Heartbeat e Status
- **LED Heartbeat**: Pulse bianco ogni 2 secondi (sistema vivo)
- **Uptime Display**: Tempo funzionamento in minuti
- **Status Componenti**: Verde/Rosso per ogni modulo
- **Recovery Attempts**: Contatori tentativi per debug

## 🔧 Hardware Supportato

### Pinout Arduino Mega 2560 Completo
```
I2C: SDA(20), SCL(21) → AM2315C + DS1307
Digital: D2 → DHT11
SPI: D4(SD_CS), D6(TOUCH_CS), D7(TOUCH_IRQ), D11(MOSI), D12(MISO), D13(SCK)
LED: D8(24bit), D9(12bit) → WS2812B strips
Buzzer: D10 → Buzzer passivo (PWM/tone)
Relè: D22-D27 → 6 attuatori (logica invertita)
Display: Pin automatici 8-bit parallelo (MCUFRIEND_kbv)
```

### Librerie Arduino Required
```cpp
- Adafruit AM2315 (v2.1.0+)    // Sensore interno
- DHT sensor library (v1.4.4+) // Sensore esterno  
- RTClib (v2.1.1+)             // Real Time Clock
- MCUFRIEND_kbv (v2.9.9+)      // Display ILI9486
- XPT2046_Touchscreen (v1.4+)  // Touch capacitivo
- FastLED (v3.5.0+)            // LED WS2812B
+ Librerie incluse: Wire, SPI, EEPROM, SD, avr/wdt.h
```

## 🚀 Performance e Ottimizzazioni

### Gestione Memoria
- **Strutture Ottimizzate**: Packing efficiente dati
- **Buffer Circolari**: Per grafici e log
- **Loading Dinamico**: Programmi caricati solo quando necessario
- **String Management**: Uso F() macro per PROGMEM

### Temporizzazioni Ottimizzate
- **Controlli Asincroni**: Evita delay() nel loop principale
- **Intervalli Sfalsati**: Previene collisioni temporali
- **Priority-Based**: Controlli più critici più frequenti
- **Batch Operations**: Operazioni SD raggruppate

### Resource Management
- **LED Brightness**: 50% default per evitare sovraccarico
- **Watchdog Smart**: 8s per evitare reset accidentali
- **SD Retry**: Max 3 tentativi per evitare hang
- **Memory Footprint**: Ottimizzato per Arduino Mega 2560

## 📊 Statistiche Sistema

### Codice Implementato
- **~3000+ righe**: Codice C++ professionale
- **~100 funzioni**: Modulari e documentate  
- **~20 strutture**: Dati organizzati e tipizzati
- **~50 costanti**: Configurabili e ottimizzate

### Funzionalità Testate
- ✅ Inizializzazione hardware completa
- ✅ Lettura sensori con validazione
- ✅ Controllo attuatori con protezioni
- ✅ Interfaccia touch multi-schermata
- ✅ Sistema emergenze con recovery
- ✅ LED patterns per tutte le modalità
- ✅ Gestione SD con fallback
- ✅ Sistema allarmi configurabile

## 🎯 Caso d'Uso Completo

### Scenario: Stagionatura Salame Felino

1. **🔄 Avvio Sistema**
   - Animazione LED startup
   - Inizializzazione hardware
   - Caricamento lista programmi da SD
   - Dashboard pronto

2. **📱 Selezione Programma**
   - Touch "PROGRAMMI" → Lista programmi SD
   - Selezione "Salame Felino"
   - Avvio automatico

3. **🏃 Esecuzione Automatica**
   - **Fase 1**: Stufatura (18-20°C, H:NC, 48h)
     - LED progress bar blu
     - Controllo solo temperatura
     - Avanzamento automatico dopo 48h
   
   - **Fase 2**: Asciugatura (19-20°C, 70-85%, 168h)
     - LED progress bar verde  
     - Controllo temperatura + umidità
     - 7 giorni durata
   
   - **Fase 3**: Invecchiamento (10-12°C, 58-62%, ∞)
     - LED breathing effect (fase infinita)
     - Stagionatura continua fino a stop manuale

4. **📊 Monitoraggio Continuo**
   - LED strip 24: Status attuatori real-time
   - LED strip 12: Sistema + sensori + heartbeat
   - Display: Dati sensori + progress + controlli
   - Seriale: Log completo operazioni

5. **🚨 Gestione Emergenze**
   - Auto-detection condizioni critiche
   - Modalità LED emergenza (rosso lampeggiante)
   - Allarmi sonori prioritizzati
   - Recovery automatico quando possibile

6. **✅ Completamento**
   - Fase finale infinita 
   - Stop manuale quando desiderato
   - Statistiche complete salvate

## 🎉 Sistema Pronto per Produzione

**Stagionino V1.0** è ora un sistema **completo e professionale** pronto per:

- ✅ **Uso Industriale**: Affidabilità e robustezza per uso continuo
- ✅ **Stagionatura Reale**: Controllo preciso per salumi artigianali  
- ✅ **Manutenzione Facile**: Diagnostica avanzata e interfaccia intuitiva
- ✅ **Espandibilità**: Architettura modulare per funzionalità future
- ✅ **Sicurezza Alimentare**: Protezioni multiple per evitare deterioramento

🥩 **Buona stagionatura con Stagionino!** ✨ 