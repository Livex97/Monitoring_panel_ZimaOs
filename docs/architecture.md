# ZimaOS NAS Monitoring Panel — System Architecture

## Overview

The ZimaOS NAS Monitoring Panel is an embedded dashboard system designed for the **Waveshare ESP32-S3-LCD-1.47** development board (172×320 IPS display). It provides real-time status and operational metrics for a ZimaOS NAS host over Wi-Fi without relying on Home Assistant or third-party cloud services.

## High-Level Diagram

```text
┌─────────────────────────────────────────────────────────────┐
│                        ZimaOS NAS Host                      │
│                                                             │
│   ┌─────────────────────────────────────────────────────┐   │
│   │ Docker Container: zimaos-monitor-api                │   │
│   │                                                     │   │
│   │  ┌──────────────┐  ┌─────────────┐  ┌────────────┐  │   │
│   │  │ System       │  │ Storage     │  │ Docker     │  │   │
│   │  │ Collector    │  │ Collector   │  │ Collector  │  │   │
│   │  └──────┬───────┘  └──────┬──────┘  └─────┬──────┘  │   │
│   │         │                 │               │         │   │
│   │         └─────────┬───────┴───────────────┘         │   │
│   │                   ▼                                 │   │
│   │          Monitor Aggregator Service                 │   │
│   │                   │                                 │   │
│   │                   ▼                                 │   │
│   │          FastAPI REST Server (port 8787)            │   │
│   └───────────────────┬─────────────────────────────────┘   │
└───────────────────────┼─────────────────────────────────────┘
                        │ HTTP / REST JSON
                        │ Wi-Fi 2.4 GHz
                        ▼
┌─────────────────────────────────────────────────────────────┐
│             ESP32-S3 LCD 1.47" Hardware Client              │
│                                                             │
│  ┌───────────────────────┐       ┌───────────────────────┐  │
│  │ HTTP API Client       ├──────►│ Central Data Model    │  │
│  └───────────────────────┘       └──────────┬────────────┘  │
│                                             │               │
│  ┌───────────────────────┐       ┌──────────▼────────────┐  │
│  │ Dual Side Buttons     ├──────►│ LVGL UI Controller    │  │
│  └───────────────────────┘       └──────────┬────────────┘  │
│                                             │               │
│                                  ┌──────────▼────────────┐  │
│                                  │ ST7789 IPS Display    │  │
│                                  │ 172x320 @ Portrait    │  │
│                                  └───────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## Architectural Principles

1. **Decoupled Architecture**: Backend API and ESP32 Firmware are entirely independent modules.
2. **Read-Only / Non-Destructive**: The API collector executes strictly read-only operations and does not alter system configuration.
3. **Graceful Fallbacks**: If hardware metrics (such as CPU temperature or SMART stats) are unavailable on specific hardware, fields return `null` with `available: false` instead of failing.
4. **Resilient Network Handling**: The ESP32 firmware handles HTTP timeouts, server disconnects, and invalid responses without blocking the main loop or crashing.
5. **No External Cloud Dependencies**: Operating entirely inside the local network.
