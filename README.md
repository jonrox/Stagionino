# 🥩 Stagionino V1.2 - Sistema Completo
## Centralina Intelligente per Stagionatura Salumi - **VERSIONE FINALE OTTIMIZZATA**

## 📋 Panoramica del Progetto

**Stagionino V1.2** è un sistema di controllo ambientale **completo e professionale** per la stagionatura artigianale dei salumi, basato su Arduino Mega 2560 con interfaccia touch grafica avanzata. 

Il sistema mantiene automaticamente le condizioni ottimali di temperatura e umidità per la stagionatura dei salumi, controllando 6 attuatori (frigorifero, riscaldatore, deumidificatore, umidificatore, ventole) con **protezioni multiple**, **sistema emergenze avanzato**, **LED WS2812B colorati** e **interfaccia touch professionale**.

### 🎉 **SISTEMA COMPLETO E FUNZIONALE V1.2** 
✅ **Display Nextion NX4832K035** | ✅ **Interfaccia Touch professionale** | ✅ **Modalità demo avanzata** | ✅ **Pronto per uso reale**

### 🎯 Caratteristiche Principali

- **Sistema a Stati Finiti**: Controllo dinamico e adattivo basato sui dispositivi disponibili
- **Modalità Automatica**: Programma selezionato con transizioni automatiche fasi successive (Massimo 15 fasi per programma ottimizzate)
- **Programmi Ottimizzati**: Sistema di programmi con fasi cicliche salvati su SD card (gestione memoria migliorata)

- **🧪 Modalità Demo Avanzata V1.2**: Test completo sistema senza sensori fisici
  - **Attivazione Automatica**: Si attiva automaticamente se nessun sensore è rilevato
  - **Attivazione Manuale**: Toggle ON/OFF tramite Impostazioni → DEMO ON/OFF
  - **Comando Seriale**: `demo` / `nodemo` per controllo rapido
  - **Dati Simulati Realistici**: Variazioni lente e graduali (±2°C, ±5% umidità)
  - **Indicatori Visivi**: Dati sensori in giallo con tag "SIM", titolo "MODALITÀ DEMO"
  - **Funzionalità Complete**: Tutti i controlli, interfaccia e retroilluminazione funzionanti
  - **Protezione LED**: Evita modalità ERROR durante demo e inizializzazione

- **Modalità Manuale**: Controllo continuo con parametri configurabili dall'utente
- **Controllo Solo Temperatura**: Opzione per controllare solo la temperatura

- **🖥️ Display e Touch Ottimizzati V1.2**:
  - **Display Nextion**: NX4832K035 con interfaccia touch professionale
  - **Pin Condivisi Gestiti**: Soluzione robusta per pin Touch/SD/TFT condivisi
  - **Test Pattern**: Comando `testdisplay` per diagnostica colori (rosso, verde, blu, bianco, nero)
  - **Touch Calibrato**: Comando `testtouch` per verifica touchscreen XPT2046
  - **Display Reattivo**: Aggiornamento ogni 2 secondi (era 10s)
  - **Anti-freeze**: Prevenzione blocchi durante inizializzazione

- **Dashboard Intuitiva**: Temperatura/umidità attuale con range target, frecce controllo, icone attuatori
- **Anti-bounce Touch**: Controllo tocchi multipli con timeout 300ms per precisione input
- **Monitoraggio Real-time**: Sensori di precisione interni ed esterni con filtraggio avanzato
- **Log Temporali**: RTC per timestamp e monitoraggio continuo
- **Indicatori LED**: Stato visivo del sistema con animazioni ottimizzate
- **Protezione Watchdog**: Auto-riavvio in caso di blocchi (8 secondi) con ripresa stato
- **Gestione Errori Robusta**: Retry automatico, fallback sicuro e controllo completo

## 🚀 **OTTIMIZZAZIONI VERSIONE 1.2**

