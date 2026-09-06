#include "network/ota_manager.h"

OTAManager::OTAManager() 
    : m_initialized(false), m_otaInProgress(false), m_lastHTTPCheck(0) {}

OTAManager::~OTAManager() {
    if (m_initialized) {
        ArduinoOTA.end();
    }
}

void OTAManager::begin() {
#if OTA_ENABLED
    // Configure ArduinoOTA for PlatformIO wireless upload
    ArduinoOTA.setPort(OTA_PORT);
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    
    if (strlen(OTA_PASSWORD) > 0) {
        ArduinoOTA.setPassword(OTA_PASSWORD);
    }

    // Set callbacks
    ArduinoOTA.onStart([this]() { this->onOTAStart(); });
    ArduinoOTA.onEnd([this]() { this->onOTAEnd(); });
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) { 
        this->onOTAProgress(progress, total); 
    });
    ArduinoOTA.onError([this](ota_error_t error) { this->onOTAError(error); });

    ArduinoOTA.begin();
    m_initialized = true;
    
    Serial.printf("[OTA] ArduinoOTA ready at %s.local:%d\n", OTA_HOSTNAME, OTA_PORT);
    Serial.printf("[OTA] Firmware version: %s (%s)\n", FIRMWARE_VERSION, FIRMWARE_BUILD_DATE);
#else
    Serial.println("[OTA] OTA disabled in config");
#endif
}

void OTAManager::update() {
#if OTA_ENABLED
    if (m_initialized) {
        ArduinoOTA.handle();
    }
#endif
}

void OTAManager::handleArduinoOTA() {
    // This is handled automatically in update()
    // Can be called explicitly if needed
    update();
}

void OTAManager::onOTAStart() {
    m_otaInProgress = true;
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    notifyStatus(("OTA Start: " + type).c_str());
    Serial.printf("[OTA] Start updating %s\n", type.c_str());
}

void OTAManager::onOTAEnd() {
    m_otaInProgress = false;
    notifyStatus("OTA Complete - Rebooting...");
    Serial.println("[OTA] End");
}

void OTAManager::onOTAProgress(unsigned int progress, unsigned int total) {
    notifyProgress(progress, total);
    if (total > 0) {
        Serial.printf("[OTA] Progress: %u%%\r", (progress * 100) / total);
    }
}

void OTAManager::onOTAError(ota_error_t error) {
    m_otaInProgress = false;
    const char* errorMsg = "Unknown error";
    switch (error) {
        case OTA_AUTH_ERROR: errorMsg = "Auth Failed"; break;
        case OTA_BEGIN_ERROR: errorMsg = "Begin Failed"; break;
        case OTA_CONNECT_ERROR: errorMsg = "Connect Failed"; break;
        case OTA_RECEIVE_ERROR: errorMsg = "Receive Failed"; break;
        case OTA_END_ERROR: errorMsg = "End Failed"; break;
    }
    notifyError(errorMsg);
    Serial.printf("[OTA] Error[%u]: %s\n", error, errorMsg);
}

bool OTAManager::checkAndApplyHTTPUpdate(const String& updateUrl, const String& expectedVersion) {
#if OTA_ENABLED
    if (m_otaInProgress) {
        notifyError("OTA already in progress");
        return false;
    }

    // Optional version check
    if (expectedVersion.length() > 0 && expectedVersion == FIRMWARE_VERSION) {
        notifyStatus("Already on latest version");
        Serial.println("[OTA] Already on latest version");
        return false;
    }

    notifyStatus("Downloading update...");
    Serial.printf("[OTA] HTTP Update from: %s\n", updateUrl.c_str());
    
    return performHTTPUpdate(updateUrl);
#else
    notifyError("OTA disabled");
    return false;
#endif
}

bool OTAManager::performHTTPUpdate(const String& url) {
    WiFiClient client;
    HTTPClient http;
    
    http.begin(client, url);
    http.setTimeout(30000); // 30 second timeout for download
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        notifyError(("HTTP Error: " + String(httpCode)).c_str());
        Serial.printf("[OTA] HTTP GET failed: %d\n", httpCode);
        http.end();
        return false;
    }

    int contentLength = http.getSize();
    if (contentLength <= 0) {
        notifyError("Invalid content length");
        Serial.println("[OTA] Invalid content length");
        http.end();
        return false;
    }

    notifyStatus("Verifying update...");
    
    // Check if we have enough space
    if (!Update.begin(contentLength)) {
        notifyError("Not enough space for update");
        Serial.printf("[OTA] Not enough space: %d bytes needed\n", contentLength);
        http.end();
        return false;
    }

    // Stream the update
    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buffer[1024];
    
    notifyProgress(0, contentLength);
    
    while (http.connected() && written < contentLength) {
        size_t available = stream->available();
        if (available) {
            size_t read = stream->readBytes(buffer, std::min(available, sizeof(buffer)));
            if (read > 0) {
                size_t result = Update.write(buffer, read);
                if (result != read) {
                    notifyError("Write failed");
                    Serial.println("[OTA] Write failed");
                    Update.abort();
                    http.end();
                    return false;
                }
                written += read;
                notifyProgress(written, contentLength);
            }
        }
        delay(1); // Yield to prevent watchdog
    }

    http.end();

    if (written != contentLength) {
        notifyError("Incomplete download");
        Serial.printf("[OTA] Incomplete: %d/%d\n", written, contentLength);
        Update.abort();
        return false;
    }

    notifyStatus("Finalizing update...");
    if (!Update.end(true)) {
        notifyError("Update finalization failed");
        Serial.println("[OTA] Update.end() failed");
        return false;
    }

    notifyStatus("Update complete - Rebooting...");
    Serial.println("[OTA] HTTP Update successful, rebooting...");
    delay(500);
    ESP.restart();
    return true;
}

void OTAManager::notifyProgress(int progress, int total) {
    if (m_progressCb) {
        m_progressCb(progress, total);
    }
}

void OTAManager::notifyStatus(const char* status) {
    if (m_statusCb) {
        m_statusCb(status);
    }
}

void OTAManager::notifyError(const char* error) {
    if (m_errorCb) {
        m_errorCb(error);
    }
}