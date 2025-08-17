# 📐 STAGIONINO - Specifiche Layout Nextion Professionale

## 🎯 **Pixel-Perfect Layout per Display 480x320**

### **📱 Griglia di Base:**
```
Display Nextion NX4832K035: 480 x 320 pixels
Aspect Ratio: 3:2 (Landscape)
Touch Resolution: Stessa del display
Color Depth: 65K colors (RGB565)
```

---

## 🏠 **PAGINA 0: DASHBOARD - Coordinate Precise**

### **📏 Struttura Layout:**
```
Header Area:           Y: 0-30     (30px height)
Controls Panel:        Y: 30-130   (100px height)  
Program Panel:         Y: 130-190  (60px height)
Actuators Panel:       Y: 190-250  (60px height)
Navigation Panel:      Y: 250-310  (60px height)
Status Bar:            Y: 310-320  (10px height)
```

### **🎛️ Componenti Nextion e Coordinate:**

#### **Header (Y: 0-30):**
```
t0: "STAGIONINO V1.2"          X:240  Y:5   (centrato)
t1: "Sistema Stagionatura..."  X:240  Y:18  (centrato)
```

#### **Layout Compatto Professionale (Y: 30-160) - STILE FOTO:**
```
Background: DARK_BLUE (#2c387e)

// === SEZIONE SINISTRA: TEMPERATURA ===
// Header temperatura (sfondo blu scuro)
t20: "Temperatura"        X:20   Y:35   Size:14px  Color:WHITE  Bold:YES  BgColor:BLUE
// Valore principale grande LCD (stile foto - giallo su viola scuro)
t21: "18.6"              X:20   Y:55   Size:40px  Color:YELLOW Bold:YES  BgColor:DARK_PURPLE LCD7Seg:YES

// Range boxes temperatura con indicatori circolari
b10: "16"  (Tmin)        X:20   Y:110  W:45 H:30  TouchPress:YES Border:DYNAMIC BgColor:DARK_PURPLE
t22: "Tmin"              X:22   Y:122  Size:8px   Color:WHITE
c10: Indicatore Tmin     X:50   Y:145  R:6       Color:DYNAMIC  // Pallino colorato

b11: "19"  (Tmax)        X:80   Y:110  W:45 H:30  TouchPress:YES Border:DYNAMIC BgColor:DARK_PURPLE  
t23: "Tmax"              X:82   Y:122  Size:8px   Color:WHITE
c11: Indicatore Tmax     X:110  Y:145  R:6       Color:DYNAMIC  // Pallino colorato

// === SEZIONE CENTRO: CONTROLLI ===
// Setpoint temperatura con sfondo cyan (stile foto)
t24: "Temp Pr."          X:170  Y:35   Size:10px  Color:WHITE
t25: "6.1"               X:170  Y:50   Size:28px  Color:WHITE   BgColor:CYAN Bold:YES LCD7Seg:YES

// Controlli frecce circolari (info button style)
b0:  "ⓘ"  (Info)         X:170  Y:80   Size:35x35 Color:WHITE   BgColor:GRAY BorderRadius:17
b1:  [🔺]  (Su)          X:170  Y:80   Size:20x15 Color:WHITE   BgColor:TRANSPARENT
b2:  [🔻]  (Giù)         X:170  Y:95   Size:20x15 Color:WHITE   BgColor:TRANSPARENT

// Icone funzioni in grid 2x2 (stile foto)
t26: "📁"               X:150  Y:120  Size:16px  Color:WHITE  BgColor:DARK_GRAY
t27: "🌊"               X:180  Y:120  Size:16px  Color:WHITE  BgColor:DARK_GRAY
t28: "🕐"               X:150  Y:140  Size:16px  Color:WHITE  BgColor:DARK_GRAY  
t29: "⏹"                X:180  Y:140  Size:16px  Color:WHITE  BgColor:DARK_GRAY

// === SEZIONE DESTRA: UMIDITÀ ===
// Header umidità (sfondo blu scuro)
t30: "Umidità"           X:300  Y:35   Size:14px  Color:WHITE  Bold:YES  BgColor:BLUE
// Valore principale grande LCD (stile foto - verde su viola scuro)  
t31: "75.2"             X:300  Y:55   Size:40px  Color:GREEN  Bold:YES  BgColor:DARK_PURPLE LCD7Seg:YES

// Range boxes umidità con indicatori circolari
b12: "50"  (Umin)        X:300  Y:110  W:45 H:30  TouchPress:YES Border:DYNAMIC BgColor:DARK_PURPLE
t32: "Umin"              X:302  Y:122  Size:8px   Color:WHITE
c12: Indicatore Umin     X:330  Y:145  R:6       Color:DYNAMIC  // Pallino colorato

b13: "75"  (Umax)        X:360  Y:110  W:45 H:30  TouchPress:YES Border:DYNAMIC BgColor:DARK_PURPLE
t33: "Umax"              X:362  Y:122  Size:8px   Color:WHITE  
c13: Indicatore Umax     X:390  Y:145  R:6       Color:DYNAMIC  // Pallino colorato

// === STATI DINAMICI RANGE BOXES ===
// NON_SELEZIONATO: Border:INVISIBLE, Touch:NO
// SELEZIONATO_TEMP: Border:BLUE(#2196F3) Width:3px, Pallino:BLUE  
// SELEZIONATO_UMID: Border:GREEN(#4CAF50) Width:3px, Pallino:GREEN
// EDITANDO: Border:YELLOW + BLINK, Pallino:YELLOW + BLINK

// === COMPORTAMENTO TOUCH ===
// 1. Click su range box → Selezione (bordo colorato + pallino colorato)
// 2. Click frecce [🔺][🔻] → Modifica valore selezionato (+1/-1)
// 3. Click altro range → Cambia selezione
// 4. Click vuoto → Deseleziona tutto (bordi invisibili)
```