### **🖥️ DISPLAY E TOUCHSCREEN OTTIMIZZATI**
- **Display Moderno**: Nextion NX4832K035 con processore grafico dedicato
- **Gestione Pin Condivisi**: Soluzione robusta per pin Touch/SD/TFT condivisi su shield MCUFRIEND
- **Diagnostica Display**: Test pattern colorati per verifica funzionamento
- **Touch Preciso**: Ripristino pin dopo ogni lettura per compatibilità shield
- **Display Veloce**: Aggiornamento 2s invece di 10s per interfaccia reattiva
- **Inizializzazione Robusta**: Progress visivo e protezioni anti-blocco

### **⚡ GESTIONE PIN SPI/TFT CONDIVISI**
- **Problema Risolto**: Pin touchscreen condivisi con display TFT su shield Arduino
- **Soluzione MCUFRIEND**: Ripristino pin dopo ogni `getPoint()` come da forum Arduino
- **Funzione Helper**: `Touch_getXY()` basata su best practice community
- **CS Management**: Gestione corretta Chip Select per Touch/SD/TFT
- **Debug Avanzato**: Coordinate touch in tempo reale e test diagnostici

### **🔧 SISTEMA LED OTTIMIZZATO**
- **Protezione Demo**: LED non vanno in modalità ERROR durante demo
- **Animazione Veloce**: Startup animation semplificata per evitare blocchi
- **Watchdog Safe**: Reset watchdog durante animazioni per stabilità
- **Modalità Intelligente**: Rilevamento automatico stato sistema per LED appropriati

### **MEMORIA RAM OTTIMIZZATA (+2000 bytes)**
- **Ridotto**: Array programmi da 20x64 a 10x32 char (-1280 bytes)
- **Ridotto**: Buffer descrizioni da 128 a 64 char (-480 bytes totali)
- **Ridotto**: Fasi programma da 30 a 15 (-720 bytes)
- **Stringhe PROGMEM**: Interfaccia utente (-200 bytes stimati)
- **Risultato**: **+2000 bytes RAM libera** (da 2370 a ~4370 bytes)

### **TIMING CRITICO 4X PIÙ VELOCE**
- **Sensori**: 30s → **15s** (risposta più rapida)
- **Display**: 10s → **2s** (interfaccia molto fluida)
- **Frigorifero**: 8min → **2min** (controllo critico)
- **Riscaldatore**: 5min → **1.5min** (precisione termica)
- **Controllo Umidità**: 3-4min → **1min** (stabilità ambiente)
- **Timing Adattivo**: 30s per temperature critiche (emergenze)

### **🛡️ GESTIONE SD ROBUSTA**
- **Timeout SD**: Max 3 secondi per inizializzazione
- **Pin CS Dedicato**: Gestione corretta pin 4 per SD
- **Retry Intelligente**: Tentativi multipli con delay ottimizzati
- **Non-blocking**: Operazioni SD asincrone con watchdog protection
- **Fallback Sicuro**: Sistema funziona anche senza SD

### **📊 FILTRAGGIO SENSORI AVANZATO**
- **Media Mobile**: 3 campioni per stabilizzare letture
- **Rilevamento Spike**: Soglie ±5°C / ±15% per scartare anomalie
- **Buffer Circolare**: Gestione efficiente memoria per filtri
- **Validazione Robusta**: Controlli multipli prima dell'applicazione

### **🚨 SISTEMA EMERGENZE AFFINATO**
- **Soglie Critiche**: Ottimizzate a **5°C** (bilanciate vs sbalzi momentanei)
- **Temperature Estreme**: Range sicurezza **-5/+35°C** 
- **Fallimenti Sensore**: Soglia ridotta a **6** errori consecutivi
- **Recovery Intelligente**: Validazioni multiple (2/3 test sensori)
- **Recovery Temperatura**: Trend migliorativo + tempo minimo

