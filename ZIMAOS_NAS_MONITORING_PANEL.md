# ZimaOS NAS Monitoring Panel — Procedura completa

## 1. Obiettivo

Questo progetto trasforma una **Waveshare ESP32-S3-LCD-1.47** in un piccolo pannello di monitoraggio dedicato al NAS ZimaOS.

L'ESP32 si collega direttamente via Wi-Fi al PC che esegue ZimaOS.

**Home Assistant non è una dipendenza.**

Architettura:

```text
                    LAN / Wi-Fi
                         │
              ┌──────────┴──────────┐
              │                     │
        ┌─────▼─────┐         ┌─────▼─────┐
        │  ZimaOS   │         │ ESP32-S3  │
        │    NAS    │◄────────│ 1.47 LCD  │
        │           │  HTTP   │           │
        │ Docker    │  JSON   │ LVGL UI   │
        │ Storage   │         │ RGB LED   │
        │ CPU/RAM   │         └───────────┘
        └───────────┘
              │
       ┌──────▼─────────┐
       │ zimaos-monitor │
       │      -api      │
       │ Docker         │
       │ FastAPI        │
       └────────────────┘
```

---

# 2. Hardware

Scheda:

**Waveshare ESP32-S3-LCD-1.47**

Caratteristiche rilevanti:

- ESP32-S3R8
- dual-core fino a 240 MHz
- 16 MB Flash
- 8 MB PSRAM
- Wi-Fi 2.4 GHz
- Bluetooth LE
- display IPS 172×320
- controller ST7789/ST7789V3
- RGB LED
- USB-A
- TF/microSD
- pulsanti BOOT/RESET
- pulsanti laterali

La scheda dell'utente è la versione **USB-A**, non la 1.47B USB-C.

Prima di fissare nel firmware pin e sequenza di inizializzazione, verificare sempre la documentazione del modello esatto.

Riferimenti ufficiali:

- Waveshare ESP32-S3-LCD-1.47
- Waveshare 1.47-inch LCD/ST7789
- LVGL documentation

---

# 3. Filosofia del progetto

Il progetto deve essere diviso in due blocchi:

```text
BACKEND
ZimaOS → API JSON

FIRMWARE
API JSON → ESP32 → LVGL → LCD
```

Questo permette di:

- cambiare firmware senza modificare ZimaOS;
- cambiare backend senza rifare la UI;
- testare il backend con curl;
- simulare dati durante lo sviluppo;
- aggiungere altri client in futuro.

---

# 4. Milestone 1 — ZimaOS Monitor API

## 4.1 Scopo

Creare un container Docker:

```text
zimaos-monitor-api
```

che raccolga le informazioni reali del NAS.

Tecnologia consigliata:

```text
Python
FastAPI
Pydantic
psutil
Docker SDK
```

---

## 4.2 Regola fondamentale sulle API

Non assumere endpoint ZimaOS.

Prima bisogna verificare la OpenAPI della versione installata.

ZimaOS dispone di API per:

- system;
- storage;
- disk/RAID;
- app management;
- Docker;
- altri servizi.

Quando un dato è disponibile tramite API ufficiale, usare l'API ufficiale.

Quando non lo è, utilizzare un fallback Linux read-only.

Riferimenti ufficiali:

- ZimaOS OpenAPI Developer Guide
- ZimaOS OpenAPI Live Preview
- IceWhale OpenAPI repository

---

# 5. Metriche

Il backend deve cercare di produrre:

## Server

- online
- hostname
- ZimaOS version
- timestamp
- uptime

## CPU

- usage %
- load average
- core count
- frequency, se disponibile

## RAM

- total
- used
- available
- free
- usage %

## Temperature

- CPU
- system
- altre disponibili

## Storage

- pool
- filesystem
- total
- used
- free
- %

## Disk

- nome
- modello
- capacità
- temperatura
- health
- SMART, quando disponibile

Evitare di trasmettere seriali degli HDD se non necessari.

## RAID

- tipo
- stato
- health
- degraded
- rebuild/resilver

## Docker