#### **Attuatori e Controlli (Y: 170-250):**
```
// === ATTUATORI - ROW 1 (Y: 170-200) ===
c0: Cerchio Frigo     X:30   Y:175  R:15  Color:DYNAMIC  Touch:DYNAMIC
t40:"❄️"  (Icona)     X:25   Y:170  Size:20px Color:WHITE
t41:"FRIGO" (Label)   X:15   Y:195  Size:8px  Color:WHITE

c1: Cerchio Risc      X:100  Y:175  R:15  Color:DYNAMIC  Touch:DYNAMIC  
t42:"🔥"  (Icona)     X:95   Y:170  Size:20px Color:WHITE
t43:"RISC" (Label)    X:85   Y:195  Size:8px  Color:WHITE

c2: Cerchio Deum      X:170  Y:175  R:15  Color:DYNAMIC  Touch:DYNAMIC
t44:"💨"  (Icona)     X:165  Y:170  Size:20px Color:WHITE  
t45:"DEUM" (Label)    X:155  Y:195  Size:8px  Color:WHITE

c3: Cerchio Umid      X:240  Y:175  R:15  Color:DYNAMIC  Touch:DYNAMIC
t46:"💧"  (Icona)     X:235  Y:170  Size:20px Color:WHITE
t47:"UMID" (Label)    X:225  Y:195  Size:8px  Color:WHITE

c4: Cerchio Vent1     X:310  Y:175  R:15  Color:DYNAMIC  Touch:DYNAMIC
t48:"🌀"  (Icona)     X:305  Y:170  Size:20px Color:WHITE
t49:"VNT1" (Label)    X:295  Y:195  Size:8px  Color:WHITE

c5: Cerchio Vent2     X:380  Y:175  R:15  Color:DYNAMIC  Touch:DYNAMIC
t50:"🌀"  (Icona)     X:375  Y:170  Size:20px Color:WHITE
t51:"VNT2" (Label)    X:365  Y:195  Size:8px  Color:WHITE

// === STATI ATTUATORI ===
// OFF: Color:GRAY (#8410), Touch:NO (modalità programma)
// ON:  Color:BLUE/RED/ORANGE/GREEN, Touch:NO (modalità programma)
// MANUAL_OFF: Color:GRAY + Border:ORANGE, Touch:YES
// MANUAL_ON:  Color:VIVID + Border:ORANGE, Touch:YES
```