### **🔍 DIAGNOSTICA PRODUZIONE CON TOUCH**
- **Schermata Diagnostica**: Accessibile da Impostazioni → Diagnostica
- **Test Display**: `nextion` - Test comunicazione e funzionalità display Nextion
- **Test Touch**: `testtouch` - Verifica touchscreen XPT2046 con punti visuali
- **Test Memoria**: Analisi RAM in tempo reale con soglie colorate
- **Status Hardware**: Sensori, SD, RTC con indicatori visivi
- **Comandi Seriali**: `status`, `refresh`, `backlight` per debug completo

### ✅ **Compatibilità Verificata Post-Ottimizzazione V1.2**
✅ **Display**: Nextion NX4832K035 con interfaccia touch professionale  
✅ **Touch**: Integrato nel display Nextion (nessun pin aggiuntivo)  
✅ **Memoria RAM**: +2000 bytes libera (margine sicurezza)  
✅ **Timing**: Risposta 4x più rapida in condizioni critiche  
✅ **SD Card**: Gestione robusta senza blocchi sistema  
✅ **Sensori**: Filtraggio spike + media mobile  
✅ **Modalità Demo**: Sistema completo funzionante senza sensori fisici  
✅ **LED System**: Protezioni anti-blocco e modalità intelligenti  

## 🚨 Modalità Emergenza Frigorifero

### Panoramica
Il sistema include una modalità di emergenza automatica che mantiene la temperatura di un frigorifero normale (4°C ±1°C) quando si verificano anomalie critiche, prevenendo il deperimento dei prodotti stagionati.

### Attivazione Automatica
La modalità emergenza si attiva automaticamente quando:
- **Sensore interno non risponde**: 6 cicli di lettura consecutivi falliti (ottimizzato da 10)
- **Temperatura critica**: Temperatura oltre 5°C dal target per più di 15 minuti
- **Ventilazione Emergenza**: Se temperatura interna > esterna, attiva ventola immissione

### Indicatori Visivi di Emergenza
- **LED 24bit e 12bit**: Pattern rosso/giallo alternato ogni secondo
- **Display**: Schermata rossa con messaggio di emergenza prominente
- **Touch Responsive**: Interfaccia touch rimane funzionale per controllo manuale

### Recupero Automatico
Il sistema esce automaticamente dalla modalità emergenza quando:
- Il sensore interno torna a comunicare correttamente per 2/3 test consecutivi
- La temperatura rientra sopra la soglia critica (target - 5°C)

## 🧪 Modalità Demo Completa V1.2

### **Panoramica**
La modalità demo permette di testare completamente il sistema Stagionino senza collegare sensori fisici, utilizzando dati simulati realistici per dimostrazioni, test di sviluppo, o verifica dell'interfaccia.

### **🔄 Attivazione Modalità Demo**

#### **Attivazione Automatica**
- **Trigger**: Nessun sensore AM2315C o DHT11 rilevato durante l'inizializzazione
- **Comportamento**: Sistema si avvia automaticamente in modalità demo
- **Protezione LED**: Sistema non va in modalità ERROR con demo attiva

#### **🎛️ Attivazione Manuale**
- **Percorso Display**: Dashboard → Impostazioni → Pulsante "DEMO ON/OFF"
- **Comandi Seriali V1.2**: 
  - `demo` → Attiva modalità demo
  - `nodemo` → Disattiva modalità demo  
  - `status` → Mostra stato completo sistema
  - `nextion` → Test diagnostico display Nextion NX4832K035
  - `testtouch` → Test diagnostico touchscreen XPT2046
  - `refresh` → Forza aggiornamento display immediato

### **📊 Caratteristiche Dati Simulati**
```
Temperatura Interna: 12.5°C (±2.0°C variazione graduale)
Umidità Interna:     60.0% (±5.0% variazione graduale)  
Temperatura Esterna: 15.0°C (±1.6°C variazione graduale)
Umidità Esterna:     55.0% (±4.5% variazione graduale)
```

