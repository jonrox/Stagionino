# 🎨 STAGIONINO - Mockup Interfaccia Nextion Professionale

## 📱 Overview Design System

**Display:** Nextion NX4832K035 (480x320 pixels)  
**Style:** Moderno, minimalista, professionale  
**Colori:** Palette scura con accenti colorati  
**Typography:** Font chiari e leggibili  
**UX:** Massimo 3 tap per raggiungere qualsiasi funzione

---

## 🏠 **PAGINA 0: DASHBOARD COMPATTO PROFESSIONALE (480x320)**

### **Layout Stile Foto - 3 Sezioni Compatte:**

```
┌─────────────────────────────────────────────────────────────┐
│                   🏭 STAGIONINO V1.2                       │ 30px
│              Sistema Stagionatura Salumi                   │
├─────────────────────────────────────────────────────────────┤ 60px
│ ┌─ CONTROLLI AMBIENTALI ───────────────────────────────────┐ │
│ │                                                           │ │ 100px
│ │  📊 18.6°C    🎯 15°C [🔺]    💧 75.2%                   │ │
│ │      Temp      Target [🔻]      Umid                     │ │
│ │                                                           │ │
│ │  ┌───────┐  ┌───────┐  ┌───────┐  ┌───────┐  ┌───────┐ │ │
│ │  │  16   │  │  19   │  │  📊   │  │ 6.1   │  │  50   │ │ │
│ │  │ Tmin  │  │ Tmax  │  │ LOG   │  │ Prog  │  │ Umin  │ │ │
│ │  └───────┘  └───────┘  └───────┘  └───────┘  └───────┘ │ │
│ └─────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤ 170px
│ ┌─ 🎯 PROGRAMMA ATTIVO ─────────────────────────────────────┐ │
│ │                                                           │ │ 60px
│ │   SALAME TRADIZIONALE                    FASE 2/4        │ │
│ │   ████████████████▒▒▒▒▒▒▒▒▒▒ 60%    12g 4h rimanenti   │ │
│ │                                                           │ │
│ └─────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤ 240px
│ ┌─ 🎛️ ATTUATORI ─────────────────────────────────────────┐ │
│ │                                                           │ │ 60px
│ │   ❄️      🔥      💨      💧      🌀      🌀            │ │
│ │  FRIGO   RISC    DEUM    UMID   VENT1   VENT2          │ │
│ │   🔵     ⚫      ⚫      🟢     🟢     ⚫             │ │
│ │                                                           │ │
│ └─────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤ 310px
│ [📊 SENSORI]    [📋 PROGRAMMI]    [⚙️ SETTINGS]  🕐14:35 📶│ 10px
└─────────────────────────────────────────────────────────────┘
```

### **Specifiche Tecniche Dashboard Professionale:**

#### **🌡️ Controlli Ambientali (Stile Industriale):**
- **Temperatura Attuale**: 18.6°C (Font 28px, giallo/verde)
- **Setpoint Target**: 15°C (Font 24px, azzurro, modificabile)
- **Pulsanti ±**: [🔺][🔻] per regolare setpoint
- **Umidità**: 75.2% (Font 28px, verde)
- **Range Min/Max**: Tmin 16°C, Tmax 19°C, Umin 50%, Umax 75%
- **Progress Programma**: 6.1 (valore corrente fase)

#### **📊 Programma Attivo:**
**Quando programma è attivo:**
- **Nome**: "SALAME TRADIZIONALE" (Font 16px, bianco)
- **Fase**: "FASE 2/4" (Font 14px, azzurro)  
- **Progress Bar**: Barra colorata 60% completamento
- **Tempo**: "12g 4h rimanenti" (Font 12px, grigio)

**Quando nessun programma attivo (MODALITÀ MANUALE):**
- **Testo principale**: "🎛️ CONTROLLO MANUALE" (Font 16px, arancione)
- **Sottotitolo**: "👤 Utente controlla tutti gli attuatori" (Font 12px, grigio)
- **Suggerimento**: "⚙️ Tap PROGRAMMI per avviare automatico" (Font 10px, ciano)
- **Status Override**: "🔄 Override: Attivo" (Font 10px, arancione)
- **Progress Bar**: Nascosta completamente
- **Tempo rimanente**: Sostituito con "Controllo libero"

#### **🎛️ Attuatori con Feedback Visivo:**

