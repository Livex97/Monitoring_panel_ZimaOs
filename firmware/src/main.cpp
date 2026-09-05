#include <Arduino.h>
#include "config.h"
#include "network/wifi_manager.h"
#include "api/api_client.h"
#include "input/button_handler.h"
#include "led/rgb_led.h"
#include "ui/ui_manager.h"

// Secrets header (Ensure firmware/include/secrets.h exists)
#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

WiFiManager wifiManager;
APIClient apiClient;
ButtonHandler buttonHandler;
RGBController rgbController;
UIManager uiManager;

SystemMetrics currentMetrics;
uint32_t lastApiFetch = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- ZimaOS NAS Monitoring Panel Firmware v1.0.0 ---");

    buttonHandler.begin(PIN_BTN_LEFT, PIN_BTN_RIGHT);
    rgbController.begin(PIN_RGB_LED);
    uiManager.begin();

    wifiManager.begin(WIFI_SSID, WIFI_PASSWORD);
    apiClient.begin(ZIMAOS_MONITOR_HOST, ZIMAOS_MONITOR_PORT, ZIMAOS_API_KEY);

    rgbController.setStatus(LED_STATUS_BUSY);
}

void loop() {
    wifiManager.update();
    rgbController.update();

    // Check physical button inputs
    ButtonEvent btnEvt = buttonHandler.update();
    if (btnEvt == BTN_EVENT_LEFT_CLICK) {
        Serial.println("[Button] Left clicked -> Previous Page");
        uiManager.prevPage();
    } else if (btnEvt == BTN_EVENT_RIGHT_CLICK) {
        Serial.println("[Button] Right clicked -> Next Page");
        uiManager.nextPage();
    } else if (btnEvt == BTN_EVENT_BOTH_LONG_PRESS) {
        Serial.println("[Button] Both held 3s -> Resetting...");
        ESP.restart();
    }

    // Fetch API Metrics periodically when Wi-Fi is connected
    if (wifiManager.isConnected()) {
        if (millis() - lastApiFetch >= API_REFRESH_INTERVAL_MS) {
            lastApiFetch = millis();
            
            bool success = apiClient.fetchMetrics(currentMetrics);
            if (success) {
                uiManager.updateData(currentMetrics);

                if (currentMetrics.health.status == "healthy") {
                    rgbController.setStatus(LED_STATUS_OK);
                } else if (currentMetrics.health.status == "warning") {
                    rgbController.setStatus(LED_STATUS_WARNING);
                } else {
                    rgbController.setStatus(LED_STATUS_CRITICAL);
                }
            } else {
                uiManager.showOfflineScreen("Failed to reach ZimaOS API");
                rgbController.setStatus(LED_STATUS_CRITICAL);
            }
        }
    } else {
        uiManager.showOfflineScreen("Connecting to Wi-Fi...");
        rgbController.setStatus(LED_STATUS_BUSY);
    }

    lv_timer_handler();
    delay(5);
}
