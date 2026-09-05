#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

enum ButtonEvent {
    BTN_EVENT_NONE,
    BTN_EVENT_LEFT_CLICK,
    BTN_EVENT_RIGHT_CLICK,
    BTN_EVENT_BOTH_LONG_PRESS
};

class ButtonHandler {
public:
    ButtonHandler();
    void begin(uint8_t pinLeft, uint8_t pinRight);
    ButtonEvent update();

private:
    uint8_t m_pinLeft;
    uint8_t m_pinRight;
    
    bool m_lastStateLeft;
    bool m_lastStateRight;
    uint32_t m_lastPressLeft;
    uint32_t m_lastPressRight;
};

#endif // BUTTON_HANDLER_H
