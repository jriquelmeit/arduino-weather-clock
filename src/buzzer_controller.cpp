#include "buzzer_controller.h"

BuzzerController::BuzzerController(uint8_t pin)
    : _pin(pin),
      _isAlarming(false),
      _isBeeping(false),
      _pinState(false),
      _alarmStartTime(0),
      _alarmMaxDuration(0),
      _lastToggleTime(0),
      _beepEndTime(0) {}

void BuzzerController::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void BuzzerController::startAlarm(unsigned long maxDurationMs) {
    _isAlarming = true;
    _isBeeping = false;
    _alarmStartTime = millis();
    _alarmMaxDuration = maxDurationMs;
    _lastToggleTime = millis();
    _pinState = true;
    digitalWrite(_pin, HIGH);
}

void BuzzerController::stop() {
    _isAlarming = false;
    _isBeeping = false;
    _pinState = false;
    digitalWrite(_pin, LOW);
}

void BuzzerController::beep(unsigned long durationMs) {
    if (_isAlarming) return; // Prioridad para alarma
    _isBeeping = true;
    _beepEndTime = millis() + durationMs;
    _pinState = true;
    digitalWrite(_pin, HIGH);
}

bool BuzzerController::isAlarming() const {
    return _isAlarming;
}

void BuzzerController::update() {
    unsigned long now = millis();

    // 1. Manejo de Beep corto
    if (_isBeeping) {
        if (now >= _beepEndTime) {
            _isBeeping = false;
            _pinState = false;
            digitalWrite(_pin, LOW);
        }
        return;
    }

    // 2. Manejo de Alarma intermitente
    if (_isAlarming) {
        // Verificar tiempo máximo de duración
        if ((now - _alarmStartTime) >= _alarmMaxDuration) {
            stop();
            return;
        }

        unsigned long elapsed = now - _lastToggleTime;
        if (_pinState && elapsed >= ALARM_BEEP_ON_MS) {
            _pinState = false;
            _lastToggleTime = now;
            digitalWrite(_pin, LOW);
        } else if (!_pinState && elapsed >= ALARM_BEEP_OFF_MS) {
            _pinState = true;
            _lastToggleTime = now;
            digitalWrite(_pin, HIGH);
        }
    }
}
