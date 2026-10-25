#include "buzzer_controller.h"

// ----------------------------------------------------
// 1. MELODÍA: Star Wars - La Marcha Imperial
// ----------------------------------------------------
static const MelodyStep MELODY_STAR_WARS_STEPS[] = {
    // Frase 1 (Tema Principal)
    { NOTE_A4, 500 }, { 0, 50 },
    { NOTE_A4, 500 }, { 0, 50 },
    { NOTE_A4, 500 }, { 0, 50 },
    { NOTE_F4, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_A4, 500 }, { 0, 50 },
    { NOTE_F4, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_A4, 650 }, { 0, 450 },

    // Frase 2 (Respuesta Aguda)
    { NOTE_E5, 500 }, { 0, 50 },
    { NOTE_E5, 500 }, { 0, 50 },
    { NOTE_E5, 500 }, { 0, 50 },
    { NOTE_F5, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_GS4, 500 }, { 0, 50 },
    { NOTE_F4, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_A4, 650 }, { 0, 450 },

    // Frase 3 (Salto de Octava y Descenso)
    { NOTE_A5, 500 }, { 0, 50 },
    { NOTE_A4, 300 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_A5, 500 }, { 0, 50 },
    { NOTE_GS5, 325 }, { 0, 50 },
    { NOTE_G5, 175 }, { 0, 50 },
    { NOTE_FS5, 125 }, { 0, 50 },
    { NOTE_F5, 125 }, { 0, 50 },
    { NOTE_FS5, 250 }, { 0, 250 },
    { NOTE_AS4, 250 }, { 0, 50 },
    { NOTE_DS5, 400 }, { 0, 50 },
    { NOTE_D5, 200 }, { 0, 50 },
    { NOTE_CS5, 150 }, { 0, 50 },
    { NOTE_C5, 125 }, { 0, 50 },
    { NOTE_B4, 125 }, { 0, 50 },
    { NOTE_C5, 250 }, { 0, 250 },
    { NOTE_F4, 250 }, { 0, 50 },
    { NOTE_GS4, 450 }, { 0, 50 },
    { NOTE_F4, 350 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_C5, 450 }, { 0, 50 },
    { NOTE_A4, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_E5, 650 }, { 0, 450 },

    // Frase 4 (Variación Final y Cierre)
    { NOTE_A5, 500 }, { 0, 50 },
    { NOTE_A4, 300 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_A5, 500 }, { 0, 50 },
    { NOTE_GS5, 325 }, { 0, 50 },
    { NOTE_G5, 175 }, { 0, 50 },
    { NOTE_FS5, 125 }, { 0, 50 },
    { NOTE_F5, 125 }, { 0, 50 },
    { NOTE_FS5, 250 }, { 0, 250 },
    { NOTE_AS4, 250 }, { 0, 50 },
    { NOTE_DS5, 400 }, { 0, 50 },
    { NOTE_D5, 200 }, { 0, 50 },
    { NOTE_CS5, 150 }, { 0, 50 },
    { NOTE_C5, 125 }, { 0, 50 },
    { NOTE_B4, 125 }, { 0, 50 },
    { NOTE_C5, 250 }, { 0, 250 },
    { NOTE_F4, 250 }, { 0, 50 },
    { NOTE_GS4, 450 }, { 0, 50 },
    { NOTE_F4, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_A4, 500 }, { 0, 50 },
    { NOTE_F4, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_A4, 750 }, { 0, 800 }
};
static const uint16_t STAR_WARS_LENGTH = sizeof(MELODY_STAR_WARS_STEPS) / sizeof(MELODY_STAR_WARS_STEPS[0]);

