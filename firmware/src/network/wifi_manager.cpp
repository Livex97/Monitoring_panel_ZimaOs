#include "network/wifi_manager.h"

WiFiManager::WiFiManager() 
    : m_ssid(nullptr), m_password(nullptr), m_lastReconnectAttempt(0), m_connected(false) {}

void WiFiManager::begin(const char* ssid, const char* password) {
    m_ssid = ssid;
    m_password = password;
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(m_ssid, m_password);
    Serial.printf("[WiFi] Connecting to %s...\n", m_ssid);
}

void WiFiManager::update() {
    bool current = (WiFi.status() == WL_CONNECTED);
    if (current != m_connected) {
        m_connected = current;
        if (m_connected) {
            Serial.printf("[WiFi] Connected! IP: %s, RSSI: %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
        } else {
            Serial.println("[WiFi] Disconnected.");
        }
    }

    if (!m_connected && (millis() - m_lastReconnectAttempt > 10000)) {
        m_lastReconnectAttempt = millis();
        Serial.println("[WiFi] Reconnecting...");
        WiFi.reconnect();
    }
}

bool WiFiManager::isConnected() const {
    return m_connected;
}

int WiFiManager::getRSSI() const {
    return m_connected ? WiFi.RSSI() : 0;
}

String WiFiManager::getIP() const {
    return m_connected ? WiFi.localIP().toString() : "0.0.0.0";
}
