#include "buzzer_controller.h"

// Patrón rítmico Super Mario Bros (intro clásica):
// Pares de { ON, OFF } en milisegundos
// 1. E (120ms ON, 60ms OFF)
// 2. E (120ms ON, 150ms OFF)
// 3. E (120ms ON, 150ms OFF)
// 4. C (120ms ON, 60ms OFF)
// 5. E (120ms ON, 200ms OFF)
// 6. G (260ms ON, 400ms OFF)
// 7. G bajo (260ms ON, 800ms OFF - pausa de cierre antes de repetir)
static const uint16_t MARIO_RHYTHM[] = {
    120,  60,
    120, 150,
    120, 150,
    120,  60,
    120, 200,
    260, 400,
    260, 800
};
static const uint8_t MARIO_STEPS = sizeof(MARIO_RHYTHM) / sizeof(MARIO_RHYTHM[0]);

BuzzerController::BuzzerController(uint8_t pin)
    : _pin(pin),
      _isAlarming(false),
      _isBeeping(false),
      _pinState(false),
      _alarmStartTime(0),
      _alarmMaxDuration(0),
      _stepEndTime(0),
      _beepEndTime(0),
      _rhythmIndex(0) {}

void BuzzerController::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void BuzzerController::startAlarm(unsigned long maxDurationMs) {
    _isAlarming = true;
    _isBeeping = false;
    _alarmStartTime = millis();
    _alarmMaxDuration = maxDurationMs;
    _rhythmIndex = 0;
    _pinState = true;
    digitalWrite(_pin, HIGH);
    _stepEndTime = millis() + MARIO_RHYTHM[0];
}

void BuzzerController::stop() {
    _isAlarming = false;
    _isBeeping = false;
    _pinState = false;
    _rhythmIndex = 0;
    digitalWrite(_pin, LOW);
}

void BuzzerController::beep(unsigned long durationMs) {
    if (_isAlarming) return; // Prioridad para la alarma
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

    // 2. Manejo de Alarma con ritmo Mario Bros
    if (_isAlarming) {
        // Verificar tiempo máximo de duración (ej. 30 segundos)
        if ((now - _alarmStartTime) >= _alarmMaxDuration) {
            stop();
            return;
        }

        // Avanzar al siguiente paso del ritmo
        if (now >= _stepEndTime) {
            _rhythmIndex++;
            if (_rhythmIndex >= MARIO_STEPS) {
                _rhythmIndex = 0;
            }

            // Pasos pares: sonido (HIGH). Pasos impares: silencio (LOW).
            _pinState = (_rhythmIndex % 2 == 0);
            digitalWrite(_pin, _pinState ? HIGH : LOW);
            _stepEndTime = now + MARIO_RHYTHM[_rhythmIndex];
        }
    }
}
