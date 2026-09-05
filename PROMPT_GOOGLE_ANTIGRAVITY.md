# Prompt completo per Google Antigravity — ZimaOS NAS Monitoring Panel

## Ruolo

Agisci come **Senior Embedded Systems Engineer + Full-Stack/DevOps Engineer + UI/UX Engineer**, specializzato in:

- ESP32-S3
- Arduino/PlatformIO
- LVGL
- display SPI ST7789
- Wi-Fi embedded
- REST/JSON APIs
- Docker
- Linux
- ZimaOS
- monitoring e dashboard real-time

Devi progettare e implementare **l'intero progetto software**, in maniera incrementale, verificabile e documentata.

Non voglio una demo teorica: voglio un progetto realmente compilabile e installabile.

---

# 1. OBIETTIVO DEL PROGETTO

Realizzare un **Mini NAS Monitoring Panel** basato su:

- **Waveshare ESP32-S3-LCD-1.47**
- display IPS 1.47", 172×320 pixel
- controller LCD ST7789/ST7789V3
- ESP32-S3R8
- 16 MB Flash
- 8 MB PSRAM
- Wi-Fi 2.4 GHz
- LED RGB integrato
- due pulsanti fisici laterali

Il dispositivo deve collegarsi **direttamente via Wi-Fi al PC sul quale è installato ZimaOS**.

### Vincolo fondamentale

**Home Assistant NON deve essere necessario.**

Home Assistant esiste su un'altra macchina/VM Proxmox, ma questo progetto deve funzionare autonomamente:

```text
ESP32-S3 LCD
     │
     │ Wi-Fi / HTTP(S) / JSON
     ▼
ZimaOS
     │
     └── Docker container: zimaos-monitor-api
```

Home Assistant potrà eventualmente essere integrato in futuro, ma NON deve essere una dipendenza.

---

# 2. RISULTATO FINALE DESIDERATO

Il dispositivo deve diventare un piccolo pannello cyber/tech dedicato al NAS.

L'estetica di riferimento è quella del mockup fornito dall'utente:

- dark background
- nero/blu molto scuro
- elementi HUD
- linee sottili/cyan
- accenti cyan
- verde per stato OK
- giallo/arancio per warning
- rosso per errori
- viola per alcuni grafici secondari
- icone tecniche
- barre di progresso
- indicatori circolari
- piccoli grafici
- animazioni leggere
- look professionale, non giocattolo

Il display è piccolo, quindi la UI deve essere progettata specificamente per **172×320 px**, non semplicemente ridimensionata da una dashboard desktop.

---

# 3. ARCHITETTURA GENERALE

Implementare due componenti indipendenti.

## COMPONENTE A — ZimaOS Monitor API

Un container Docker installabile direttamente su ZimaOS.

Responsabilità:

1. interrogare le API ufficiali ZimaOS quando disponibili;
2. recuperare metriche del sistema host quando le API non espongono un dato;
3. recuperare stato dei container Docker;
4. normalizzare tutto in un unico JSON;
5. fornire endpoint REST molto semplici;
6. non modificare lo stato del NAS;
7. operare in modalità il più possibile read-only;
8. essere leggero;
9. avere health check;
10. avere logging;
11. avere gestione errori e valori mancanti.

Nome suggerito:

`zimaos-monitor-api`

Tecnologia preferita:

- Python 3
- FastAPI
- Uvicorn
- Pydantic
- psutil
- Docker SDK per Python oppure accesso controllato al Docker socket
- smartmontools solo se realmente necessario
- pytest per i test

Non aggiungere dipendenze inutili.

---

# 4. PRIMA REGOLA: NON INVENTARE LE API ZIMAOS

Prima di scrivere il codice che interroga ZimaOS:

1. consultare la documentazione ufficiale ZimaOS OpenAPI;
2. identificare gli endpoint realmente disponibili nella versione attuale;
3. verificare autenticazione;
4. verificare endpoint di:
   - sistema
   - CPU
   - RAM
   - storage
   - dischi
   - RAID
   - applicazioni
   - Docker
   - network, se disponibile;
