# 🖥️ Stagionino - Progetto Nextion Display NX4832K035

## 📋 Specifiche Progetto Nextion

### **Hardware:**
- **Display:** NX4832K035 (3.5" 480x320 pixels)
- **Comunicazione:** UART (9600 baud)
- **Pin Arduino:** Serial1 (TX1=18, RX1=19) o SoftwareSerial

### **Struttura Pagine:**

#### **Pagina 0: Dashboard Principale**
```
Elementi:
- t0: Titolo "STAGIONINO" (font size 2, white)
- t1: Sottotitolo "Sistema Stagionatura Salumi" (font size 1, white)
- t2: Indicatore modalità DEMO (giallo quando attivo)
- t3: Temperatura interna (verde/rosso)
- t4: Umidità interna (verde/rosso)
- t5: Temperatura esterna (ciano/giallo)
- t6: Umidità esterna (ciano/giallo)
- t7: Timestamp ultimo aggiornamento (grigio)
- c0-c5: Icone attuatori circolari (frigorifero, riscaldatore, deumidificatore, umidificatore, ventole)
- b0: Pulsante "SENSORI" (blu)
- b1: Pulsante "PROGRAMMI" (verde)
- b2: Pulsante "SETTINGS" (arancione)

Colori:
- Sfondo: Nero (0)
- Testo normale: Bianco (65535)
- Demo: Giallo (65504)
- Sensore interno: Verde (2016)
- Sensore esterno: Ciano (2047)
- Errore: Rosso (63488)
```

#### **Pagina 1: Dati Sensori Dettagliati**
```
Elementi:
- t0: Titolo "DATI SENSORI"
- t1: "=== SENSORE INTERNO (AM2315C) ==="
- t2: Temperatura interna dettagliata
- t3: Umidità interna dettagliata
- t4: Contatore errori interno
- t5: "=== SENSORE ESTERNO (DHT11) ==="
- t6: Temperatura esterna dettagliata
- t7: Umidità esterna dettagliata
- t8: Contatore errori esterno
- t9: Timestamp ultimo aggiornamento
- b0: Pulsante "INDIETRO"
```

#### **Pagina 2: Impostazioni Sistema**
```
Elementi:
- t0: Titolo "IMPOSTAZIONI"
- t1: "Modalità Demo:"
- s0: Switch Demo ON/OFF
- t2: "Retroilluminazione:"
- s1: Switch Backlight ON/OFF
- h0: Slider luminosità (0-100)
- t3: Valore luminosità %
- b0: Pulsante "CALIBRA TOUCH"
- b1: Pulsante "DIAGNOSTICA"
- b2: Pulsante "PROFILO GIORNO"
- b3: Pulsante "PROFILO NOTTE"
- b4: Pulsante "INDIETRO"
```

#### **Pagina 3: Gestione Programmi**
```
Elementi:
- t0: Titolo "PROGRAMMI STAGIONATURA"
- t1: "Programma corrente: Nessuno"
- t2: Lista programmi disponibili
- b0: Pulsante "AVVIA PROGRAMMA"
- b1: Pulsante "STOP PROGRAMMA"
- b2: Pulsante "NUOVO PROGRAMMA"
- b3: Pulsante "INDIETRO"
```

#### **Pagina 4: Emergenza**
```
Elementi:
- t0: Titolo "EMERGENZA" (rosso lampeggiante)
- t1: Descrizione errore
- t2: Timer emergenza
- t3: Azioni intraprese
- b0: Pulsante "DASHBOARD"
- b1: Pulsante "MUTE ALLARMI"
- b2: Pulsante "RECOVERY"
```

#### **Pagina 5: Diagnostica**
```
Elementi:
- t0: Titolo "TEST DIAGNOSTICI"
- t1: Stato sensori
- t2: Stato attuatori
- t3: Stato SD card
- t4: Memoria libera
- t5: Uptime sistema
- b0: Pulsante "TEST SENSORI"
- b1: Pulsante "TEST ATTUATORI"
- b2: Pulsante "INDIETRO"
```

### **Protocollo Comunicazione UART:**

#### **Comandi Arduino → Nextion:**
```
// Aggiornamento valori sensori
page0.t3.txt="25.5"    // Temperatura interna
page0.t4.txt="65.2"    // Umidità interna
page0.t5.txt="23.1"    // Temperatura esterna
page0.t6.txt="NON DISP" // Sensore esterno non disponibile

// Indicatori modalità
page0.t2.txt="MODALITA' DEMO"
page0.t2.pco=65504     // Colore giallo

// Stato attuatori (icone circolari)
page0.c0.pco=31        // Frigorifero: blu quando attivo
page0.c1.pco=63488     // Riscaldatore: rosso quando attivo

// Cambio pagina
page 1                 // Vai a pagina sensori
```

#### **Eventi Nextion → Arduino:**
```
// Touch eventi (via seriale)
0x65 0x00 0x01 0x01 0xFF 0xFF 0xFF  // Pagina 0, Pulsante b0 (SENSORI)
0x65 0x00 0x02 0x01 0xFF 0xFF 0xFF  // Pagina 0, Pulsante b1 (PROGRAMMI)
0x65 0x02 0x00 0x01 0xFF 0xFF 0xFF  // Pagina 2, Switch s0 (DEMO)
```

### **Timer e Aggiornamenti:**
- **Sensori:** Ogni 2 secondi
- **Attuatori:** Ogni 1 secondo
- **Timestamp:** Ogni 10 secondi
- **Modalità demo:** Su cambio stato

### **File da Creare:**
1. `Stagionino.HMI` - Progetto principale Nextion
2. `Stagionino.tft` - File compilato per upload su display
3. `nextion_protocol.h` - Header protocollo comunicazione Arduino
4. `nextion_communication.cpp` - Implementazione comunicazione
