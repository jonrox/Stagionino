# 🥩 Stagionino V1.0 - Sistema Completo
## Centralina Intelligente per Stagionatura Salumi - **IMPLEMENTAZIONE FINALE**

## 📋 Panoramica del Progetto

**Stagionino V1.0** è un sistema di controllo ambientale **completo e professionale** per la stagionatura artigianale dei salumi, basato su Arduino Mega 2560 con interfaccia touch grafica avanzata. 

Il sistema mantiene automaticamente le condizioni ottimali di temperatura e umidità per la stagionatura dei salumi, controllando 6 attuatori (frigorifero, riscaldatore, deumidificatore, umidificatore, ventole) con **protezioni multiple**, **sistema emergenze avanzato**, **LED WS2812B colorati** e **interfaccia touch professionale**.

### 🎉 **SISTEMA COMPLETO E FUNZIONALE** 
✅ **Tutti gli 8 step implementati** | ✅ **Pronto per uso reale** | ✅ **Codice testato e ottimizzato**

### 🎯 Caratteristiche Principali

- **Sistema a Stati Finiti**: Controllo dinamico e adattivo basato sui dispositivi disponibili
Possiblità di scegliere se presente umidificatore,deumidificatore,Ventola Immissione,Ventola Estrazione
se presenti tutti i componenti le ventole vengono utilizzate per i cicli di cambio aria per ventilazione
Se presente umidificatore e non deumidificatore, usare la ventola di estrazione per abbassare umidità
Se presente deumidificatore e non umidificatore usare ventola di immissione con frigorifero e riscaldatore attivo per creare umidità
se non presente umidificatore e deumidificatore cercare di regolare umidità con entrambe le ventole in base alla situazione
Frigorifero e riscaldatore sempre presenti
- **Modalità Automatica**: Programma selezionato con transizioni automatiche fasi sucessive
Massimo 30 fasi per programma
- **Programmi Illimitati**: Sistema di programmi con fasi cicliche salvati su SD card (infiniti)

- **Modalità Manuale**: Controllo continuo con parametri configurabili dall'utente (Impostazione manuale Tmax-Tmin Umin-Umax ciclo continuo)senza fasi
- **Controllo Solo Temperatura**: Opzione per controllare solo la temperatura

- **Interfaccia Touch Completa**: Configurazione parametri, programmi e monitoraggio tramite display 3.5"
- **Dashboard Intuitiva**: Tattuale con tmax-tmin Uattuale con umax-umin, frecce per aumentare o diminuire, icona che indica attuatore acceso (frigorifero,riscaldatore,umidificatore,deumidificatore,Ventola Immissione,Ventola Estrazione). T/U esterna ed orario.
in modalità automatica mostrare da quanto è iniziato il programma, fase ciclo e tempo rimanente
- **Anti-bounce Touch**: Controllo tocchi multipli con timeout 300ms per precisione input
- **Monitoraggio Real-time**: Sensori di precisione interni ed esterni
- **Log Temporali**: RTC per timestamp e monitoraggio continuo
- **Indicatori LED**: Stato visivo del sistema con conto alla rovescia
- **Protezione Watchdog**: Auto-riavvio in caso di blocchi (15 secondi)e ripresa fase se attivo programma o continuo manuale
- **Cicli Minimi Dispositivi**: Protezione meccanica per attuatori
- **Timer Asincroni**: Intervalli ottimizzati per evitare conflitti (30s, 12s, 33s)
- **Gestione Errori SD Card**: Retry automatico, fallback sicuro e controllo spazio disponibile
- **Validazione Dati Sensori**: Controlli specifici per AM2315C e DHT11
- **Protezione Controllo Attuatori**: Validazione dati prima di ogni controllo automatico e manuale
- **Gestione Overflow millis()**: per prevenire problemi dopo 50 giorni

- **Ventole per Salubrità**: Ricircolo aria indipendente impostazioni cicli/24h e durata paramtrizzabili da interfaccia
- **Buzzer Configurabile**: Avvisi acustici abilitabili/disabilitabili
- **Mute Temporaneo**: Silenziamento allarmi attivi per 5 minuti+
- **Gestione Overflow millis()**: Prevenzione perdita stato mute allarme
- **Sistema Allarmi Intelligente**: Monitoraggio continuo con notifiche su display e lampeggio LED 
## 🚨 Modalità Emergenza Frigorifero

### Panoramica
Il sistema include una modalità di emergenza automatica che mantiene la temperatura di un frigorifero normale (4°C ±1°C) quando si verificano anomalie critiche, prevenendo il deperimento dei prodotti stagionati.
Se attiva modalità di emergenza e temperatura interna > esterna azionare ventola immissione per abassare la temperatura (se presente)

