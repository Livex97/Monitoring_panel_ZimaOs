#include "network/web_server.h"

WebServerManager::WebServerManager()
    : m_server(WEB_SERVER_PORT), m_initialized(false) {}

WebServerManager::~WebServerManager() {
    if (m_initialized) {
        m_server.stop();
    }
}

void WebServerManager::begin() {
#if ENABLE_WEB_SERVER
    if (m_initialized) return;

    // Register default endpoints
    on("/api/version", [this]() {
        String json = "{\"version\":\"" + String(FIRMWARE_VERSION) + "\",\"build_date\":\"" + String(FIRMWARE_BUILD_DATE) + "\"}";
        m_server.send(200, "application/json", json);
    });

    on("/api/status", [this]() {
        String json = "{\"wifi_connected\":";
        json += (WiFi.status() == WL_CONNECTED) ? "true" : "false";
        json += ",\"ip_address\":\"" + WiFi.localIP().toString() + "\",\"rssi\":" + String(WiFi.RSSI()) + ",\"version\":\"" + String(FIRMWARE_VERSION) + "\"}";
        m_server.send(200, "application/json", json);
    });

    on("/", [this]() {
        String html = "<div style='font-family: Arial, sans-serif; margin: 20px;'>\n";
        html += "<h1>ZimaOS NAS Monitoring Panel</h1>\n";
        html += "<div style='background: #f0f0f0; padding: 15px; border-radius: 5px; margin: 10px 0;'>\n";
        html += "<span style='font-weight: bold;'>Firmware Version:</span> <span style='color: #0066cc;'>" + String(FIRMWARE_VERSION) + "</span></div>\n";
        html += "<div style='background: #f0f0f0; padding: 15px; border-radius: 5px; margin: 10px 0;'>\n";
        html += "<span style='font-weight: bold;'>Build Date:</span> <span style='color: #0066cc;'>" + String(FIRMWARE_BUILD_DATE) + "</span></div>\n";
        html += "<div style='background: #f0f0f0; padding: 15px; border-radius: 5px; margin: 10px 0;'>\n";
        html += "<span style='font-weight: bold;'>IP Address:</span> <span style='color: #0066cc;'>" + WiFi.localIP().toString() + "</span></div>\n";
        html += "<div style='background: #f0f0f0; padding: 15px; border-radius: 5px; margin: 10px 0;'>\n";
        html += "<span style='font-weight: bold;'>WiFi Status:</span> <span style='color: #0066cc;'>" + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected") + "</span></div>\n";
        html += "<div style='background: #f0f0f0; padding: 15px; border-radius: 5px; margin: 10px 0;'>\n";
        html += "<span style='font-weight: bold;'>Signal Strength:</span> <span style='color: #0066cc;'>" + String(WiFi.RSSI()) + " dBm</span></div>\n";
        html += "<hr><p><small>API Endpoints:</small></p>\n";
        html += "<ul><li><code>GET /api/version</code> - Firmware version info</li>\n";
        html += "<li><code>GET /api/status</code> - System status</li></ul></div>";
        
        m_server.send(200, "text/html", html);
    });

    m_server.begin();
    m_initialized = true;
    Serial.printf("[Web Server] Started on port %d\n", WEB_SERVER_PORT);
    Serial.printf("[Web Server] Access at http://%s\n", WiFi.localIP().toString().c_str());
#else
    Serial.println("[Web Server] Web server disabled in config");
#endif
}

void WebServerManager::handleClient() {
#if ENABLE_WEB_SERVER
    if (m_initialized) {
        m_server.handleClient();
    }
#endif
}

void WebServerManager::on(const char* uri, std::function<void()> handler) {
#if ENABLE_WEB_SERVER
    m_server.on(uri, handler);
#endif
}