// ----------------------------------------------------
// 2. MELODÍA: Super Mario Bros (Overworld Theme)
// ----------------------------------------------------
static const MelodyStep MELODY_MARIO_STEPS[] = {
    // Intro
    { NOTE_E5, 130 }, { 0, 50 },
    { NOTE_E5, 130 }, { 0, 150 },
    { NOTE_E5, 130 }, { 0, 150 },
    { NOTE_C5, 130 }, { 0, 50 },
    { NOTE_E5, 130 }, { 0, 150 },
    { NOTE_G5, 250 }, { 0, 250 },
    { NOTE_G4, 250 }, { 0, 350 },

    // Parte A
    { NOTE_C5, 250 }, { 0, 100 },
    { NOTE_G4, 250 }, { 0, 100 },
    { NOTE_E4, 250 }, { 0, 100 },
    { NOTE_A4, 200 }, { 0, 60 },
    { NOTE_B4, 200 }, { 0, 60 },
    { NOTE_AS4, 180 }, { 0, 60 },
    { NOTE_A4, 200 }, { 0, 60 },
    { NOTE_G4, 180 }, { 0, 60 },
    { NOTE_E5, 200 }, { 0, 60 },
    { NOTE_G5, 200 }, { 0, 60 },
    { NOTE_A5, 250 }, { 0, 60 },
    { NOTE_F5, 180 }, { 0, 60 },
    { NOTE_G5, 200 }, { 0, 60 },
    { NOTE_E5, 200 }, { 0, 60 },
    { NOTE_C5, 180 }, { 0, 60 },
    { NOTE_D5, 180 }, { 0, 60 },
    { NOTE_B4, 250 }, { 0, 150 },

    // Parte A (Repetición)
    { NOTE_C5, 250 }, { 0, 100 },
    { NOTE_G4, 250 }, { 0, 100 },
    { NOTE_E4, 250 }, { 0, 100 },
    { NOTE_A4, 200 }, { 0, 60 },
    { NOTE_B4, 200 }, { 0, 60 },
    { NOTE_AS4, 180 }, { 0, 60 },
    { NOTE_A4, 200 }, { 0, 60 },
    { NOTE_G4, 180 }, { 0, 60 },
    { NOTE_E5, 200 }, { 0, 60 },
    { NOTE_G5, 200 }, { 0, 60 },
    { NOTE_A5, 250 }, { 0, 60 },
    { NOTE_F5, 180 }, { 0, 60 },
    { NOTE_G5, 200 }, { 0, 60 },
    { NOTE_E5, 200 }, { 0, 60 },
    { NOTE_C5, 180 }, { 0, 60 },
    { NOTE_D5, 180 }, { 0, 60 },
    { NOTE_B4, 300 }, { 0, 250 },

    // Parte B (Descensos y saltos)
    { NOTE_G5, 180 }, { 0, 50 },
    { NOTE_FS5, 180 }, { 0, 50 },
    { NOTE_F5, 180 }, { 0, 50 },
    { NOTE_DS5, 200 }, { 0, 50 },
    { NOTE_E5, 200 }, { 0, 100 },
    { NOTE_GS4, 150 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_C5, 180 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_D5, 220 }, { 0, 150 },

    { NOTE_G5, 180 }, { 0, 50 },
    { NOTE_FS5, 180 }, { 0, 50 },
    { NOTE_F5, 180 }, { 0, 50 },
    { NOTE_DS5, 200 }, { 0, 50 },
    { NOTE_E5, 200 }, { 0, 100 },
    { NOTE_C6, 250 }, { 0, 50 },
    { NOTE_C6, 200 }, { 0, 50 },
    { NOTE_C6, 350 }, { 0, 250 },

    { NOTE_G5, 180 }, { 0, 50 },
    { NOTE_FS5, 180 }, { 0, 50 },
    { NOTE_F5, 180 }, { 0, 50 },
    { NOTE_DS5, 200 }, { 0, 50 },
    { NOTE_E5, 200 }, { 0, 100 },
    { NOTE_GS4, 150 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_C5, 180 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_D5, 250 }, { 0, 150 },

    { NOTE_DS5, 300 }, { 0, 60 },
    { NOTE_D5, 300 }, { 0, 60 },
    { NOTE_C5, 450 }, { 0, 600 }
};
static const uint16_t MARIO_LENGTH = sizeof(MELODY_MARIO_STEPS) / sizeof(MELODY_MARIO_STEPS[0]);