### Attivazione Automatica
La modalità emergenza si attiva automaticamente quando:
- **Sensore interno non risponde**: 10 cicli di lettura consecutivi falliti
- **Temperatura critica**: Temperatura scende oltre 5°C sotto il target per più di 15 minuti
### Indicatori Visivi di Emergenza
- **LED 24bit e 12bit**: Lampeggiano alternatamente in rosso (ogni secondo)
- **Display**: Schermata rossa con messaggio di emergenza prominente
- **Tempo di emergenza**: Visualizzazione del tempo trascorso in modalità emergenza
### Recupero Automatico
Il sistema esce automaticamente dalla modalità emergenza quando:
- Il sensore interno torna a comunicare correttamente
- La temperatura rientra sopra la soglia critica (target - 5°C)
	
- **Storage Intelligente**: Caricamento programmi in memoria solo quando necessario per ottimizzare RAM
- **Gestione errori SD**: Segnalazione errori lettura/danneggiamento SD
### **Sistema di Retry Automatico**
- **Inizializzazione**: 3 tentativi con delay di 1 secondo
- **Verifica stato**: Controllo SD prima di ogni operazione
- **Fallback sicuro**: modalità emergenza se SD danneggiata e preset non in memoria in modalità automatica, altrimenti se non è presente SD solo modalità manuale
- **Continuo funzionamento**: Sistema operativo anche senza SD
### **Comportamento in Caso di Errore**
1. **SD non risponde** → 3 tentativi di inizializzazione
2. **Fallimento inizializzazione** → modalità emergenza se SD danneggiata e preset non in memoria in modalità automatica, altrimenti se non è presente SD solo modalità manuale

-** Sistema Grafici**: Oscillazioni temperatura/umidità per l'intero programma oppure in caso manuale per un dato tempo, buffer circolare

### Esempi Pratici di Programmi salvati in SD e richiamabili

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
Fine fase Invecchiamento\stagionatura (stessi parametri finche non viene terminato manualmente il programma)
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
Fine fase Invecchiamento\stagionatura (stessi parametri finche non viene terminato manualmente il programma)
```

#### Prosciutto Crudo (6 fasi - Molto Dettagliato)
```
Fase 1: Stufatura
- Temperatura: 14-22°C (isteresi ±0.5°C)
- Umidità: Non controllata
- Durata: 3 giorni

Fase 2: Pre-Asciugatura
- Temperatura: Max 16°C
- Umidità: Range 65-75% (isteresi ±2%)
- Durata: 2 giorni

Fase 3: Asciugatura G1
- Temperatura: Max 16°C
- Umidità: 75% (costante, isteresi ±2%)
- Durata: 1 giorno

Fase 4: Asciugatura G2
- Temperatura: Max 16°C
- Umidità: 78% (costante, isteresi ±2%)
- Durata: 1 giorno

Fase 5: Asciugatura G3
- Temperatura: Max 16°C
- Umidità: 82% (costante, isteresi ±2%)
- Durata: 5 giorni

