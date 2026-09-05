#include "input/button_handler.h"
#include "config.h"

ButtonHandler::ButtonHandler() 
    : m_pinLeft(0), m_pinRight(14), m_lastStateLeft(HIGH), m_lastStateRight(HIGH),
      m_lastPressLeft(0), m_lastPressRight(0) {}

void ButtonHandler::begin(uint8_t pinLeft, uint8_t pinRight) {
    m_pinLeft = pinLeft;
    m_pinRight = pinRight;
    pinMode(m_pinLeft, INPUT_PULLUP);
    pinMode(m_pinRight, INPUT_PULLUP);
}

ButtonEvent ButtonHandler::update() {
    bool stateLeft = digitalRead(m_pinLeft);
    bool stateRight = digitalRead(m_pinRight);
    uint32_t now = millis();
    ButtonEvent event = BTN_EVENT_NONE;

    // Both long press check (3 seconds)
    if (stateLeft == LOW && stateRight == LOW) {
        if (m_lastPressLeft > 0 && m_lastPressRight > 0 && (now - m_lastPressLeft > 3000)) {
            m_lastPressLeft = now + 5000;
            return BTN_EVENT_BOTH_LONG_PRESS;
        }
    }

    // Left button click
    if (m_lastStateLeft == HIGH && stateLeft == LOW) {
        m_lastPressLeft = now;
    } else if (m_lastStateLeft == LOW && stateLeft == HIGH) {
        if (now - m_lastPressLeft > BUTTON_DEBOUNCE_MS && now - m_lastPressLeft < 1500) {
            event = BTN_EVENT_LEFT_CLICK;
        }
    }

    // Right button click
    if (m_lastStateRight == HIGH && stateRight == LOW) {
        m_lastPressRight = now;
    } else if (m_lastStateRight == LOW && stateRight == HIGH) {
        if (now - m_lastPressRight > BUTTON_DEBOUNCE_MS && now - m_lastPressRight < 1500) {
            event = BTN_EVENT_RIGHT_CLICK;
        }
    }

    m_lastStateLeft = stateLeft;
    m_lastStateRight = stateRight;
    return event;
}