5. documentare gli endpoint utilizzati;
6. implementare adapter separati per ciascuna categoria.

Se un dato NON è disponibile tramite API ufficiale, non inventare un endpoint.

Utilizzare un fallback Linux read-only solo quando necessario.

Fonti ufficiali di riferimento:

- ZimaOS OpenAPI Developer Guide
- ZimaOS OpenAPI Live Preview
- repository ufficiale IceWhale OpenAPI
- documentazione ufficiale Waveshare
- documentazione LVGL

---

# 5. STRUTTURA DEL PROGETTO

Creare un repository ordinato con questa struttura iniziale:

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
│   │   │   ├── system.py
│   │   │   ├── storage.py
│   │   │   ├── docker.py
│   │   │   ├── network.py
│   │   │   └── zimaos_api.py
│   │   └── services/
│   │       └── monitor.py
│   └── tests/
│
└── firmware/
    ├── platformio.ini
    ├── README.md
    ├── include/
    ├── src/
    │   ├── main.cpp
    │   ├── config.h
    │   ├── network/
    │   ├── api/
    │   ├── display/
    │   ├── ui/
    │   ├── input/
    │   └── system/
    └── test/
```

Puoi modificare questa struttura se esiste una motivazione tecnica concreta, ma mantieni sempre una separazione netta tra API e firmware.

---

# 6. COMPONENTE A — METRICHE DA RACCOGLIERE

Il JSON finale deve essere progettato per contenere almeno:

## Stato generale

- online
- hostname
- ZimaOS version, se disponibile
- timestamp
- uptime

## CPU

- usage percent
- load 1m
- load 5m
- load 15m
- core count
- frequency, se disponibile

## RAM

- total
- used
- free
- available
- percentage

## Temperatura

Tentare di recuperare:

- CPU temperature
- system temperature
- eventuali temperature disponibili

Se il dato non è disponibile:

```json
{
  "value": null,
  "available": false
}
```

Non visualizzare dati inventati.

## Storage

Per ogni pool/filesystem disponibile:

- name
- total
- used
- free
- percentage

## Dischi

Per ogni disco:

- name
- model
- serial solo se strettamente necessario; preferibilmente NON esporlo
- capacity
- temperature
- health
- SMART status, se disponibile

Non esporre informazioni sensibili o inutili.

## RAID / pool

- name
- type
- status
- health
- degraded
- rebuild/resilver progress, se disponibile

## Docker

Per ogni container:

- name
- status
- running
- image, se utile
- uptime, se disponibile
- restart count, se disponibile

## Network

- interface
- RX bytes
- TX bytes
- RX rate
- TX rate
- link status, se disponibile

L'API può calcolare RX/TX rate usando due campionamenti consecutivi.

---

# 7. JSON CANONICO

Creare un modello Pydantic stabile.

Esempio concettuale:

```json
{
  "schema_version": "1.0",
  "timestamp": "2026-09-05T10:00:00Z",
  "server": {
    "online": true,
    "hostname": "zima-nas",
    "os": "ZimaOS",
    "version": "..."
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

Questo è solo uno schema iniziale.

Adattalo ai dati realmente disponibili.

---

# 8. API DEL CONTAINER

Creare almeno:

```text
GET /health
GET /api/v1/status
GET /api/v1/system
GET /api/v1/storage
GET /api/v1/docker
GET /api/v1/network
```

Endpoint principale:

```text
GET /api/v1/status
```

deve restituire il JSON completo.

Aggiungere:

```text
GET /api/v1/version
```

e una pagina Swagger/OpenAPI.

La API deve avere timeout ragionevoli.

Il firmware non deve rimanere bloccato se ZimaOS risponde lentamente.

---

# 9. SICUREZZA

La sicurezza è importante.

NON inserire nell'ESP32:

- password amministratore ZimaOS;
- credenziali personali;
- token con permessi di scrittura;
- segreti hardcoded nel repository.

Preferire:

1. endpoint read-only;
2. token/API key con minimo privilegio, se supportato;
3. configurazione separata;
4. variabili d'ambiente;
5. possibilità di proteggere l'API tramite API key.

Se possibile, implementare:

```text
X-API-Key
```

sull'endpoint del monitor.

L'ESP32 deve conservare la API key in una configurazione locale non committata.

---

# 10. DOCKER CONTAINER

Creare un Dockerfile piccolo e sicuro.

Requisiti:

- immagine Python slim;
- utente non-root dove possibile;
- HEALTHCHECK;
- restart policy;
- log ragionevoli;
- nessun privilegio inutile.

Per leggere il Docker Engine, utilizzare preferibilmente:

```text
/var/run/docker.sock
```

in modalità read-only.

Se servono metriche host, valutare mount read-only come:

```text
/proc
/sys
```

ma evitare privilegi elevati.

Se un mount o un accesso privilegiato è indispensabile, documentarne chiaramente il motivo.

Non utilizzare `privileged: true` senza una necessità tecnica dimostrata.

---

# 11. INSTALLAZIONE SU ZIMAOS

Fornire:

```text
docker-compose.yml
```

compatibile con ZimaOS.

Deve essere possibile installarlo:

- tramite terminale;
- tramite Docker Compose;
- eventualmente adattarlo successivamente al formato App Store/x-casaos.

Non rendere obbligatoria la pubblicazione nello store.

Creare anche:

```text
.env.example
```

con:

```text
MONITOR_API_KEY=
MONITOR_PORT=8787
```

Non inserire valori reali.

---

# 12. TEST DEL BACKEND PRIMA DEL FIRMWARE

Prima di sviluppare la UI ESP32, il backend deve essere testabile da PC.

Fornire comandi tipo:

```bash
curl http://ZIMAOS_IP:8787/health
```

e:

```bash
curl -H "X-API-Key: ..." \
  http://ZIMAOS_IP:8787/api/v1/status
```

Creare test automatici per:

- schema JSON;
- gestione dati mancanti;
- Docker;
- storage;
- CPU;
- RAM;
- network;
- error handling.

Il progetto deve poter simulare ZimaOS attraverso fixture/mock per testare il backend senza NAS reale.

---

# 13. COMPONENTE B — FIRMWARE ESP32-S3

Utilizzare preferibilmente:

- PlatformIO
- framework Arduino
- LVGL
- driver display appropriato
- WiFi
- HTTPClient oppure client HTTP equivalente
- JSON parsing leggero

Valutare PSRAM.

La scelta Arduino + LVGL è preferita per velocizzare sviluppo e manutenzione, salvo impedimenti tecnici reali.

---

# 14. HARDWARE TARGET

Target:

**Waveshare ESP32-S3-LCD-1.47**

Specifiche di riferimento:

- ESP32-S3R8
- 240 MHz
- 16 MB Flash
- 8 MB PSRAM
- LCD 172×320
- ST7789/ST7789V3
- SPI
- RGB LED
- BOOT
- RESET
- due pulsanti laterali

NON assumere il modello 1.47B USB-C: il dispositivo dell'utente è la versione USB-A mostrata nelle foto.

Prima di fissare definitivamente pin e init sequence, verificare la documentazione ufficiale del modello esatto.

---

# 15. DISPLAY

Il display deve essere orientato verticalmente:

```text
172 px
┌──────────────┐
│              │
│              │
│              │
│              │ 320 px
│              │
│              │
│              │
└──────────────┘
```

Ottimizzare la UI per la leggibilità da circa 30–100 cm.

Usare font leggibili.

Non utilizzare testi troppo piccoli solo per mostrare più informazioni.

---

# 16. UI FINALE

Creare almeno 3 pagine.

## PAGINA 1 — OVERVIEW

Contenuti:

- ZIMAOS
- NAS SERVER
- ONLINE/OFFLINE
- CPU %
- RAM %
- temperatura
- storage %
- network RX/TX
- uptime

Look:

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

Usare grafici mini/sparkline solo se rimane leggibile.

---

# 17. PAGINA 2 — STORAGE

Mostrare:

- pool principale;
- utilizzato;
- libero;
- percentuale;
- dischi;
- temperatura;
- health;
- RAID/pool status.

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

Se il NAS ha una configurazione diversa, adattare dinamicamente.

---

# 18. PAGINA 3 — DOCKER

Mostrare:

- totale container;
- running;
- stopped;
- lista container;
- stato.

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

Se ci sono più container della capacità del display:

- scroll automatico lento;
oppure
- paginazione.

Preferire paginazione o scroll controllato, evitando animazioni inutilmente rapide.

---

# 19. NAVIGAZIONE

Usare i due pulsanti laterali.

Comportamento desiderato:

- pulsante sinistro → pagina precedente;
- pulsante destro → pagina successiva;
- pressione breve → cambio pagina;
- pressione lunga di entrambi → reset/reconfigure Wi-Fi, ma solo se chiaramente documentato.

Implementare debounce hardware/software.

Non usare il touch perché il modello dell'utente non è touch.

---

# 20. LED RGB

Usare il LED RGB integrato come indicatore globale.

Proposta:

- verde fisso/pulsante → tutto OK;
- giallo → warning;
- rosso → errore critico;
- blu lampeggiante → connessione/aggiornamento;
- viola → configurazione;
- spento → sleep/offline.

Evitare lampeggiamenti fastidiosi.

Il LED deve essere configurabile.

---

# 21. STATO OFFLINE

Se ZimaOS non risponde:

mostrare:

```text
ZIMAOS

● OFFLINE

Unable to reach
monitor API

Retrying...
```

Con:

- ultima lettura valida;
- timestamp ultima lettura;
- retry automatico.

Non bloccare mai il firmware.

---

# 22. REFRESH

Default:

```text
5 secondi
```

Configurabile.

Non interrogare ZimaOS più spesso del necessario.

Se il backend risponde con un timestamp, visualizzare i dati come "stale" se troppo vecchi.

Implementare timeout HTTP ad esempio nell'ordine di 2–3 secondi, configurabile.

---

# 23. ANIMAZIONI

Usare animazioni leggere:

- progress bar;
- rotazione/refresh indicator;
- transizione pagina;
- pulse degli indicatori.

NON sacrificare:

- FPS;
- leggibilità;
- RAM;
- stabilità Wi-Fi.

---

# 24. CONFIGURAZIONE FIRMWARE

Creare un file tipo:

```text
config.h
```

o sistema equivalente per:

```text
WIFI_SSID
WIFI_PASSWORD
ZIMAOS_MONITOR_HOST
ZIMAOS_MONITOR_PORT
ZIMAOS_API_KEY
REFRESH_INTERVAL
```

I dati personali devono essere esclusi da Git.

Creare:

```text
secrets.example.h
```

ma NON committare:

```text
secrets.h
```

---

# 25. WIFI

Implementare:

- connessione automatica;
- reconnect;
- timeout;
- indicatore Wi-Fi;
- RSSI;
- fallback se la connessione cade.

Non riavviare continuamente l'ESP32 in caso di Wi-Fi assente.

---

# 26. GESTIONE ERRORI

Gestire:

- Wi-Fi assente;
- DNS fallito;
- timeout HTTP;
- HTTP 401;
- HTTP 404;
- HTTP 500;
- JSON invalido;
- dati mancanti;
- ZimaOS offline;
- backend offline;
- container non disponibile;
- storage non disponibile.

L'UI deve continuare a funzionare anche in caso di errore.

---

# 27. PERFORMANCE

Prestare attenzione a:

- frammentazione heap;
- allocazioni String ripetute;
- JSON troppo grandi;
- redraw completo continuo;
- memoria LVGL;
- PSRAM;
- watchdog;
- blocchi durante HTTP.

Preferire buffer e strutture dati stabili.

Monitorare:

```text
free heap
free PSRAM
loop time
HTTP duration
```

Durante lo sviluppo.

---

# 28. LOGGING

Implementare livelli:

```text
ERROR
WARN
INFO
DEBUG
```

DEBUG disattivabile.

Non stampare:

- password;
- API key;
- dati sensibili.

---

# 29. AGGIORNAMENTO UI

L'architettura deve separare:

```text
Network
   ↓
API Client
   ↓
Data Model
   ↓
UI
```

La UI non deve effettuare direttamente richieste HTTP.

---

# 30. MODELLO DATI ESP32

Creare strutture tipizzate per:

- ServerStatus
- CPUStatus
- MemoryStatus
- TemperatureStatus
- StorageStatus
- DiskStatus
- DockerStatus
- ContainerStatus
- NetworkStatus
- HealthStatus

Se il JSON cambia versione:

```text
schema_version
```

deve permettere di gestire compatibilità futura.

---

# 31. PAGINA DI CONFIGURAZIONE

Prevedere una modalità iniziale di configurazione Wi-Fi.

Per la prima versione può essere semplice:

- SSID/password compilati in secrets.h.

Come evoluzione:

- captive portal;
- AP mode;
- configurazione da browser.

Non rendere il captive portal obbligatorio per la prima release.

---

# 32. RECOVERY

Implementare un comportamento sicuro:

Se firmware non riesce a connettersi al Wi-Fi per un certo periodo:

- non andare in boot loop;
- mantenere UI locale;
- permettere eventuale reconfiguration futura.

---

# 33. BUILD E FLASH

Documentare:

```bash
pio run
pio run --target upload
pio device monitor
```

Indicare:

- ambiente PlatformIO;
- board;
- framework;
- librerie;
- baudrate;
- eventuale boot procedure;
- eventuale configurazione USB.

---

# 34. README

Il README principale deve spiegare:

1. cosa fa il progetto;
2. architettura;
3. hardware;
4. installazione backend;
5. configurazione;
6. flashing firmware;
7. API;
8. troubleshooting;
9. sicurezza;
10. roadmap.

---

# 35. DOCUMENTAZIONE ZIMAOS

Creare:

```text
docs/zimaos-api.md
```

con una tabella:

| Dato | Fonte | Endpoint/Metodo | Disponibile | Fallback |
|---|---|---|---|---|
| CPU | ZimaOS/Linux | ... | sì/no | ... |
| RAM | ... | ... | ... | ... |
| Temp | ... | ... | ... | ... |
| Storage | ... | ... | ... | ... |
| RAID | ... | ... | ... | ... |
| Docker | Docker Engine | ... | ... | ... |
| Network | Linux | ... | ... | ... |

NON compilare la tabella con supposizioni: verificare realmente.

---

# 36. ROADMAP OBBLIGATORIA

Procedere in milestone.

## MILESTONE 1 — BACKEND

Obiettivo:

```text
ZimaOS
   ↓
zimaos-monitor-api
   ↓
JSON
```

Prima completare e testare questo.

Deliverable:

- Dockerfile
- docker-compose.yml
- API
- JSON schema
- test
- README
- documentazione

NON iniziare la UI firmware prima che il backend sia funzionante.

---

## MILESTONE 2 — TEST API

Verificare:

```text
/health
/api/v1/status
/api/v1/system
/api/v1/storage
/api/v1/docker
/api/v1/network
```

Testare con curl.

---

## MILESTONE 3 — DISPLAY BASE

Prima visualizzare:

```text
ZIMAOS
Connecting...
```

Poi:

```text
WiFi OK
ZimaOS OK
```

---

## MILESTONE 4 — API CLIENT ESP32

Collegare ESP32 al backend.

Visualizzare:

- CPU
- RAM
- temp
- storage.

---

## MILESTONE 5 — LVGL UI

Implementare la grafica definitiva.

---

## MILESTONE 6 — STORAGE

Implementare pagina Storage.

---

## MILESTONE 7 — DOCKER

Implementare pagina Docker.

---

## MILESTONE 8 — INPUT

Implementare pulsanti e navigazione.

---

## MILESTONE 9 — ERROR HANDLING

Testare:

- ZimaOS spento;
- Wi-Fi spento;
- backend spento;
- JSON invalido;
- API key errata;
- container down.

---

## MILESTONE 10 — RELEASE

Preparare:

```text
v1.0.0
```

con:

- firmware;
- Docker image;
- compose;
- documentazione;
- changelog.

---

# 37. CRITERI DI ACCETTAZIONE

Il progetto è considerato completato solo se:

### Backend

- [ ] container avviabile su ZimaOS;
- [ ] `/health` funzionante;
- [ ] `/api/v1/status` funzionante;
- [ ] JSON validato;
- [ ] metriche reali;
- [ ] Docker status reale;
- [ ] storage reale;
- [ ] gestione errori;
- [ ] API documentata.

### ESP32

- [ ] Wi-Fi funzionante;
- [ ] display corretto;
- [ ] UI 172×320;
- [ ] API ZimaOS funzionante;
- [ ] refresh automatico;
- [ ] 3 pagine;
- [ ] pulsanti;
- [ ] LED RGB;
- [ ] offline mode;
- [ ] watchdog;
- [ ] nessun boot loop.

### Qualità

- [ ] codice commentato dove serve;
- [ ] nessun segreto nel repository;
- [ ] README completo;
- [ ] installazione ripetibile;
- [ ] struttura ordinata;
- [ ] test principali presenti.

---

# 38. REGOLE DI SVILUPPO PER ANTIGRAVITY

Queste regole sono importanti.

1. **Non generare tutto in un unico passaggio gigantesco.**
2. Procedere milestone per milestone.
3. Dopo ogni milestone:
   - compilare;
   - testare;
   - correggere;
   - documentare.
4. Non dichiarare una milestone completata senza verificarla.
5. Non inventare API ZimaOS.
6. Non inventare pin hardware.
7. Consultare sempre la documentazione ufficiale per hardware/API.
8. Se un dato non è disponibile, rappresentarlo come `null`/`unavailable`.
9. Non inserire credenziali reali.
10. Non introdurre Home Assistant come dipendenza.
11. Mantenere backend e firmware indipendenti.
12. Preferire codice semplice e manutenibile.
13. Evitare dipendenze non necessarie.
14. Ottimizzare per stabilità prima che per effetti grafici.
15. Prima del rilascio eseguire un test end-to-end.

---

# 39. MOCKUP UI

Usa il mockup fornito dall'utente come riferimento visivo.

Non è necessario copiarlo pixel-per-pixel.

Deve essere interpretato come:

- direzione estetica;
- layout;
- gerarchia informazioni;
- palette;
- stile cyber/tech.

La priorità è:

```text
leggibilità
>
informazioni importanti
>
stabilità
>
estetica
>
animazioni
```

---

# 40. FASE FINALE

Al termine del progetto produci:

```text
1. Docker image/backend
2. docker-compose.yml
3. API documentation
4. JSON schema
5. PlatformIO firmware
6. configurazione secrets
7. README
8. installation guide
9. troubleshooting guide
10. changelog
```

E soprattutto produci un report finale:

```text
ARCHITETTURA
----------------
Backend:
...

ESP32:
...

API:
...

Display:
...

Metriche:
...

Limitazioni:
...

Test eseguiti:
...

Problemi residui:
...

Come fare il flash:
...

Come aggiornare il backend:
...

Come aggiornare il firmware:
...
```

## ORDINE DI ESECUZIONE DA RISPETTARE

```text
1. Analizza hardware e documentazione ufficiale
2. Analizza API ZimaOS
3. Definisci schema JSON
4. Implementa backend
5. Dockerizza backend
6. Installa/testa backend su ZimaOS
7. Testa JSON
8. Implementa firmware base
9. Testa display
10. Testa Wi-Fi
11. Implementa API client
12. Implementa data model
13. Implementa LVGL
14. Implementa Overview
15. Implementa Storage
16. Implementa Docker
17. Implementa pulsanti
18. Implementa LED
19. Implementa error handling
20. Test end-to-end
21. Documentazione
22. Release v1.0.0
```

**Inizia dalla MILESTONE 1. Non procedere alla milestone successiva finché quella corrente non è verificata.**