#### **Programma Attivo (Y: 210-280):**
```
// === PANNELLO PROGRAMMA COMPLETO ===
// Header programma
t60: "SALAME TRADIZIONALE"    X:20   Y:215  Size:14px  Color:WHITE Bold:YES
t61: "FASE 2/4"               X:350  Y:215  Size:12px  Color:CYAN

// Progress bar con percentuale
j0:  Progress Bar             X:20   Y:235  W:400 H:12 Color:PURPLE Value:0-100
t62: "67%"                    X:20   Y:250  Size:11px  Color:WHITE
t63: "8g 12h rimanenti"       X:300  Y:250  Size:11px  Color:GRAY

// Indicatore modalità
t64: "🎛️ MANUALE"            X:20   Y:265  Size:12px  Color:ORANGE  (se manual_mode)
t64: "🤖 AUTOMATICO"          X:20   Y:265  Size:12px  Color:GREEN   (se programma_attivo)

// === MODALITÀ PROGRAMMA VS MANUALE ===
// PROGRAMMA_ATTIVO:
//   - Mostra nome, fase, progress, tempo
//   - Attuatori: touch=NO, colori automatici
//   - Range: touch=NO, bordi invisibili
//   - Indicatore: "🤖 AUTOMATICO" verde

// MODALITÀ_MANUALE:  
//   - Nasconde progress bar (j0.vis=0)
//   - Testo: "🎛️ CONTROLLO MANUALE"
//   - Attuatori: touch=YES, bordi arancioni
//   - Range: touch=YES, pallini grigi disponibili
//   - Indicatore: "🎛️ MANUALE" arancione
```

#### **Footer Navigation (Y: 285-320):**
```
// === PULSANTI NAVIGAZIONE ===
b20: "SENSORI"      X:20   Y:290  W:80 H:25  Color:BLUE   Touch:YES
b21: "PROGRAMMI"    X:120  Y:290  W:80 H:25  Color:GREEN  Touch:YES  
b22: "SETTINGS"     X:220  Y:290  W:80 H:25  Color:PURPLE Touch:YES
b23: "EMERGENZA"    X:320  Y:290  W:80 H:25  Color:RED    Touch:YES
```

#### **Programma Attivo (Y: 130-190):**
```
t20: "SALAME TRADIZIONALE"     X:20   Y:140  Size:16px  Color:WHITE
t21: "FASE 2/4"                X:400  Y:140  Size:14px  Color:CYAN

j0:  Progress Bar              X:20   Y:160  W:440 H:15 Color:PURPLE
t22: "60%"                     X:20   Y:175  Size:12px  Color:WHITE
t23: "12g 4h rimanenti"        X:350  Y:175  Size:12px  Color:GRAY
```

#### **Attuatori (Y: 190-250):**
```
// Cerchi attuatori (diameter: 40px, spacing: 75px)
c0: FRIGO   X:30   Y:205  D:40  Color:BLUE/GRAY
c1: RISC    X:105  Y:205  D:40  Color:RED/GRAY
c2: DEUM    X:180  Y:205  D:40  Color:ORANGE/GRAY
c3: UMID    X:255  Y:205  D:40  Color:GREEN/GRAY
c4: VENT1   X:330  Y:205  D:40  Color:GREEN/GRAY
c5: VENT2   X:405  Y:205  D:40  Color:GREEN/GRAY

// Labels sotto cerchi
t30: "FRIGO"  X:25  Y:235  Size:10px  Color:WHITE
t31: "RISC"   X:100 Y:235  Size:10px  Color:WHITE
t32: "DEUM"   X:175 Y:235  Size:10px  Color:WHITE
t33: "UMID"   X:250 Y:235  Size:10px  Color:WHITE
t34: "VENT1"  X:325 Y:235  Size:10px  Color:WHITE
t35: "VENT2"  X:400 Y:235  Size:10px  Color:WHITE
```

#### **Navigazione (Y: 250-310):**
```
b10: "📊 SENSORI"     X:20   Y:260  W:140 H:40  Color:GREEN
b11: "📋 PROGRAMMI"   X:170  Y:260  W:140 H:40  Color:PURPLE  
b12: "⚙️ SETTINGS"   X:320  Y:260  W:140 H:40  Color:ORANGE
```

#### **Status Bar (Y: 310-320):**
```
t40: "🕐 14:35"       X:400  Y:312  Size:8px   Color:GRAY
t41: "📶"             X:460  Y:312  Size:8px   Color:GREEN
```

---

## 📊 **PAGINA 1: SENSORI - Coordinate Precise**

### **📏 Struttura Layout:**
```
Header:                Y: 0-30     (30px)
Sensore Interno:       Y: 30-140   (110px)
Sensore Esterno:       Y: 140-250  (110px)
Footer Controls:       Y: 250-320  (70px)
```

### **🎛️ Componenti Sensori:**

