#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
public:
    WiFiManager();
    void begin(const char* ssid, const char* password);
    void update();
    bool isConnected() const;
    int getRSSI() const;
    String getIP() const;

private:
    const char* m_ssid;
    const char* m_password;
    uint32_t m_lastReconnectAttempt;
    bool m_connected;
};

#endif // WIFI_MANAGER_H
