# Stagionino - Installazione Librerie Arduino

## 📚 Librerie Necessarie

Per il corretto funzionamento del sistema Stagionino, è necessario installare le seguenti librerie tramite **Arduino IDE Library Manager**.

### 🔧 Come Installare le Librerie

1. Apri **Arduino IDE**
2. Vai su **Tools > Manage Libraries** (Strumenti > Gestione Librerie)
3. Cerca ogni libreria per nome e installa la **versione più recente**

---

## 📋 Lista Librerie da Installare

### 1. **Sensori e RTC**
```
- Adafruit AM2315       (v2.1.0+)  - Sensore temperatura/umidità interno
- DHT sensor library    (v1.4.4+)  - Sensore temperatura/umidità esterno  
- RTClib                (v2.1.1+)  - Real Time Clock DS1307
```

### 2. **Display e Touchscreen**
```
- Adafruit GFX Library  (v1.11.3+) - Libreria grafica base
- MCUFRIEND_kbv         (v2.9.9+)  - Display ILI9486 8-bit parallelo
- XPT2046_Touchscreen   (v1.4+)    - Touchscreen capacitivo
```

### 3. **LED e Controllo**
```
- FastLED               (v3.5.0+)  - Controllo LED WS2812B programmabili
```

### 4. **Librerie Incluse in Arduino**
Le seguenti librerie sono **già incluse** in Arduino IDE:
```
- Wire                  (I2C)
- SPI                   (SPI)  
- EEPROM                (Memoria persistente)
- SD                    (SD Card)
- avr/wdt.h            (Watchdog Timer)
```

---

## 🚨 Verifiche Post-Installazione

### Controllo Compilazione
Dopo l'installazione, verifica che il progetto compili correttamente:

1. Apri il file `Stagionino.ino`
2. Seleziona la board: **Arduino Mega 2560**
3. Clicca su **Verify/Compile** (Verifica)
4. Non devono esserci errori di librerie mancanti

### Messaggi di Errore Comuni

**ERRORE**: `fatal error: Adafruit_AM2315.h: No such file or directory`
**SOLUZIONE**: Installare la libreria "Adafruit AM2315"

**ERRORE**: `fatal error: MCUFRIEND_kbv.h: No such file or directory`  
**SOLUZIONE**: Installare la libreria "MCUFRIEND_kbv"

**ERRORE**: `fatal error: XPT2046_Touchscreen.h: No such file or directory`
**SOLUZIONE**: Installare la libreria "XPT2046_Touchscreen"

**ERRORE**: `fatal error: FastLED.h: No such file or directory`
**SOLUZIONE**: Installare la libreria "FastLED"

---

## 🔗 Link Alternativi

Se non riesci a trovare alcune librerie tramite Library Manager:

### Adafruit AM2315
```
URL: https://github.com/adafruit/Adafruit_AM2315
Installazione: Download ZIP > Sketch > Include Library > Add ZIP Library
```

### MCUFRIEND_kbv
```
URL: https://github.com/prenticedavid/MCUFRIEND_kbv  
Installazione: Library Manager cerca "MCUFRIEND_kbv"
```

### XPT2046_Touchscreen
```
URL: https://github.com/PaulStoffregen/XPT2046_Touchscreen
Installazione: Library Manager cerca "XPT2046_Touchscreen"
```

### FastLED
```
URL: https://github.com/FastLED/FastLED
Installazione: Library Manager cerca "FastLED"
```

---

## ⚡ Test Hardware

Una volta installate tutte le librerie e caricato il codice:

1. **Monitor Seriale**: Apri il monitor seriale (115200 baud)
2. **Sequenza Avvio**: Dovresti vedere tutti i componenti inizializzarsi
3. **LED Test**: Le strip LED dovrebbero fare una sequenza colorata
4. **Display**: Dovrebbe apparire "STAGIONINO V1.0" 

### Output Seriale Atteso
```
=== STAGIONINO V1.0 - AVVIO SISTEMA ===
Sistema di controllo ambientale per stagionatura salumi
Inizializzazione hardware...
-> Inizializzazione pin e componenti base
  Relè inizializzati - Stato: TUTTI OFF
-> Inizializzazione sensori
  -> Inizializzazione AM2315C (I2C)
     AM2315C: OK
  -> Inizializzazione DHT11
     DHT11: Inizializzato
-> Inizializzazione display
  -> Inizializzazione Display ILI9486
     Display ID: 0x9486
     Display: OK - Risoluzione: 480x320
  -> Inizializzazione Touchscreen XPT2046
     Touchscreen: OK
-> Inizializzazione RTC
  -> Inizializzazione RTC DS1307
     RTC: OK - Orario mantenuto
     Data/Ora: 15/12/2024 14:30:25
-> Inizializzazione LED
  -> Inizializzazione LED WS2812B
     Test sequenza LED...
     LED Strip 24bit: 24 LED - OK
     LED Strip 12bit: 12 LED - OK
-> Inizializzazione SD Card
  -> Inizializzazione SD Card
     Tentativo SD 1/3
     SD Card: Inizializzazione OK
     Tipo SD: SDHC
     Dimensione: 8192 MB
     Directory '/programs' creata
     Directory '/logs' creata
     Test scrittura: OK
-> Inizializzazione hardware completata
Abilitazione protezione watchdog...
=== SISTEMA STAGIONINO PRONTO ===
```

Se vedi questo output, l'hardware è correttamente inizializzato! 🎉 