#### **Header:**
```
b99: "← INDIETRO"     X:20   Y:5   W:100 H:20  Color:BLUE
t50: "📊 DATI SENSORI" X:240 Y:8   Size:16px   Color:WHITE
t51: "🕐 14:35"       X:420  Y:8   Size:12px   Color:GRAY
```

#### **Sensore Interno (Y: 30-140):**
```
t52: "📊 SENSORE INTERNO (AM2315C)"  X:30 Y:40  Size:14px Color:WHITE

// Status line
t53: "Status: ✅ ONLINE"      X:40  Y:60  Size:12px  Color:GREEN
t54: "Precisione: ±0.1°C"     X:200 Y:60  Size:12px  Color:CYAN  
t55: "Uptime: 24h"            X:350 Y:60  Size:12px  Color:GRAY

// Value boxes (Y: 80-110)
// Box 1: Temperatura
t56: "25.5°C"       X:40  Y:85  W:100 H:20  Size:16px  Color:YELLOW
t57: "Temperatura"  X:45  Y:105 Size:10px   Color:WHITE

// Box 2: Umidità  
t58: "65.2%"        X:160 Y:85  W:100 H:20  Size:16px  Color:BLUE
t59: "Umidità"      X:170 Y:105 Size:10px   Color:WHITE

// Box 3: Trend
t60: "STABILE"      X:280 Y:85  W:100 H:20  Size:16px  Color:GREEN
t61: "Trend"        X:310 Y:105 Size:10px   Color:WHITE

// Statistics
t62: "Letture: 1,247 ✅ | Errori: 0 ✅ | Ultimo: 2s fa"  X:40 Y:125 Size:10px Color:GRAY
```

#### **Sensore Esterno (Y: 140-250):** 
```
// Stessa struttura del sensore interno, ma Y offset +110
t70-t80: [Componenti identici con Y+110]
```

#### **Footer (Y: 250-320):**
```
b13: "[24H]"           X:40  Y:260 W:60  H:30  Color:BLUE
b14: "[7D]"            X:110 Y:260 W:60  H:30  Color:BLUE  
b15: "[30D]"           X:180 Y:260 W:60  H:30  Color:BLUE
b16: "[📊 ESPORTA]"    X:300 Y:260 W:120 H:30  Color:GREEN
```

---

## ⚙️ **PAGINA 2: IMPOSTAZIONI - Coordinate Precise**

### **📏 Struttura Layout:**
```
Header:                Y: 0-30     (30px)
Modalità Sistema:      Y: 30-100   (70px)
Display/Interfaccia:   Y: 100-180  (80px)
Manutenzione:          Y: 180-270  (90px)
System Info:           Y: 270-320  (50px)
```

### **🎛️ Componenti Impostazioni:**

#### **Modalità Sistema (Y: 30-100):**
```
t100: "🎭 MODALITÀ SISTEMA"  X:30 Y:40  Size:14px  Color:WHITE

// Row 1 (Y: 55-75)
t101: "Demo Mode:"     X:40  Y:60  Size:12px  Color:WHITE
b20:  "[🔄 AUTO]"     X:120 Y:55  W:80  H:20  Color:PURPLE
t102: "Controllo:"     X:240 Y:60  Size:12px  Color:WHITE
b21:  "[📱 MANUALE]"  X:310 Y:55  W:80  H:20  Color:BLUE

// Row 2 (Y: 75-95)  
t103: "Target Temp: 15°C"  X:40  Y:80  Size:12px  Color:CYAN
b22:  "[🔺]"              X:160 Y:75  W:25  H:15  Color:BLUE
b23:  "[🔻]"              X:160 Y:85  W:25  H:15  Color:BLUE
t104: "Target Umid: 70%"   X:240 Y:80  Size:12px  Color:GREEN
b24:  "[🔺]"              X:360 Y:75  W:25  H:15  Color:GREEN
b25:  "[🔻]"              X:360 Y:85  W:25  H:15  Color:GREEN
```

