# 🎨 Legenda Colori LED Ring - Stagionino V1.0

## 🎯 Sistema LED Adattivo per Modalità

Il sistema LED ring cambia comportamento in base alla modalità operativa per fornire sempre le informazioni più utili.

---

## 🔧 MODALITÀ MANUALE

### 🕐 Ring 24 (Esterno): ORE DI FUNZIONAMENTO
**Concetto:** Conto in **avanti** - Ogni LED = 1 ora di funzionamento

#### 🌈 Colori Ore Funzionamento
| Tempo | Colore | Significato |
|-------|--------|-------------|
| **0-6h** | 🟢 **Verde** | Funzionamento recente |
| **6-12h** | 🟡 **Giallo** | Funzionamento normale |
| **12-18h** | 🟠 **Arancione** | Funzionamento prolungato |
| **18-24h** | 🔴 **Rosso** | Funzionamento molto lungo |

#### ⚡ Effetti Speciali
- **LED Lampeggiante Bianco**: Ora corrente in corso
- **LED Spenti**: Grigio molto scuro per contrasto
- **Max 24h**: Dopo 24 ore il ring si riempie tutto rosso

---

### 📱 Ring 12 (Interno): ATTUATORI ATTIVI + SISTEMA
**Concetto:** Legenda fissa - Ogni LED = 1 componente specifico

#### 🎛️ Attuatori (LED 0-5)
| LED | Attuatore | Colore Attivo | Colore Spento | Protezione |
|-----|-----------|---------------|---------------|------------|
| **0** | 🔵 **Frigorifero** | Blu intenso | Blu scuro | Lampeggia |
| **1** | 🔴 **Riscaldatore** | Rosso intenso | Rosso scuro | Lampeggia |
| **2** | 🟠 **Deumidificatore** | Arancione intenso | Arancione scuro | Lampeggia |
| **3** | 🔷 **Umidificatore** | Ciano intenso | Ciano scuro | Lampeggia |
| **4** | 🟢 **Ventola IN** | Verde intenso | Verde scuro | Lampeggia |
| **5** | 🟢 **Ventola OUT** | Lime intenso | Lime scuro | Lampeggia |

#### 🖥️ Sistema (LED 6-11)
| LED | Componente | Verde (OK) | Rosso (Errore) | Giallo (Warning) |
|-----|------------|------------|----------------|------------------|
| **6** | **Sensore Interno** | ✅ Funziona | ❌ Offline | - |
| **7** | **Sensore Esterno** | ✅ Funziona | ❌ Offline | ⚠️ Inaccurato |
| **8** | **RTC (Orologio)** | ✅ Funziona | ❌ Offline | - |
| **9** | **SD Card** | ✅ Funziona | ❌ Offline | - |
| **10** | **Heartbeat** | ⚪ Pulse bianco ogni 2s | - | - |
| **11** | **Heartbeat** | ⚪ Pulse bianco ogni 2s | - | - |

---

## 🤖 MODALITÀ AUTOMATICA (Programma Attivo)

### 🕐 Ring 24 (Esterno): ORE RIMANENTI
**Concetto:** Conto alla **rovescia** - Ogni LED = 1 ora rimanente

#### 🌈 Colori Tempo Rimanente
| Tempo | Colore | Significato |
|-------|--------|-------------|
| **>12h** | 🟢 **Verde** | Molto tempo rimanente |
| **6-12h** | 🟡 **Giallo** | Tempo normale |
| **2-6h** | 🟠 **Arancione** | Poco tempo |
| **<2h** | 🔴 **Rosso** | ⚠️ Scadenza vicina |

#### ⚡ Effetti Speciali
- **LED Lampeggiante Bianco**: Ora corrente in scadenza
- **Fasi Infinite**: 4 LED viola rotanti

---

### 📋 Ring 12 (Interno): FASI PROGRAMMA
**Concetto:** Progress fasi - Ogni LED = 1 fase del programma

#### 🎯 Colori Fasi
| Stato Fase | Colore | Significato |
|------------|--------|-------------|
| **Completata** | 🟢 **Verde** | Fase finita con successo |
| **Corrente** | 🔵 **Blu** / ⚪ **Bianco** | Fase attiva (lampeggia) |
| **Future** | ⚫ **Grigio** | Fasi da completare |
| **Infinita** | 💜 **Viola** | Breathing per fase finale |