Fase 6: Invecchiamento
- Temperatura: 6-12°C (isteresi ±0.5°C)
- Umidità: Range 50-60% (isteresi ±2%)
- Durata: 365 giorni (1 anno)
Fine fase Invecchiamento\stagionatura (stessi parametri finche non viene terminato manualmente il programma)
```


--## 🔧 Componenti Hardware

--### Componenti Principali
| Componente | Modello | Quantità | Funzione |
|------------|---------|----------|----------|
| **Controller** | Arduino Mega 2560 | 1 | Controllo principale |
| **Display** | ILI9486 LCD 3.5" Touch | 1 | Interfaccia utente |
| **Sensore Interno** | AM2315C | 1 | Temperatura/umidità interna |
| **Sensore Esterno** | DHT11 | 1 | Temperatura/umidità esterna |
| **RTC** | DS1307 | 1 | Orologio real-time |
| **Modulo Relè** | 6V 6 Channel | 1 | Controllo attuatori |
| **LED Indicatori** | WS2812B 5050 RGB 24bit | 1 strip | Indicatori principali |
| **LED Secondari** | WS2812B 5050 RGB 12bit | 1 strip | Indicatori secondari |
| **Buzzer** | Passivo Piezo 5V | 1 | Allarmi acustici differenziati |

--### Attuatori Controllati
- **Frigorifero**: Raffreddamento quando temperatura troppo alta
- **Riscaldatore**: Mantenimento temperatura minima
- **Deumidificatore**: Riduzione umidità eccessiva
- **Umidificatore**: Aumento umidità insufficiente
- **Ventola Immissione**: Introduzione aria esterna per ricircolo
- **Ventola Estrazione**: Estrazione aria interna per ricircolo

--### Componenti Aggiuntivi Necessari
- **Alimentazione**: Alimentatore 12V/5A
- **Regolatori**: 5V/3A
- **Scatole Stagno**: IP65 per protezione umidità
- **Cavi e Connettori**: Impermeabili per ambiente umido

--## 📐 Schema Elettrico

--### Pinout Arduino Mega 2560

| Pin | Componente | Funzione |
|-----|------------|----------|
| **SDA(20), SCL(21)** | AM2315C + DS1307 | I2C - Sensori e RTC |
| **D2** | DHT11 | Sensore esterno |
| **2** | ILI9486 LCD_CS | Display SPI |
| **1** | ILI9486 LCD_RST | Reset display |
| **3** | ILI9486 LCD_RS | DisplayData/Command |
| **4** | ILI9486 LCD_WR | Display Write |
| **5** | ILI9486 LCD_RD | Display Read |
| **9-16** | ILI9486 D0-D7 | Display Data Bus 8-bit |
| **D11** | MOSI | SD CARD MOSI/XPT2046 MOSI   |
| **D4** | CS | SD CARD CS	 |
| **D12** | MISO | SD CARD MISO/XPT2046 MISO  |
| **D13** | SCK | SD CARD SCK/XPT2046 SCK  |
| **D6** | CS | XPT2046 CS	 |
| **D7** | IRQ | XPT2046 IRQ |
| **D8** | WS2812B 24bit | LED indicatori principali |
| **D9** | WS2812B 12bit | LED indicatori secondari |
| **D10** | Buzzer Passivo | Allarmi acustici (PWM/tone) |
| **D22** | Relè Frigorifero | Controllo frigorifero |
| **D23** | Relè Riscaldatore | Controllo riscaldatore |
| **D24** | Relè Deumidificatore | Controllo deumidificatore |
| **D25** | Relè Umidificatore | Controllo umidificatore |
| **D26** | Relè Ventola Immissione | Controllo ventola immissione |
| **D27** | Relè Ventola Estrazione | Controllo ventola estrazione |

#### Librerie Arduino Necessarie
```cpp
// Installare tramite Arduino IDE Library Manager:
#include <Wire.h>                    // I2C (inclusa)
#include <Adafruit_AM2315.h>         // Sensore AM2315C
#include <DHT.h>                     // Sensore DHT11
#include <RTClib.h>                  // RTC DS1307
#include <Adafruit_GFX.h>            // Grafica display
#include <Adafruit_ILI9486.h>        // Display ILI9486
#include <XPT2046_Touchscreen.h>     // Touch screen
#include <FastLED.h>                 // LED WS2812B
#include <EEPROM.h>                  // Memoria persistente
#include <SD.h>                      // SD Card
#include <SPI.h>                     // SPI (inclusa)
#include <avr/wdt.h>                 // Protezione watchdog
```

## ⏱️ Temporizzazioni Ottimizzate per Efficienza e Sicurezza

### **Temporizzazioni Operative**
- **Lettura sensori**: Ogni 30 secondi (monitoraggio continuo)
- **Aggiornamento display**: Ogni 15 secondi (interfaccia fluida)
- **Controllo frigorifero**: Ogni 8 minuti (sicurezza alimentare)
- **Controllo riscaldatore**: Ogni 5 minuti (processo critico)
- **Controllo deumidificatore**: Ogni 4 minuti (efficienza)
- **Controllo umidificatore**: Ogni 3 minuti (stabilità)
- **Controllo ventole**: Ogni 2 minuti (ricircolo aria)

### **Cicli Minimi Dispositivi (Protezione Meccanica)**
| Dispositivo      | Ciclo Minimo ON | Ciclo Minimo OFF | Isteresi |
|------------------|-----------------|------------------|----------|
| **Frigorifero**  | 5 minuti        | 3 minuti         | ±1.0°C   |
| **Riscaldatore** | 4 minuti        | 2 minuti         | ±0.8°C   |
| **Deumidificatore** | 10 minuti    | 5 minuti         | ±3.0%    |
| **Umidificatore** | 5 minuti      | 3 minuti         | ±2.5%    |
| **Ventole** | 1 minuto       | 30 secondi       | ±2.0°C/±5.0% |

## 🔍 Validazione Dati Sensori

### **Range di Validazione Specifici**
| Sensore | Temperatura | Umidità | Precisione | Note |
|---------|-------------|---------|------------|------|
| **AM2315C** | -40°C a +80°C | 0% a 100% | ±0.3°C, ±2% RH | Sensore interno, alta precisione |
| **DHT11** | 0°C a +50°C | 20% a 90% | ±2°C, ±5% RH | Sensore esterno, economico |

### **Comportamento in Caso di Dati Non Validi**
1. **Dati fuori range** → Impostazione NAN e avviso su display anomalia
2. **Controlli bloccati** → Nessuna azione attuatori con dati non validi
3. **Sistema stabile** → Mantenimento stato precedente o modalità emergenza
### **Messaggi di Errore Specifici**
- **AM2315C**: "ERRORE: Dati AM2315C fuori range (T: -40/+80°C, H: 0-100%)"
- **DHT11**: "ERRORE: Dati DHT11 fuori range (T: 0/+50°C, H: 20-90%)"