#### **⏱️ Dinamiche Temporali Ottimizzate**
- **Aggiornamento**: Ogni 15 secondi con variazioni graduali (era 30s)
- **Variazioni**: ±0.2°C e ±0.5% per ciclo
- **Stabilità**: Nessuna variazione brusca o spike anomali
- **Realismo**: Trend lenti e naturali per simulazione credibile

### **🎨 Indicatori Visivi Modalità Demo**

#### **🏠 Dashboard Principale**
- **Titolo**: "Sistema Stagionatura Salumi - **MODALITÀ DEMO**" (giallo)
- **Dati Sensori**: Colore giallo invece di verde/ciano
- **Tag Identificativo**: " SIM" dopo ogni valore di temperatura/umidità
- **Esempio**: `Interno: 12.3°C 59.8% SIM` (tutto in giallo)

#### **⚙️ Schermata Impostazioni**
- **Pulsante Demo**: "DEMO ON" (giallo) / "DEMO OFF" (grigio)
- **Tipo Demo**: "DEMO: FORZATA" o "DEMO: AUTO"
- **Status Visibile**: Indicazione chiaramente visibile del tipo di demo attivo

### **✅ Funzionalità Complete in Demo V1.2**

#### **Completamente Funzionanti**
- **🖥️ Display Touch**: Tutti i pulsanti, navigazione, test diagnostici
- **💡 Retroilluminazione**: Auto-dim, profili, controlli PWM completi
- **🔧 Menu Impostazioni**: Calibrazione, diagnostica, tutti i controlli
- **📊 Visualizzazione**: Dashboard, grafici, statistiche in tempo reale
- **🎛️ Controlli Sistema**: Tutti i pulsanti e configurazioni funzionanti
- **🔍 Diagnostica**: Test memoria, status hardware, stress test completi
- **🎨 LED System**: Animazioni e indicatori completamente funzionali

#### **⚠️ Limitazioni Protettive**
- **🚫 Controllo Attuatori**: Relè controllati ma senza azioni sui sensori fisici
- **🚫 Modalità Emergenza**: Non si attiva mai con dati simulati stabili
- **🚫 Allarmi Critici**: Dati simulati sempre in range sicuro

## 📋 Esempi Pratici di Programmi

### **Programmi Ottimizzati (Max 15 Fasi)**

#### Salame Felino (3 fasi - Classico)
```
Fase 1: Stufatura
- Temperatura: 18-20°C (isteresi ±0.5°C)
- Umidità: Non controllata
- Durata: 2 giorni

Fase 2: Asciugatura
- Temperatura: 19-20°C (isteresi ±0.5°C)
- Umidità: 70% - 85% (isteresi ±2%)
- Durata: 7 giorni

Fase 3: Invecchiamento
- Temperatura: 10-12°C (isteresi ±0.5°C)
- Umidità: 60% (isteresi ±2%)
- Durata: 30 giorni
Fine fase: Stagionatura continua
```

#### Salame Milanese (4 fasi - Dettagliato)
```
Fase 1: Stufatura
- Temperatura: 16-24°C (isteresi ±0.5°C)
- Umidità: Non controllata
- Durata: 1 giorno

Fase 2: Pre-Asciugatura
- Temperatura: Max 18°C
- Umidità: Range 70-80% (isteresi ±2%)
- Durata: 2 giorni

Fase 3: Asciugatura
- Temperatura: Max 18°C
- Umidità: 82% → 85% → 88% (progressiva, isteresi ±2%)
- Durata: 3 giorni

Fase 4: Invecchiamento
- Temperatura: 8-14°C (isteresi ±0.5°C)
- Umidità: Range 60-70% (isteresi ±2%)
- Durata: 25 giorni
Fine fase: Stagionatura continua
```

## 🔧 Componenti Hardware

