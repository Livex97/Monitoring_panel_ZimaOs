#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"

class WebServerManager {
public:
    WebServerManager();
    ~WebServerManager();

    void begin();
    void handleClient();

    // Register a handler for a specific endpoint
    void on(const char* uri, std::function<void()> handler);

    // Get server instance for advanced configuration
    WebServer& getServer() { return m_server; }

private:
    WebServer m_server;
    bool m_initialized;
};

#endif // WEB_SERVER_H