# Canonical JSON Schema v1.0

## Specification

The primary endpoint `/api/v1/status` returns a structured JSON document representing the full state of the ZimaOS host.

```json
{
  "schema_version": "1.0",
  "timestamp": "2026-09-05T11:00:00Z",
  "server": {
    "online": true,
    "hostname": "zima-nas",
    "os": "ZimaOS",
    "version": "1.2.0"
  },
  "uptime": {
    "seconds": 1048320,
    "formatted": "12d 03h 12m"
  },
  "cpu": {
    "usage_percent": 23.5,
    "load_1m": 0.42,
    "load_5m": 0.37,
    "load_15m": 0.31,
    "cores": 4
  },
  "memory": {
    "total_bytes": 17179869184,
    "used_bytes": 7030000000,
    "free_bytes": 10149869184,
    "available_bytes": 10149869184,
    "usage_percent": 40.9
  },
  "temperature": {
    "cpu_celsius": 46.0,
    "system_celsius": null,
    "available": true
  },
  "storage": {
    "pools": [
      {
        "name": "main-pool",
        "filesystem": "ext4",
        "mount_point": "/",
        "total_bytes": 2000000000000,
        "used_bytes": 1560000000000,
        "free_bytes": 440000000000,
        "usage_percent": 78.0
      }
    ],
    "disks": [
      {
        "name": "sda",
        "model": "WDC WD2003FZEX",
        "capacity_bytes": 2000398934016,
        "temperature_celsius": 42.0,
        "health": "OK",
        "smart_status": "PASSED"
      }
    ]
  },
  "docker": {
    "total": 7,
    "running": 7,
    "stopped": 0,
    "containers": [
      {
        "name": "Jellyfin",
        "status": "running",
        "image": "jellyfin/jellyfin:latest",
        "uptime": "Up 3 days"
      }
    ]
  },
  "network": {
    "interfaces": [
      {
        "name": "eth0",
        "rx_bytes": 104857600,
        "tx_bytes": 52428800,
        "rx_rate_bytes_sec": 50547000.0,
        "tx_rate_bytes_sec": 13002000.0
      }
    ]
  },
  "health": {
    "status": "healthy",
    "warnings": [],
    "errors": []
  }
}
```