### Componenti Principali
| Componente | Modello | Quantità | Funzione |
|------------|---------|----------|----------|
| **Controller** | Arduino Mega 2560 | 1 | Controllo principale |
| **Display** | Nextion NX4832K035 LCD 3.5" Touch | 1 | Interfaccia utente professionale |
| **Sensore Interno** | AM2315C | 1 | Temperatura/umidità interna |
| **Sensore Esterno** | DHT11 | 1 | Temperatura/umidità esterna |
| **RTC** | DS1307 | 1 | Orologio real-time |
| **Modulo Relè** | 6V 6 Channel | 1 | Controllo attuatori |
| **LED Indicatori** | WS2812B 5050 RGB 24bit | 1 strip | Indicatori principali |
| **LED Secondari** | WS2812B 5050 RGB 12bit | 1 strip | Indicatori secondari |
| **Buzzer** | Passivo Piezo 5V | 1 | Allarmi acustici differenziati |

### Attuatori Controllati
- **Frigorifero**: Raffreddamento quando temperatura troppo alta
- **Riscaldatore**: Mantenimento temperatura minima
- **Deumidificatore**: Riduzione umidità eccessiva
- **Umidificatore**: Aumento umidità insufficiente
- **Ventola Immissione**: Introduzione aria esterna per ricircolo
- **Ventola Estrazione**: Estrazione aria interna per ricircolo

## 📐 Schema Elettrico V1.2

### **🔌 Pinout Arduino Mega 2560 (Ottimizzato)**

| Pin | Componente | Funzione | Note V1.2 |
|-----|------------|----------|-----------|
| **SDA(20), SCL(21)** | AM2315C + DS1307 | I2C - Sensori e RTC | Standard I2C |
| **D2** | DHT11 | Sensore esterno | Pin dedicato |
| **UART** | Nextion NX4832K035 | Display TFT con touch seriale | TX1=18, RX1=19 |
| **D11** | MOSI | SD CARD MOSI | **SPI** |
| **D12** | MISO | SD CARD MISO | **SPI** |
| **D13** | SCK | SD CARD SCK | **SPI** |
| **D4** | CS | SD CARD CS | **Pin dedicato** |
| **D44** | BACKLIGHT | Controllo retroilluminazione PWM | Opzionale se supportato |
| **D8** | WS2812B 24bit | LED indicatori principali | Pin dedicato |
| **D9** | WS2812B 12bit | LED indicatori secondari | Pin dedicato |
| **D10** | Buzzer Passivo | Allarmi acustici (PWM/tone) | Pin dedicato |
| **D22-D27** | Relè 1-6 | Controllo attuatori | Logica invertita |

### **✅ VANTAGGI NEXTION: Pin Liberi**
- **Solo UART**: TX1(18), RX1(19) per display e touch
- **Pin SPI**: Solo per SD Card - nessun conflitto
- **18+ Pin liberati**: Disponibili per altre funzioni
- **Cablaggio semplice**: 4 fili vs 20+ precedenti

#### **📚 Librerie Arduino Necessarie V1.2**
```cpp
// Installare tramite Arduino IDE Library Manager:
#include <Wire.h>                    // I2C (inclusa)
#include <Adafruit_AM2315.h>         // Sensore AM2315C (v2.1.0+)
#include <DHT.h>                     // Sensore DHT11 (v1.4.4+)
#include <RTClib.h>                  // RTC DS1307 (v2.1.1+)
#include "nextion_protocol.h"        // Display Nextion NX4832K035 (tutto-in-uno)
#include <FastLED.h>                 // LED WS2812B (v3.5.0+)
#include <EEPROM.h>                  // Memoria persistente (inclusa)
#include <SD.h>                      // SD Card (inclusa)
#include <SPI.h>                     // SPI (inclusa)
#include <avr/wdt.h>                 // Protezione watchdog (inclusa)
```

## ⏱️ Temporizzazioni Ottimizzate V1.2

