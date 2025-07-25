# Stagionino - Formato Programmi

## 📋 Struttura File Programma

I programmi devono essere salvati nella directory `/programs/` della SD card con estensione `.txt`.

### Formato Header
```
STAGIONINO_PROGRAM
NAME:Nome del programma
DESC:Descrizione del programma
```

### Formato Fasi
```
PHASE:nome_fase,temp_min,temp_max,hum_min,hum_max,duration_hours,is_final
```

### Parametri Fase
- **nome_fase**: Nome descrittivo (max 31 caratteri)
- **temp_min/max**: Range temperatura in °C
- **hum_min/max**: Range umidità in % (usare -1 per non controllare)
- **duration_hours**: Durata in ore (0 = infinita)
- **is_final**: 1 = fase finale/stagionatura, 0 = fase normale

## 🔧 Esempi Pratici

### Salame Felino (3 fasi)
- **Stufatura**: 18-20°C, umidità non controllata, 48h
- **Asciugatura**: 19-20°C, 70-85%, 168h (7 giorni)  
- **Invecchiamento**: 10-12°C, 58-62%, infinita (fase finale)

### Salame Milanese (4 fasi)
- **Stufatura**: 16-24°C, umidità non controllata, 24h
- **Pre-Asciugatura**: 15-18°C, 70-80%, 48h
- **Asciugatura**: 15-18°C, 82-88%, 72h
- **Invecchiamento**: 8-14°C, 60-70%, infinita (fase finale)

## 🚀 Come Usare

1. **Copia file** nella directory `/programs/` della SD
2. **Riavvia Stagionino** per caricare la lista programmi
3. **Seleziona programma** dal menu touch
4. **Avvia esecuzione** automatica
5. **Monitoraggio** real-time via display/seriale

## ⚠️ Note Importanti

- **Max 30 fasi** per programma
- **Fase finale** continua infinitamente fino a stop manuale
- **Controllo umidità** opzionale (-1 = solo temperatura)
- **Avanzamento automatico** o manuale tra fasi
- **Fallback SD**: continua in modalità manuale se SD fallisce
- **Recovery automatico**: retry SD ogni 5 secondi

## 🛡️ Validazione

Il sistema valida automaticamente:
- ✅ Formato file corretto
- ✅ Range temperature validi (-50°C a +100°C)
- ✅ Parametri umidità corretti (0-100% o -1)
- ✅ Numero fasi valido (1-30)
- ✅ Durata ore positive o zero 