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

#endif // CONFIG_H