BuzzerController::BuzzerController(uint8_t pin, uint8_t melodyType)
    : _pin(pin),
      _melodyType(melodyType),
      _isAlarming(false),
      _isBeeping(false),
      _alarmStartTime(0),
      _alarmMaxDuration(0),
      _stepEndTime(0),
      _beepEndTime(0),
      _melodyIndex(0) {}

void BuzzerController::begin() {
    pinMode(_pin, OUTPUT);
    noTone(_pin);
    digitalWrite(_pin, LOW);
}

void BuzzerController::setMelodyType(uint8_t melodyType) {
    _melodyType = melodyType;
}

const MelodyStep* BuzzerController::getActiveMelody() const {
    if (_melodyType == MELODY_MARIO) {
        return MELODY_MARIO_STEPS;
    }
    return MELODY_STAR_WARS_STEPS;
}

uint16_t BuzzerController::getActiveMelodyLength() const {
    if (_melodyType == MELODY_MARIO) {
        return MARIO_LENGTH;
    }
    return STAR_WARS_LENGTH;
}

void BuzzerController::playStartupMelody(int stopPin1, int stopPin2, int stopPin3) {
    const MelodyStep* melody = getActiveMelody();
    uint16_t length = getActiveMelodyLength();

    for (uint16_t i = 0; i < length; i++) {
        uint16_t freq = melody[i].freq;
        uint16_t dur = melody[i].duration;

        if (freq > 0) {
            tone(_pin, freq);
        } else {
            noTone(_pin);
        }

        unsigned long startWait = millis();
        while (millis() - startWait < dur) {
            delay(15);
            yield();

            // Si se presiona algún botón durante la prueba, silenciar de inmediato
            if ((stopPin1 >= 0 && digitalRead(stopPin1) == LOW) ||
                (stopPin2 >= 0 && digitalRead(stopPin2) == LOW) ||
                (stopPin3 >= 0 && digitalRead(stopPin3) == LOW)) {
                stop();
                return;
            }
        }
    }
    noTone(_pin);
    digitalWrite(_pin, LOW);
}

void BuzzerController::startAlarm(unsigned long maxDurationMs) {
    const MelodyStep* melody = getActiveMelody();

    _isAlarming = true;
    _isBeeping = false;
    _alarmStartTime = millis();
    _alarmMaxDuration = maxDurationMs;
    _melodyIndex = 0;

    uint16_t freq = melody[0].freq;
    if (freq > 0) {
        tone(_pin, freq);
    } else {
        noTone(_pin);
    }
    _stepEndTime = millis() + melody[0].duration;
}

void BuzzerController::stop() {
    _isAlarming = false;
    _isBeeping = false;
    _melodyIndex = 0;
    noTone(_pin);
    digitalWrite(_pin, LOW);
}

void BuzzerController::beep(unsigned long durationMs, uint16_t freq) {
    if (_isAlarming) return; // Prioridad para la alarma
    _isBeeping = true;
    _beepEndTime = millis() + durationMs;
    tone(_pin, freq);
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
            noTone(_pin);
            digitalWrite(_pin, LOW);
        }
        return;
    }

    // 2. Manejo de Alarma con notas reales (no bloqueante)
    if (_isAlarming) {
        if ((now - _alarmStartTime) >= _alarmMaxDuration) {
            stop();
            return;
        }

        if (now >= _stepEndTime) {
            const MelodyStep* melody = getActiveMelody();
            uint16_t length = getActiveMelodyLength();

            _melodyIndex++;
            if (_melodyIndex >= length) {
                _melodyIndex = 0;
            }

            uint16_t freq = melody[_melodyIndex].freq;
            if (freq > 0) {
                tone(_pin, freq);
            } else {
                noTone(_pin);
            }
            _stepEndTime = now + melody[_melodyIndex].duration;
        }
    }
}