- numero totale
- running
- stopped
- nome
- stato
- uptime, se disponibile

## Network

- interfaccia
- RX
- TX
- rate RX
- rate TX

---

# 6. Schema JSON

Il formato deve essere stabile.

Esempio:

```json
{
  "schema_version": "1.0",
  "timestamp": "2026-09-05T10:00:00Z",
  "server": {
    "online": true,
    "hostname": "zima-nas",
    "os": "ZimaOS",
    "version": "1.x"
  },
  "uptime": {
    "seconds": 1048320
  },
  "cpu": {
    "usage_percent": 23.0,
    "load_1m": 0.42,
    "load_5m": 0.37,
    "load_15m": 0.31,
    "cores": 4
  },
  "memory": {
    "total_bytes": 17179869184,
    "used_bytes": 7030000000,
    "available_bytes": 10100000000,
    "usage_percent": 41.0
  },
  "temperature": {
    "cpu_celsius": 46.0,
    "system_celsius": null
  },
  "storage": {
    "pools": [],
    "disks": []
  },
  "docker": {
    "total": 7,
    "running": 7,
    "stopped": 0,
    "containers": []
  },
  "network": {
    "interfaces": []
  },
  "health": {
    "status": "healthy",
    "warnings": [],
    "errors": []
  }
}
```

Questo è uno schema di progetto, non una garanzia dei nomi/valori restituiti da ZimaOS.

---

# 7. Endpoint

Implementare:

```text
GET /health
GET /api/v1/status
GET /api/v1/system
GET /api/v1/storage
GET /api/v1/docker
GET /api/v1/network
GET /api/v1/version
```

Il più importante è:

```text
GET /api/v1/status
```

che deve contenere tutti i dati necessari al firmware.

Aggiungere documentazione OpenAPI/Swagger.

---

# 8. Docker

Il container deve:

- essere leggero;
- avere healthcheck;
- essere preferibilmente non-root;
- avere solo i privilegi indispensabili;
- usare Docker socket in read-only se necessario;
- usare mount `/proc`/`/sys` solo se realmente necessari;
- evitare `privileged: true`.

Esempio concettuale:

```text
ZimaOS
│
├── /var/run/docker.sock
│       ↓ read-only
│
└── zimaos-monitor-api
```

Per metriche host, se necessarie:

```text
/host/proc
/host/sys
```

in sola lettura.

---

# 9. Sicurezza

Non mettere password o API key nel repository.

Creare:

```text
.env.example
```

con:

```text
MONITOR_API_KEY=
MONITOR_PORT=8787
```

L'API può essere protetta con:

```text
X-API-Key
```

L'ESP32 conserverà la chiave localmente.

La chiave deve avere il minimo privilegio possibile.

---

# 10. Test backend

Prima di sviluppare il firmware:

```bash
curl http://IP_ZIMAOS:8787/health
```

poi:

```bash
curl -H "X-API-Key: ..." \
http://IP_ZIMAOS:8787/api/v1/status
```

Devono essere verificati:

- CPU reale;
- RAM reale;
- temperatura;
- storage;
- dischi;
- RAID;
- Docker;
- network.

Testare anche:

- ZimaOS spento;
- Docker socket non disponibile;
- storage non disponibile;
- dati mancanti;
- errore API.

---

# 11. Milestone 2 — Firmware

Il firmware sarà realizzato separatamente.

Stack consigliato:

```text
PlatformIO
Arduino
LVGL
WiFi
HTTP client
JSON parser
```

La struttura deve essere:

```text
WiFi
 ↓
HTTP API Client
 ↓
Data Model
 ↓
LVGL UI
 ↓
LCD
```

La UI non deve contenere codice HTTP.

---

# 12. Display

Target:

```text
172 × 320 px
```

Orientamento verticale.

Obiettivo:

- alta leggibilità;
- informazioni essenziali;
- pochi testi minuscoli;
- grafica cyber/tech;
- refresh fluido.

---

# 13. UI

## Pagina 1 — Overview

Mostrare:

```text
ZIMAOS
NAS SERVER

● ONLINE

SYSTEM
CPU       23%
RAM       41%
TEMP      46°C

STORAGE
████████░░ 78%

NETWORK
↓ 48.2 MB/s
↑ 12.4 MB/s

UPTIME
12d 04h 32m
```

Elementi grafici:

- progress bar;
- indicatori circolari;
- mini grafici;
- linee HUD;
- icone.

---

# 14. Pagina 2 — Storage

Esempio:

```text
STORAGE

POOL 1

78%

1.56 TB / 2.00 TB

DISK STATUS

HDD 1       ● OK
2.00 TB
42°C

HDD 2       ● OK
2.00 TB
44°C

RAID

✓ HEALTHY
RAID 1
```

Adattare dinamicamente al numero di dischi/pool.

---

# 15. Pagina 3 — Docker

Esempio:

```text
DOCKER

7 TOTAL
7 RUNNING
0 STOPPED

CONTAINERS

● Jellyfin       UP
● Immich         UP
● Grafana        UP
● Prometheus     UP
● MariaDB        UP
● Tailscale      UP
```

Se i container sono troppi:

- paginazione;
- oppure scroll lento.

---

# 16. Pulsanti

Usare i due pulsanti laterali.

```text
SINISTRO → pagina precedente
DESTRO   → pagina successiva
```

Implementare debounce.

Pressione lunga opzionale per configurazione/reset.

---

# 17. LED RGB

Stati consigliati:

| LED | Significato |
|---|---|
| Verde | tutto OK |
| Giallo | warning |
| Rosso | errore |
| Blu lampeggiante | connessione/refresh |
| Viola | configurazione |
| Spento | sleep/offline |

Il LED non deve lampeggiare continuamente senza motivo.

---

# 18. Offline mode

Se ZimaOS non risponde:

```text
ZIMAOS

● OFFLINE

Unable to reach
monitor API

Retrying...
```

Con eventuale:

```text
Last update:
10:32:15
```

Conservare l'ultima lettura valida.

---

# 19. Refresh

Default:

```text
5 secondi
```

Configurabile.

Timeout HTTP:

```text
2–3 secondi
```

Il firmware non deve bloccarsi durante una richiesta.

---

# 20. Wi-Fi

Implementare:

- auto-connect;
- reconnect;
- timeout;
- RSSI;
- gestione assenza Wi-Fi.

Non creare boot loop se il Wi-Fi è spento.

---

# 21. Gestione memoria

Monitorare durante lo sviluppo:

```text
Free heap
Free PSRAM
LVGL memory
HTTP buffer
JSON buffer
```

Evitare:

- allocazioni continue;
- JSON enormi;
- redraw completi inutili;
- String concatenate ripetutamente.

---

# 22. Configurazione

Creare:

```text
secrets.example.h
```

e usare:

```text
WIFI_SSID
WIFI_PASSWORD
ZIMAOS_MONITOR_HOST
ZIMAOS_MONITOR_PORT
ZIMAOS_API_KEY
```

Il file reale:

```text
secrets.h
```

deve essere in `.gitignore`.

---

# 23. Test firmware

Testare:

### Test 1
Wi-Fi disponibile.

### Test 2
Wi-Fi assente.

### Test 3
ZimaOS disponibile.

### Test 4
ZimaOS spento.

### Test 5
API key errata.

### Test 6
JSON incompleto.

### Test 7
Container Docker in DOWN.

### Test 8
Storage degraded.

### Test 9
Temperatura non disponibile.

### Test 10
Riavvio ESP32.

In tutti i casi il firmware deve rimanere stabile.

---

# 24. Ordine di sviluppo

Seguire rigorosamente:

```text
1. Analisi hardware
2. Analisi API ZimaOS
3. Schema JSON
4. Backend
5. Docker
6. Installazione ZimaOS
7. Test API
8. Firmware minimo
9. Test LCD
10. Wi-Fi
11. API client
12. Data model
13. LVGL
14. Overview
15. Storage
16. Docker
17. Pulsanti
18. RGB LED
19. Error handling
20. Test end-to-end
21. Documentazione
22. Release v1.0.0
```