### **⚡ Temporizzazioni Operative (4x Più Veloci)**
- **Lettura sensori**: Ogni **15 secondi** (era 30s)
- **Aggiornamento display**: Ogni **2 secondi** (era 10s)
- **Controllo frigorifero**: Ogni **2 minuti** (era 8min)
- **Controllo riscaldatore**: Ogni **1.5 minuti** (era 5min)
- **Controllo deumidificatore**: Ogni **1 minuto** (era 4min)
- **Controllo umidificatore**: Ogni **1 minuto** (era 3min)
- **Controllo ventole**: Ogni **45 secondi** (era 2min)

### **🚨 Controllo Adattivo Emergenze**
- **Temperatura critica**: Controllo ogni **30 secondi** (deviazione >3°C)
- **Temperatura borderline**: Controllo ogni **1 minuto** (deviazione >1.5°C)
- **Modalità normale**: Intervalli standard ottimizzati

### **🛡️ Cicli Minimi Dispositivi (Protezione Meccanica)**
| Dispositivo      | Ciclo Minimo ON | Ciclo Minimo OFF | Isteresi |
|------------------|-----------------|------------------|----------|
| **Frigorifero**  | 5 minuti        | 3 minuti         | ±1.0°C   |
| **Riscaldatore** | 4 minuti        | 2 minuti         | ±0.8°C   |
| **Deumidificatore** | 10 minuti    | 5 minuti         | ±3.0%    |
| **Umidificatore** | 5 minuti      | 3 minuti         | ±2.5%    |
| **Ventole** | 1 minuto       | 30 secondi       | ±2.0°C/±5.0% |

## 🔍 Validazione Dati Sensori

### **📊 Range di Validazione Specifici**
| Sensore | Temperatura | Umidità | Precisione | Note |
|---------|-------------|---------|------------|------|
| **AM2315C** | -40°C a +80°C | 0% a 100% | ±0.3°C, ±2% RH | Sensore interno, alta precisione |
| **DHT11** | 0°C a +50°C | 20% a 90% | ±2°C, ±5% RH | Sensore esterno, economico |

### **⚠️ Comportamento in Caso di Dati Non Validi**
1. **Dati fuori range** → Impostazione NAN e avviso su display
2. **Controlli bloccati** → Nessuna azione attuatori con dati non validi
3. **Sistema stabile** → Mantenimento stato precedente o modalità emergenza
4. **Filtraggio spike** → Soglie ±5°C/±15% per scartare anomalie

### **📱 Comandi Seriali Debug V1.2**
```
demo         → Attiva modalità demo
nodemo       → Disattiva modalità demo  
status       → Mostra stato completo sistema
nextion      → Test diagnostico display Nextion NX4832K035
testtouch    → Test diagnostico touchscreen XPT2046
refresh      → Forza aggiornamento display immediato
backlight    → Test controllo retroilluminazione
```

---

## 🏆 **Sistema Stagionino V1.2 - Caratteristiche Finali**

✅ **Display Ottimizzato**: Nextion NX4832K035 con interfaccia touch professionale e comunicazione seriale  
✅ **Touch Precision**: Integrato nel Nextion con calibrazione automatica  
✅ **Modalità Demo Avanzata**: Sistema completo funzionante senza sensori fisici  
✅ **Performance 4x**: Timing ottimizzato per controllo critico real-time  
✅ **Memoria Ottimizzata**: +2000 bytes RAM libera per stabilità  
✅ **Diagnostica Completa**: Test hardware integrati nell'interfaccia touch  
✅ **Gestione Errori**: Protezioni multiple e recovery automatico  
✅ **Produzione Ready**: Sistema testato e validato per uso professionale  

**Stagionino V1.2** rappresenta la versione finale ottimizzata del sistema di controllo per stagionatura salumi, con tutte le migliorie necessarie per un utilizzo professionale stabile e affidabile.