**In MODALITÀ PROGRAMMA (automatica):**
- **Stato OFF**: ⚫ (Grigio scuro #404040)
- **Stato ON**: 🔵 (Blu), 🟢 (Verde), 🔴 (Rosso), 🟠 (Arancione)  
- **Touch**: Disabilitato (controllo automatico)
- **Indicatore**: Nessun bordo

**In MODALITÀ MANUALE (controllo utente):**
- **Stato OFF**: ⚫ (Grigio scuro) + **bordo arancione** 
- **Stato ON**: 🔵🟢🔴🟠 (Colori vivaci) + **bordo arancione**
- **Touch**: Abilitato (tap per toggle ON/OFF)
- **Indicatore**: Bordo arancione lampeggiante
- **Feedback**: Animazione press quando toccato

**CONTROLLI RANGE INTERATTIVI (modalità manuale):**
- **Range boxes**: Tmin, Tmax, Umin, Umax → **Touch abilitato**
- **Selezione**: Bordo **BLU** (temperatura) o **VERDE** (umidità)
- **Non selezionato**: Bordo **GRIGIO** ma toccabile
- **Frecce [🔺][🔻]**: Controllano il range selezionato
- **Modalità programma**: Range boxes **non toccabili**, bordi invisibili

**Componenti comuni:**
- **Icone**: ❄️ 🔥 💨 💧 🌀 🌀 (24px)
- **Labels**: FRIGO, RISC, DEUM, UMID, VENT1, VENT2

#### **🎨 Colori Stati Attuatori:**
```
FRIGO (ON):    🔵 #2196F3 (Blu freddo)
RISC (ON):     🔴 #F44336 (Rosso caldo)  
DEUM (ON):     🟠 #FF9800 (Arancione)
UMID (ON):     🟢 #4CAF50 (Verde acqua)
VENT1/2 (ON):  🟢 #4CAF50 (Verde aria)
TUTTI (OFF):   ⚫ #404040 (Grigio scuro)
```

---

## 📊 **PAGINA 1: DATI SENSORI DETTAGLIATI (480x320)**

### **Layout Professionale con Cards e Metriche:**

```
┌─────────────────────────────────────────────────────────────┐
│ [← INDIETRO]              📊 DATI SENSORI          🕐 14:35 │ 30px
├─────────────────────────────────────────────────────────────┤
│ ┌─ 📊 SENSORE INTERNO (AM2315C) ─────────────────────────┐  │ 60px
│ │                                                         │  │
│ │  Status: ✅ ONLINE    Precisione: ±0.1°C    Uptime: 24h │  │ 140px
│ │                                                         │  │
│ │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐    │  │
│ │  │    25.5°C   │  │    65.2%    │  │   STABILE   │    │  │
│ │  │ Temperatura │  │   Umidità   │  │    Trend    │    │  │
│ │  └─────────────┘  └─────────────┘  └─────────────┘    │  │
│ │                                                         │  │
│ │  Letture: 1,247 ✅  |  Errori: 0 ✅  |  Ultimo: 2s fa  │  │
│ └─────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤ 200px
│ ┌─ 🌡️ SENSORE ESTERNO (DHT11) ──────────────────────────┐  │
│ │                                                         │  │ 240px
│ │  Status: ✅ ONLINE    Precisione: ±2°C      Uptime: 24h │  │
│ │                                                         │  │
│ │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐    │  │
│ │  │    22.1°C   │  │    55.0%    │  │   STABILE   │    │  │
│ │  │ Temperatura │  │   Umidità   │  │    Trend    │    │  │
│ │  └─────────────┘  └─────────────┘  └─────────────┘    │  │
│ │                                                         │  │
│ │  Letture: 1,203 ✅  |  Errori: 3 ⚠️  |  Ultimo: 2s fa  │  │
│ └─────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤ 290px
│ 📈 TREND GRAFICI: [24H] [7D] [30D]    [📊 ESPORTA DATI]   │ 20px
└─────────────────────────────────────────────────────────────┘ 310px
```

### **Caratteristiche:**
- **Header**: Breadcrumb navigation con icone
- **Cards**: Design a pannelli con informazioni strutturate
- **Status**: Indicatori colorati per stato sensori
- **Grafici**: Trend visivo delle ultime letture
- **Metriche**: Contatori errori e performance

---

## ⚙️ **PAGINA 2: IMPOSTAZIONI SISTEMA (480x320)**

### **Layout Professionale con Pannelli di Controllo:**

```
┌─────────────────────────────────────────────────────────────┐
│ [← INDIETRO]             ⚙️ IMPOSTAZIONI            🕐 14:35 │ 30px
├─────────────────────────────────────────────────────────────┤
│ ┌─ 🎭 MODALITÀ SISTEMA ──────────────────────────────────┐  │ 60px
│ │                                                         │  │
│ │  Demo Mode:    [🔄 AUTO]   Controllo:  [📱 MANUALE]   │  │ 100px
│ │  └─ Dati simulati          └─ Utente decide           │  │
│ │                                                         │  │
│ │  Target Temp: 15°C [🔺🔻]   Target Umid: 70% [🔺🔻]   │  │
│ └─────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤ 140px
│ ┌─ 💡 DISPLAY E INTERFACCIA ─────────────────────────────┐  │
│ │                                                         │  │ 180px
│ │  Retroilluminazione:  [🟢 ON]    Auto-dim:  [🟢 ON]   │  │
│ │                                                         │  │
│ │  Luminosità:  ████████████▒▒▒▒ 80%                    │  │
│ │                                                         │  │
│ │  Timeout:     [30s] [60s] [120s] [MAI]                 │  │
│ └─────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤ 220px
│ ┌─ 🔧 MANUTENZIONE E CALIBRAZIONE ───────────────────────┐  │
│ │                                                         │  │ 280px
│ │  [🎯 CALIBRA TOUCH]    [🔍 DIAGNOSTICA SISTEMA]       │  │
│ │                                                         │  │
│ │  [🌅 PROFILO GIORNO]   [🌙 PROFILO NOTTE]             │  │
│ │                                                         │  │
│ │  [📊 RESET STATISTICHE] [🔄 RESTART SISTEMA]          │  │
│ └─────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤ 300px
│ Sistema: RAM 75% | CPU 25% | Uptime: 2d 14h | Ver: 1.2    │ 10px
└─────────────────────────────────────────────────────────────┘ 310px
```

### **Funzionalità:**
- **Toggle Switches**: ON/OFF visuali e intuitivi  
- **Sliders**: Controllo luminosità con feedback visivo
- **Action Buttons**: Pulsanti per calibrazione e test
- **Info Tooltips**: Spiegazioni contestuali

---

## 📋 **PAGINA 3: GESTIONE PROGRAMMI (480x320)**

### **Layout Professionale con Status e Libreria:**

```
┌─────────────────────────────────────────────────────────────┐
│ [← INDIETRO]          📋 PROGRAMMI STAGIONATURA     🕐 14:35 │ 30px
├─────────────────────────────────────────────────────────────┤
│ ┌─ 🎯 PROGRAMMA ATTIVO ───────────────────────────────────┐  │ 60px
│ │                                                         │  │
│ │ SALAME TRADIZIONALE          Status: ▶️ RUNNING        │  │ 120px
│ │ FASE 2/4 | 60% ████████████▒▒▒▒▒▒▒▒ | 12g 4h left    │  │
│ │                                                         │  │
│ │ Target: 🌡️15°C(±1) 💧75%(±5) | [⏸️PAUSA] [⏹️STOP]   │  │
│ └─────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤ 160px
│ ┌─ 📚 LIBRERIA PROGRAMMI ─────────────────────────────────┐  │
│ │                                                         │  │ 200px
│ │ 🥓 PANCETTA      21gg  [Ready]            [▶️ AVVIA]   │  │
│ │ 🍖 LONZA         45gg  [Ready]            [▶️ AVVIA]   │  │ 240px
│ │ 🔴 BRESAOLA      60gg  [Ready]            [▶️ AVVIA]   │  │
│ │ ➕ CUSTOM        --    [Setup Needed]     [⚙️ CREA]   │  │ 280px
│ │                                                         │  │
│ └─────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤ 300px
│ [📊 STORICO] [📋 NUOVO PROGRAMMA] [⚙️ BACKUP/RESTORE]      │ 10px
└─────────────────────────────────────────────────────────────┘ 310px
```

### **Features:**
- **Progress Bar**: Visione chiara avanzamento programma
- **Program Cards**: Liste con icone distintive per tipo salume
- **Quick Actions**: Pausa, stop, dettagli immediati
- **Templates**: Programmi precaricati per prodotti tipici

---

## 🚨 **PAGINA 4: EMERGENZA (480x320)** (Solo se attiva)

### **Layout Critico con Alert e Azioni:**

```
┌─────────────────────────────────────────────────────────────┐
│                   🚨 SISTEMA IN EMERGENZA 🚨               │ 30px
├─────────────────────────────────────────────────────────────┤
│ ┌─ ⚠️ ALLARME CRITICO ────────────────────────────────────┐  │ 60px
│ │                                                         │  │
│ │ TEMPERATURA FUORI CONTROLLO    ⏰ Attivo da: 00:03:47  │  │ 120px
│ │                                                         │  │
│ │ 🌡️ Rilevato: 28.5°C    📊 Limite: 25°C    🔺 +3.5°C   │  │
│ │                                                         │  │
│ │ Rischio: 🔴 ALTO | Trend: 📈 CRESCENTE | Auto: ✅ ON   │  │
│ └─────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤ 160px
│ ┌─ 🤖 AZIONI AUTOMATICHE ─────────────────────────────────┐  │
│ │                                                         │  │ 200px
│ │ ✅ Frigorifero: ATTIVATO MAX     ⏰ 14:31:15           │  │
│ │ ✅ Riscaldatore: SPENTO          ⏰ 14:31:15           │  │ 240px
│ │ ✅ Ventole: ATTIVATE MAX         ⏰ 14:31:16           │  │
│ │ ✅ Notifica: INVIATA             ⏰ 14:32:00           │  │
│ │                                                         │  │
│ └─────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤ 280px
│ [🏠 DASHBOARD] [🔇 MUTE 5MIN] [🔧 RECOVERY] [📞 SUPPORTO]  │ 20px
│ 💡 Verificare: porta camera, guarnizioni, ventilazione     │ 10px
└─────────────────────────────────────────────────────────────┘ 310px
```

### **Emergency UX:**
- **Alert Design**: Colori rossi, testo grande, informazioni chiare
- **Action Log**: Lista azioni automatiche intraprese
- **Quick Controls**: Mute allarmi, recovery, ritorno dashboard
- **Help Tips**: Suggerimenti contestuali per risoluzione

---

## 🔍 **PAGINA 5: DIAGNOSTICA SISTEMA (480x320)**

### **Layout Tecnico con Metriche Real-time:**

```
┌─────────────────────────────────────────────────────────────┐
│ [← INDIETRO]          🔍 DIAGNOSTICA SISTEMA        🕐 14:35 │ 30px
├─────────────────────────────────────────────────────────────┤
│ ┌─ 🖥️ STATUS HARDWARE ────────────────────────────────────┐  │ 60px
│ │                                                         │  │
│ │ 🌡️ AM2315C: ✅ 1,247 ok  🌡️ DHT11: ✅ 1,203 ok       │  │ 110px
│ │ 💾 SD Card: ✅ 2.1GB     🕐 RTC: ✅ 14:35:22         │  │
│ │ 📡 Nextion: ✅ 9600 baud  💡 Display: ✅ 80%         │  │
│ │                                                         │  │
│ └─────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤ 150px
│ ┌─ 📊 PERFORMANCE REAL-TIME ──────────────────────────────┐  │
│ │                                                         │  │ 190px
│ │ 🧠 RAM: 2,156 bytes  ████████████▒▒▒▒ 75% FREE       │  │
│ │ ⚡ CPU: ████▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ 25% LOAD               │  │ 230px
│ │ 🔄 Loop: 45ms (OK)    🕐 Uptime: 2d 14h 35m          │  │
│ │                                                         │  │
│ └─────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤ 270px
│ [🧪 TEST SENSORI] [🔧 TEST ATTUATORI] [📊 RESET STATS]     │ 30px
│ Ver: 1.2 | Build: 2024.01 | Temp MCU: 45°C | Free: 2156b  │ 10px
└─────────────────────────────────────────────────────────────┘ 310px
```

### **Diagnostic Features:**
- **System Health**: Status colorati per ogni componente
- **Performance Metrics**: RAM, CPU, timing con barre progress
- **Live Data**: Aggiornamento in tempo reale
- **Test Tools**: Pulsanti per test manuali componenti

---

## 🎨 **DESIGN SYSTEM**

### **🎨 Palette Colori:**
```
Primari:
- Background: #1E1E1E (Grigio scuro)
- Cards: #2D2D2D (Grigio medio)  
- Text: #FFFFFF (Bianco)
- Borders: #404040 (Grigio bordi)

Funzionali:
- Success: #4CAF50 (Verde)
- Warning: #FF9800 (Arancione)
- Error: #F44336 (Rosso)
- Info: #2196F3 (Blu)
- Accent: #9C27B0 (Viola)
```

### **📏 Layout Grid:**
```
Margin: 10px
Card Padding: 15px
Button Height: 50px
Icon Size: 24px
Font Sizes: Title(24px), Body(16px), Caption(12px)
```

### **🔤 Typography:**
```
- Titoli: Font Bold, 24px, Centered
- Labels: Font Regular, 16px, Left-aligned  
- Values: Font Bold, 18px, Right-aligned
- Captions: Font Light, 12px, Gray
```

### **🎯 UX Principles:**
- **Tap Targets**: Minimo 44x44px
- **Contrast**: WCAG AA compliant
- **Feedback**: Visual feedback per ogni interazione
- **Navigation**: Max 3 tap per qualsiasi funzione
- **Consistency**: Stessi pattern in tutte le pagine

---

## 🛠️ **Implementazione Nextion**

### **Componenti Dashboard Professionale:**

#### **Controlli Ambientali:**
- **t0**: Temperatura attuale (18.6)
- **t1**: Setpoint temperatura (15)  
- **t2**: Umidità attuale (75.2)
- **t3**: Tmin (16), **t4**: Tmax (19)
- **t5**: Umin (50), **t6**: Umax (75)
- **t7**: Progress programma (6.1)
- **b0**: Pulsante SU temperatura [🔺]
- **b1**: Pulsante GIÙ temperatura [🔻]

#### **Programma Attivo:**
- **t10**: Nome programma ("SALAME TRADIZIONALE")
- **t11**: Fase corrente ("FASE 2/4")
- **j0**: Progress bar (0-100%)
- **t12**: Tempo rimanente ("12g 4h rimanenti")

#### **Attuatori Dinamici:**
- **c0**: Cerchio FRIGO (grigio ⚫ / blu 🔵)
- **c1**: Cerchio RISC (grigio ⚫ / rosso 🔴)
- **c2**: Cerchio DEUM (grigio ⚫ / arancione 🟠)
- **c3**: Cerchio UMID (grigio ⚫ / verde 🟢)
- **c4**: Cerchio VENT1 (grigio ⚫ / verde 🟢)
- **c5**: Cerchio VENT2 (grigio ⚫ / verde 🟢)

### **Codice Aggiornamento Arduino:**
```cpp
// Aggiornamento controlli ambientali
nextion.setText("t0", String(temp_attuale, 1));        // 18.6
nextion.setText("t1", String(temp_setpoint));          // 15
nextion.setText("t2", String(umidita_attuale, 1));     // 75.2

// Aggiornamento programma
if (programma_attivo) {
    nextion.setText("t10", nome_programma);
    nextion.setText("t11", "FASE " + String(fase) + "/" + String(fasi_totali));
    nextion.setValue("j0", percentuale_completamento);
    nextion.setText("t12", tempo_rimanente);
} else {
    nextion.setText("t10", "CONTROLLO MANUALE");
    nextion.setText("t11", "Tap PROGRAMMI per avviare");
    nextion.setValue("j0", 0);
    nextion.setText("t12", "");
}

// Aggiornamento attuatori (cambio colore dinamico)
nextion.setBackgroundColor("c0", frigorifero_on ? 0x2196F3 : 0x404040);  // Blu/Grigio
nextion.setBackgroundColor("c1", riscaldatore_on ? 0xF44336 : 0x404040);  // Rosso/Grigio
nextion.setBackgroundColor("c2", deumidificatore_on ? 0xFF9800 : 0x404040); // Arancione/Grigio
nextion.setBackgroundColor("c3", umidificatore_on ? 0x4CAF50 : 0x404040);  // Verde/Grigio
nextion.setBackgroundColor("c4", ventola1_on ? 0x4CAF50 : 0x404040);      // Verde/Grigio
nextion.setBackgroundColor("c5", ventola2_on ? 0x4CAF50 : 0x404040);      // Verde/Grigio
```

### **Eventi Touch:**
```cpp
// Pagina 0 - Dashboard
page0.b0.click → Incrementa setpoint temperatura (+0.5°C)
page0.b1.click → Decrementa setpoint temperatura (-0.5°C)
page0.b10.click → Vai a pagina sensori (page 1)
page0.b11.click → Vai a pagina programmi (page 3)  
page0.b12.click → Vai a pagina impostazioni (page 2)

// Touch attuatori per override manuale (opzionale)
page0.c0.click → Toggle manuale frigorifero
page0.c1.click → Toggle manuale riscaldatore
// etc...
```

Questo design crea un'interfaccia **moderna, professionale e intuitiva** che rende il controllo del sistema di stagionatura semplice anche per utenti non tecnici! 🎉