#### 🚨 Indicatori Speciali
- **LED Rosso**: Programma con più di 12 fasi (ultimo LED)
- **Breathing Viola**: Fase di invecchiamento infinita

---

## 🟡 MODALITÀ NORMALE (Nessun Programma)

### Ring 24 + 12: STATUS SISTEMA COMPLETO
Visualizzazione standard con:
- **Ring 24**: Status attuatori gruppati (6 gruppi da 4 LED)
- **Ring 12**: Indicatori sistema completi

---

## 🌈 SEQUENZA AVVIO (Rainbow)

### ✨ Animazione Startup
**Durata:** ~3 secondi  
**Effetto:** Wave colorata che attraversa entrambi i ring
- **Ring 24**: Wave principale con 3 colori (Rosso → Verde → Blu)
- **Ring 12**: Wave sincrona con colori complementari
- **Finale**: Flash bianco su tutti i LED

#### 🎨 Pattern Rainbow
1. **Wave 1**: Rosso (Hue 0°) con scia sfumata
2. **Wave 2**: Verde (Hue 85°) con scia sfumata  
3. **Wave 3**: Blu (Hue 170°) con scia sfumata
4. **Flash**: Bianco intenso 200ms
5. **Spegnimento**: Tutti LED neri

---

## 🎮 Esempi Pratici d'Uso

### 📋 Esempio 1: Modalità Manuale - 8 Ore di Funzionamento

**Ring 24 (Esterno):**
```
🟡🟡🟡🟡🟡🟡🟡🟡⚪⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫
↑                 ↑
8 LED gialli      Ora 9 lampeggia bianco
```

**Ring 12 (Interno):**
```
🔵🔴⚫🔷🟢⚫ 🟢⚠️🟢🟢⚪⚪
↑ ↑ ↑ ↑ ↑ ↑  ↑ ↑ ↑ ↑ ↑ ↑
F R D U VI VO SI SE R S H H
```
**Legenda:** F=Frigo(ON), R=Risc(ON), D=Deum(OFF), U=Umid(ON), VI=VentIN(ON), VO=VentOUT(OFF), SI=SensInt(OK), SE=SensEst(Warning), R=RTC(OK), S=SD(OK), H=Heartbeat

### 📋 Esempio 2: Programma Automatico - 3 Ore Rimanenti

**Ring 24 (Esterno):**
```
🔴🔴🔴⚪⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫
↑    ↑
3h rimanenti (ROSSO!)  4° ora lampeggia
```

**Ring 12 (Interno):**
```
🟢🟢⚪⚫⚫⚫⚫⚫⚫⚫⚫⚫
↑ ↑ ↑
F1 F2 F3(corrente)
```

---

## 🛠️ Configurazione e Controlli

### 🖱️ Controlli Touch Disponibili
- **Toggle LED**: ON/OFF generale
- **Luminosità**: 0-100% regolabile
- **Test Rainbow**: Animazione demo
- **Reset Timer**: Riavvio contatore ore manuali

### 🔧 Luminosità Consigliata
- **Giorno**: 70-100% per visibilità ottimale
- **Notte**: 30-50% per non disturbare
- **Ambiente Buio**: 10-30% per indicazione minima

---

## 🎯 Vantaggi Sistema Adattivo

### ✅ Modalità Manuale
- **Monitoraggio Tempo**: Controllo durata sessioni manuali
- **Status Attuatori**: Visibilità immediata cosa è attivo
- **Diagnostica Sistema**: Problemi visibili istantaneamente

### ✅ Modalità Automatica  
- **Progress Programma**: Avanzamento fasi chiaro
- **Scadenze**: Alert visivi per tempi critici
- **Controllo Fasi**: Storia completamento visuale

### ✅ Sistema Generale
- **Adattività**: LED sempre utili per il contesto
- **Semplicità**: Ogni modalità ottimizzata per il suo uso
- **Professionalità**: Sistema robusto e affidabile

🥩 **LED sempre utili e mai confusi!** ✨ 