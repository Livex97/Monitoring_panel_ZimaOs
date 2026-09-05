#ifndef RGB_LED_H
#define RGB_LED_H

#include <Arduino.h>

enum LedStatus {
    LED_STATUS_OK,         // Green
    LED_STATUS_WARNING,    // Yellow
    LED_STATUS_CRITICAL,   // Red
    LED_STATUS_BUSY        // Blue
};

class RGBController {
public:
    RGBController();
    void begin(uint8_t pin);
    void setStatus(LedStatus status);
    void update();

private:
    uint8_t m_pin;
    LedStatus m_status;
    uint32_t m_lastBlink;
    bool m_blinkState;
    
    void setRGB(uint8_t r, uint8_t g, uint8_t b);
};

#endif // RGB_LED_H