---

# 25. Struttura repository

Struttura suggerita:

```text
zimaos-nas-monitor/
│
├── README.md
├── LICENSE
├── .gitignore
├── docker-compose.yml
│
├── docs/
│   ├── architecture.md
│   ├── zimaos-api.md
│   ├── json-schema.md
│   ├── installation.md
│   ├── troubleshooting.md
│   └── ui.md
│
├── zimaos-monitor-api/
│   ├── Dockerfile
│   ├── requirements.txt
│   ├── app/
│   │   ├── main.py
│   │   ├── config.py
│   │   ├── models.py
│   │   ├── collectors/
│   │   └── services/
│   └── tests/
│
└── firmware/
    ├── platformio.ini
    ├── README.md
    ├── include/
    ├── src/
    └── test/
```

---

# 26. Criteri di completamento

## Backend

- [ ] Docker funzionante
- [ ] `/health`
- [ ] `/api/v1/status`
- [ ] CPU
- [ ] RAM
- [ ] temperatura
- [ ] storage
- [ ] dischi
- [ ] RAID
- [ ] Docker
- [ ] network
- [ ] gestione errori
- [ ] test
- [ ] documentazione

## Firmware

- [ ] LCD
- [ ] Wi-Fi
- [ ] API
- [ ] JSON
- [ ] Overview
- [ ] Storage
- [ ] Docker
- [ ] pulsanti
- [ ] LED
- [ ] offline mode
- [ ] watchdog
- [ ] stabilità

---

# 27. Design UI

La UI deve seguire il mockup cyber/tech fornito.

Palette concettuale:

```text
Background → quasi nero
Primary → cyan
Success → verde
Warning → giallo/arancio
Error → rosso
Secondary → viola
Text → bianco/grigio chiaro
```

Non sacrificare la leggibilità.

Gerarchia:

```text
STATO NAS
↓
METRICHE PRINCIPALI
↓
STORAGE / DOCKER
↓
DETTAGLI
```

---

# 28. Futuri sviluppi

Non sono obbligatori per v1.0, ma lasciare l'architettura pronta per:

- captive portal Wi-Fi;
- configurazione da browser;
- OTA firmware;
- grafici storici;
- pagina Network;
- pagina System;
- informazioni SMART avanzate;
- temperature HDD;
- alert più dettagliati;
- luminosità automatica;
- sleep;
- supporto MQTT;
- integrazione opzionale con Home Assistant.

---

# 29. Nota importante

Il backend deve essere il punto di astrazione.

L'ESP32 non deve conoscere la complessità di ZimaOS.

Il firmware deve conoscere solamente:

```text
GET /api/v1/status
```

e lo schema JSON.

In questo modo, se ZimaOS cambia le proprie API, sarà necessario aggiornare il backend e non il firmware.

---

# 30. Risultato finale atteso

Il dispositivo deve apparire come un piccolo:

**ZIMAOS NAS SERVER MONITOR**

con tre pagine:

```text
┌──────────────┐
│   ZIMAOS     │
│   ● ONLINE   │
│              │
│ CPU     23%  │
│ RAM     41%  │
│ TEMP    46°C │
│              │
│ STORAGE 78%  │
│              │
│ ↓ 48 MB/s    │
│ ↑ 12 MB/s    │
└──────────────┘
```

con estetica **cyber/tech**, indicatori colorati e stato real-time.

---

# 31. Riferimenti tecnici ufficiali

Consultare sempre la versione aggiornata della documentazione prima di implementare:

- ZimaOS OpenAPI Developer Guide
- ZimaOS OpenAPI Live Preview
- ZimaOS Developer Documentation
- Waveshare ESP32-S3-LCD-1.47
- Waveshare 1.47-inch LCD Module
- LVGL documentation

La documentazione ufficiale ZimaOS indica attualmente API separate per sistema, local storage/disk/RAID e gestione app/Docker; la documentazione Waveshare identifica il display da 172×320 e l'uso di LVGL come caso d'uso supportato.
