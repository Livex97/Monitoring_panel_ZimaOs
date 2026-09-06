#ifndef CONFIG_H
#define CONFIG_H

// Hardware Pin Assignments for Waveshare ESP32-S3-LCD-1.47 (USB-A version)
#define PIN_LCD_MOSI    45
#define PIN_LCD_SCLK    40
#define PIN_LCD_CS      42
#define PIN_LCD_DC      41
#define PIN_LCD_RST     39
#define PIN_LCD_BL      48

#define PIN_RGB_LED     38

#define PIN_BTN_LEFT    0   // BOOT button
#define PIN_BTN_RIGHT   14  // Side button

// Display Dimensions (Portrait Mode)
#define SCREEN_WIDTH    172
#define SCREEN_HEIGHT   320

// Application Configuration
#define API_REFRESH_INTERVAL_MS 3000   // 3 seconds auto-refresh as per mockup
#define HTTP_TIMEOUT_MS         2500   // 2.5 seconds timeout
#define BUTTON_DEBOUNCE_MS      50

// Firmware Version (update on each release)
#define FIRMWARE_VERSION        "1.1.1"
#define FIRMWARE_BUILD_DATE     __DATE__ " " __TIME__

// OTA Configuration
#define OTA_ENABLED             1
#define OTA_HOSTNAME            "zimaos-panel"
#define OTA_PASSWORD            ""  // Set a password for security if desired
#define OTA_PORT                3232  // Default ArduinoOTA port
#define OTA_CHECK_INTERVAL_MS   3600000  // Check for HTTP OTA updates every hour (optional)

// Network Configuration
#define USE_STATIC_IP           1
#define STATIC_IP               IPAddress(192, 168, 1, 38)
#define STATIC_GATEWAY          IPAddress(192, 168, 1, 1)
#define STATIC_SUBNET           IPAddress(255, 255, 255, 0)
#define STATIC_DNS1             IPAddress(8, 8, 8, 8)
#define STATIC_DNS2             IPAddress(8, 8, 4, 4)

// Web Server Configuration
#define ENABLE_WEB_SERVER       1
#define WEB_SERVER_PORT         80

#endif // CONFIG_H
