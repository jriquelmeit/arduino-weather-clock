#pragma once

#include <Arduino.h>

enum ButtonEvent {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SHORT_CLICK,
    BTN_EVENT_LONG_PRESS
};

class InputButton {
public:
    InputButton(uint8_t pin, unsigned long debounceMs = 25, unsigned long longPressMs = 1000, bool triggerOnPress = false);
    void begin();
    ButtonEvent update();

private:
    uint8_t _pin;
    unsigned long _debounceMs;
    unsigned long _longPressMs;
    bool _triggerOnPress;

    bool _lastPhysicalState;
    bool _debouncedState;
    unsigned long _lastDebounceTime;

    bool _isPressed;
    unsigned long _pressStartTime;
    bool _longPressTriggered;
};

class InputController {
public:
    InputController(uint8_t pinPrev, uint8_t pinNext, uint8_t pinAction, unsigned long longPressMs = 1000);
    void begin();

    ButtonEvent updatePrev();
    ButtonEvent updateNext();
    ButtonEvent updateAction();

private:
    InputButton _btnPrev;
    InputButton _btnNext;
    InputButton _btnAction;
};
