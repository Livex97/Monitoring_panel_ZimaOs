# ESP32-S3 Firmware — ZimaOS NAS Monitoring Panel

## Prerequisites

- [PlatformIO CLI](https://docs.platformio.org/en/latest/cli/index.html) or VS Code PlatformIO extension.
- Waveshare ESP32-S3-LCD-1.47 (USB-A version).

## Configuration

1. Copy `include/secrets.example.h` to `include/secrets.h`:
   ```bash
   cp include/secrets.example.h include/secrets.h
   ```
2. Edit `include/secrets.h` with your Wi-Fi credentials and ZimaOS host IP.

## Build and Flash

```bash
# Build firmware
pio run

# Flash to device
pio run --target upload

# Open Serial Monitor
pio device monitor
```
