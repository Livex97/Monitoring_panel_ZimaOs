#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "model/data_model.h"

class APIClient {
public:
    APIClient();
    void begin(const char* host, uint16_t port, const char* apiKey = "");
    bool fetchMetrics(SystemMetrics& outMetrics);

private:
    String m_host;
    uint16_t m_port;
    String m_apiKey;
    HTTPClient m_http;
};

#endif // API_CLIENT_H
