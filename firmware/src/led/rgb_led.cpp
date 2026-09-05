#include "led/rgb_led.h"
#include "config.h"

RGBController::RGBController() 
    : m_pin(PIN_RGB_LED), m_status(LED_STATUS_OK), m_lastBlink(0), m_blinkState(false) {}

void RGBController::begin(uint8_t pin) {
    m_pin = pin;
    pinMode(m_pin, OUTPUT);
    setStatus(LED_STATUS_OK);
}

void RGBController::setRGB(uint8_t r, uint8_t g, uint8_t b) {
    // Basic RMT / GPIO driver abstraction
    #ifdef RGB_BUILTIN
    neopixelWrite(m_pin, r, g, b);
    #endif
}

void RGBController::setStatus(LedStatus status) {
    m_status = status;
    switch (m_status) {
        case LED_STATUS_OK:
            setRGB(0, 255, 0); // Green
            break;
        case LED_STATUS_WARNING:
            setRGB(255, 200, 0); // Yellow
            break;
        case LED_STATUS_CRITICAL:
            setRGB(255, 0, 0); // Red
            break;
        case LED_STATUS_BUSY:
            setRGB(0, 150, 255); // Blue
            break;
    }
}

void RGBController::update() {
    if (m_status == LED_STATUS_BUSY) {
        if (millis() - m_lastBlink > 400) {
            m_lastBlink = millis();
            m_blinkState = !m_blinkState;
            if (m_blinkState) {
                setRGB(0, 150, 255);
            } else {
                setRGB(0, 20, 60);
            }
        }
    }
}