#### **Display (Y: 100-180):**
```
t110: "💡 DISPLAY E INTERFACCIA"  X:30 Y:110 Size:14px Color:WHITE

// Controls row
t111: "Retroilluminazione:"  X:40  Y:130 Size:12px Color:WHITE
b30:  "[🟢 ON]"            X:180 Y:125 W:60  H:20 Color:GREEN
t112: "Auto-dim:"           X:280 Y:130 Size:12px Color:WHITE  
b31:  "[🟢 ON]"            X:340 Y:125 W:60  H:20 Color:GREEN

// Brightness slider
t113: "Luminosità:"         X:40  Y:150 Size:12px Color:WHITE
h0:   Slider                X:120 Y:145 W:200 H:20 Range:0-100
t114: "80%"                 X:330 Y:150 Size:12px Color:YELLOW

// Timeout buttons  
t115: "Timeout:"            X:40  Y:170 Size:12px Color:WHITE
b32:  "[30s]"              X:100 Y:165 W:40  H:20 Color:GRAY
b33:  "[60s]"              X:150 Y:165 W:40  H:20 Color:GRAY
b34:  "[120s]"             X:200 Y:165 W:40  H:20 Color:BLUE
b35:  "[MAI]"              X:250 Y:165 W:40  H:20 Color:GRAY
```

#### **Manutenzione (Y: 180-270):**
```
t120: "🔧 MANUTENZIONE E CALIBRAZIONE"  X:30 Y:190 Size:14px Color:WHITE

// Row 1
b40: "[🎯 CALIBRA TOUCH]"      X:40  Y:210 W:180 H:30 Color:BLUE
b41: "[🔍 DIAGNOSTICA SISTEMA]" X:240 Y:210 W:180 H:30 Color:CYAN

// Row 2  
b42: "[🌅 PROFILO GIORNO]"     X:40  Y:245 W:120 H:20 Color:ORANGE
b43: "[🌙 PROFILO NOTTE]"      X:180 Y:245 W:120 H:20 Color:PURPLE

// Row 3
b44: "[📊 RESET STATISTICHE]"  X:320 Y:245 W:140 H:20 Color:RED
```

#### **System Info (Y: 270-320):**
```
t130: "Sistema: RAM 75% | CPU 25% | Uptime: 2d 14h | Ver: 1.2"
      X:20 Y:280 Size:10px Color:GRAY
```

---

## 🎨 **Specifiche Colori RGB565**

### **Colori Primari:**
```cpp
#define COLOR_BLACK       0x0000    // Sfondo
#define COLOR_WHITE       0xFFFF    // Testo normale  
#define COLOR_GRAY        0x8410    // Testo secondario
#define COLOR_DARK_GRAY   0x4208    // Bordi, disabilitato
```

### **Colori Funzionali:**
```cpp
#define COLOR_BLUE        0x4D9F    // Frigorifero, pulsanti
#define COLOR_RED         0xF800    // Riscaldatore, errori
#define COLOR_GREEN       0x07E0    // Umidificatore, ventole, OK
#define COLOR_ORANGE      0xFD20    // Deumidificatore, warning
#define COLOR_YELLOW      0xFFE0    // Temperatura, demo
#define COLOR_CYAN        0x07FF    // Setpoint, info
#define COLOR_PURPLE      0x9C1F    // Programmi, modalità
```

### **Stati Attuatori:**
```cpp
// ON States
FRIGO_ON:      0x4D9F    // Blu freddo
RISC_ON:       0xF800    // Rosso caldo  
DEUM_ON:       0xFD20    // Arancione
UMID_ON:       0x07E0    // Verde acqua
VENT_ON:       0x07E0    // Verde aria

// OFF State (tutti)  
ACTUATOR_OFF:  0x4208    // Grigio scuro
```

---

## 📱 **Font Sizes e Typography**

### **Gerarchia Testi:**
```cpp
// Headers e Titoli
TITLE_SIZE:        24px   // Titoli pagina
SUBTITLE_SIZE:     16px   // Sottotitoli sezioni
SECTION_SIZE:      14px   // Titoli pannelli

// Valori e Dati
VALUE_LARGE:       28px   // Valori principali (temp, umid)
VALUE_MEDIUM:      18px   // Valori secondari  
VALUE_SMALL:       12px   // Labels e info

// Interfaccia
BUTTON_TEXT:       12px   // Testo pulsanti
STATUS_TEXT:       10px   // Barra stato, info sistema
CAPTION_TEXT:      8px    // Note e timestamp
```

### **Allineamenti:**
```cpp
HEADER_TEXT:       CENTER   // Titoli centrati
VALUE_TEXT:        CENTER   // Valori numerici centrati  
LABEL_TEXT:        LEFT     // Labels allineati a sinistra
BUTTON_TEXT:       CENTER   // Testo pulsanti centrato
STATUS_TEXT:       RIGHT    // Stato e timestamp a destra
```

Questo documento fornisce **coordinate pixel-perfect** per implementare l'interfaccia nel Nextion Editor con precisione professionale! 🎯
