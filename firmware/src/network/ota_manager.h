#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include "config.h"

class OTAManager {
public:
    OTAManager();
    ~OTAManager();

    void begin();
    void update();

    // ArduinoOTA (PlatformIO upload via WiFi)
    void handleArduinoOTA();

    // HTTP OTA - Check and apply update from server
    // Returns true if update was successful and device will reboot
    bool checkAndApplyHTTPUpdate(const String& updateUrl, const String& expectedVersion = "");

    // Getters
    const char* getCurrentVersion() const { return FIRMWARE_VERSION; }
    const char* getBuildDate() const { return FIRMWARE_BUILD_DATE; }
    bool isOTAInProgress() const { return m_otaInProgress; }

    // Callbacks for UI feedback
    using ProgressCallback = void (*)(int progress, int total);
    using StatusCallback = void (*)(const char* status);
    using ErrorCallback = void (*)(const char* error);

    void setProgressCallback(ProgressCallback cb) { m_progressCb = cb; }
    void setStatusCallback(StatusCallback cb) { m_statusCb = cb; }
    void setErrorCallback(ErrorCallback cb) { m_errorCb = cb; }

private:
    bool m_initialized;
    bool m_otaInProgress;
    uint32_t m_lastHTTPCheck;
    
    ProgressCallback m_progressCb = nullptr;
    StatusCallback m_statusCb = nullptr;
    ErrorCallback m_errorCb = nullptr;

    void onOTAStart();
    void onOTAEnd();
    void onOTAProgress(unsigned int progress, unsigned int total);
    void onOTAError(ota_error_t error);
    
    bool performHTTPUpdate(const String& url);
    void notifyProgress(int progress, int total);
    void notifyStatus(const char* status);
    void notifyError(const char* error);
};

#endif // OTA_MANAGER_H