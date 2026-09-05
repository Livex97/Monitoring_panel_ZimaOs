# ZimaOS NAS Monitoring Panel

Mini pannello di monitoraggio cyber/tech per ZimaOS basato su **Waveshare ESP32-S3-LCD-1.47** (display IPS 172×320 pixel).

## Architettura

```text
ZimaOS Host ────► zimaos-monitor-api (Docker) ──── REST/JSON ────► ESP32-S3 LCD (LVGL)
```

Home Assistant **non** è richiesto.

## Struttura del Progetto

```text
zimaos-nas-monitor/
├── docs/                     # Documentazione architetturale e API
├── zimaos-monitor-api/       # Backend REST in Python/FastAPI
├── firmware/                 # Firmware ESP32-S3 (PlatformIO + LVGL)
├── docker-compose.yml        # Deployment backend container
└── .env.example              # Configurazione d'esempio
```

## Quick Start — Backend API

1. Avviare il container via Docker Compose:
   ```bash
   docker-compose up -d
   ```
2. Verificare l'endpoint di health check:
   ```bash
   curl http://localhost:8787/health
   ```
3. Verificare l'endpoint di stato unificato JSON:
   ```bash
   curl http://localhost:8787/api/v1/status
   ```
4. Consultare la documentazione OpenAPI automatica su `http://localhost:8787/docs`.

## Sviluppo & Test Backend

```bash
cd zimaos-monitor-api
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
pytest
```
