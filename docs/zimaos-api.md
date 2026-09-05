# ZimaOS & System API Mapping

## Overview

The `zimaos-monitor-api` backend queries official ZimaOS / CasaOS REST APIs when available and uses Linux system fallbacks (`/proc`, `/sys`, `psutil`, `docker.sock`) when required.

## Metric Sources Table

| Metric | Source | API Endpoint / Method | Availability | Fallback |
| :--- | :--- | :--- | :--- | :--- |
| System Hostname | ZimaOS API / OS | `GET /v1/sys/hardware` | High | `socket.gethostname()` |
| OS Version | ZimaOS API / OS | `GET /v1/sys/version` or `/etc/os-release` | High | Reading `/etc/os-release` |
| Uptime | Linux `/proc/uptime` | `psutil.boot_time()` | Guaranteed | `/proc/uptime` |
| CPU Usage % | Linux Kernel | `psutil.cpu_percent()` | Guaranteed | `/proc/stat` |
| CPU Load Averages | Linux Kernel | `os.getloadavg()` | Guaranteed | `/proc/loadavg` |
| CPU Cores | Linux Kernel | `psutil.cpu_count()` | Guaranteed | `/proc/cpuinfo` |
| RAM Usage | Linux Kernel | `psutil.virtual_memory()` | Guaranteed | `/proc/meminfo` |
| CPU Temperature | Thermal Sysfs | `psutil.sensors_temperatures()` | Hardware dependent | `/sys/class/thermal/thermal_zone*/temp` |
| Storage Usage | Storage Engine | `psutil.disk_partitions()`, `psutil.disk_usage()` | High | `statvfs` |
| Disk SMART & Health | smartctl / ZimaOS API | `smartctl -j -H /dev/sdX` / ZimaOS Storage API | Hardware dependent | `null` (available: false) |
| Docker Container List | Docker Engine | `GET /containers/json` via `/var/run/docker.sock` | High | Docker SDK for Python |
| Network RX/TX | Network Interfaces | `psutil.net_io_counters(pernic=True)` | High | Time delta sampling |

## Authentication & Security

- If `MONITOR_API_KEY` is configured in environment variables, requests to `/api/v1/*` must include the HTTP header:
  `X-API-Key: <YOUR_CONFIGURED_KEY>`
- Health check endpoint `/health` is unauthenticated to support Docker healthchecks.
