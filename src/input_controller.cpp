#include "input_controller.h"

InputButton::InputButton(uint8_t pin, unsigned long debounceMs, unsigned long longPressMs)
    : _pin(pin),
      _debounceMs(debounceMs),
      _longPressMs(longPressMs),
      _lastPhysicalState(HIGH),
      _debouncedState(HIGH),
      _lastDebounceTime(0),
      _isPressed(false),
      _pressStartTime(0),
      _longPressTriggered(false) {}

void InputButton::begin() {
    pinMode(_pin, INPUT_PULLUP);
    delay(5);
    _lastPhysicalState = digitalRead(_pin);
    _debouncedState = _lastPhysicalState;
    _isPressed = false;
    _longPressTriggered = false;
}

ButtonEvent InputButton::update() {
    bool reading = digitalRead(_pin);
    unsigned long now = millis();
    ButtonEvent event = BTN_EVENT_NONE;

    if (reading != _lastPhysicalState) {
        _lastDebounceTime = now;
        _lastPhysicalState = reading;
    }

    if ((now - _lastDebounceTime) >= _debounceMs) {
        if (reading != _debouncedState) {
            _debouncedState = reading;

            // Transición a presionado (activo en LOW)
            if (_debouncedState == LOW) {
                _isPressed = true;
                _pressStartTime = now;
                _longPressTriggered = false;
            } else {
                // Transición a liberado (HIGH)
                if (_isPressed) {
                    _isPressed = false;
                    if (!_longPressTriggered && (now - _pressStartTime < _longPressMs)) {
                        event = BTN_EVENT_SHORT_CLICK;
                    }
                }
            }
        }
    }

    // Detección de pulsación sostenida (long press) mientras sigue presionado
    if (_isPressed && !_longPressTriggered) {
        if ((now - _pressStartTime) >= _longPressMs) {
            _longPressTriggered = true;
            event = BTN_EVENT_LONG_PRESS;
        }
    }

    return event;
}

InputController::InputController(uint8_t pinPrev, uint8_t pinNext, uint8_t pinAction, unsigned long longPressMs)
    : _btnPrev(pinPrev, 40, longPressMs),
      _btnNext(pinNext, 40, longPressMs),
      _btnAction(pinAction, 40, longPressMs) {}

void InputController::begin() {
    _btnPrev.begin();
    _btnNext.begin();
    _btnAction.begin();
}

ButtonEvent InputController::updatePrev() {
    return _btnPrev.update();
}

ButtonEvent InputController::updateNext() {
    return _btnNext.update();
}

ButtonEvent InputController::updateAction() {
    return _btnAction.update();
}
