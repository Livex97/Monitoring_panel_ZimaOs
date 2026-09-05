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
bool isShowingOffline = false;
bool wasConnected = false;

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0);
    delay(500);
    Serial.println("\n--- ZimaOS NAS Monitoring Panel Firmware v1.0.0 ---");

    buttonHandler.begin(PIN_BTN_LEFT, PIN_BTN_RIGHT);
    rgbController.begin(PIN_RGB_LED);
    uiManager.begin();

    wifiManager.begin(WIFI_SSID, WIFI_PASSWORD);
    apiClient.begin(ZIMAOS_MONITOR_HOST, ZIMAOS_MONITOR_PORT, ZIMAOS_API_KEY);

    rgbController.setStatus(LED_STATUS_BUSY);
    uiManager.showOfflineScreen("Connecting to Wi-Fi...");
    isShowingOffline = true;
}

void loop() {
    wifiManager.update();
    rgbController.update();

    bool connected = wifiManager.isConnected();

    // Check physical button inputs
    ButtonEvent btnEvt = buttonHandler.update();
    if (btnEvt == BTN_EVENT_LEFT_CLICK) {
        Serial.println("[Button] Left clicked -> Previous Page");
        uiManager.prevPage();
        isShowingOffline = false;
    } else if (btnEvt == BTN_EVENT_RIGHT_CLICK) {
        Serial.println("[Button] Right clicked -> Next Page");
        uiManager.nextPage();
        isShowingOffline = false;
    } else if (btnEvt == BTN_EVENT_BOTH_LONG_PRESS) {
        Serial.println("[Button] Both held 3s -> Resetting...");
        ESP.restart();
    }

    // Fetch API Metrics periodically when Wi-Fi is connected
    if (connected) {
        if (!wasConnected) {
            wasConnected = true;
            Serial.println("[System] Wi-Fi connected!");
        }

        if (millis() - lastApiFetch >= API_REFRESH_INTERVAL_MS) {
            Serial.println("[System] Attempting API fetch...");
            lastApiFetch = millis();

            bool success = apiClient.fetchMetrics(currentMetrics);
            Serial.printf("[System] API fetch returned %s, valid=%s, health='%s'\n",
                success ? "true" : "false",
                currentMetrics.valid ? "true" : "false",
                currentMetrics.health.status.c_str());

            if (success && currentMetrics.valid) {
                // Switch to overview BEFORE updating widgets so they are visible
                if (isShowingOffline) {
                    uiManager.switchPage(0);
                    isShowingOffline = false;
                    Serial.println("[System] Switched to Overview screen.");
                }
                uiManager.updateData(currentMetrics);

                String hs = currentMetrics.health.status;
                if (hs == "healthy" || hs == "") {
                    rgbController.setStatus(LED_STATUS_OK);
                } else if (hs == "warning") {
                    rgbController.setStatus(LED_STATUS_WARNING);
                } else {
                    rgbController.setStatus(LED_STATUS_CRITICAL);
                }
            } else {
                if (!isShowingOffline) {
                    uiManager.showOfflineScreen("Failed to reach ZimaOS API");
                    isShowingOffline = true;
                }
                rgbController.setStatus(LED_STATUS_CRITICAL);
            }
        }
    } else {
        if (!isShowingOffline) {
            uiManager.showOfflineScreen("Connecting to Wi-Fi...");
            rgbController.setStatus(LED_STATUS_BUSY);
            isShowingOffline = true;
            wasConnected = false;
        }
    }

    lv_timer_handler();
    delay(5);
}
