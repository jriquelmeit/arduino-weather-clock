#pragma once

#include <Arduino.h>

// Definiciones de frecuencias para notas musicales estándar (Hz)
#define NOTE_C4   262
#define NOTE_CS4  277
#define NOTE_D4   294
#define NOTE_DS4  311
#define NOTE_E4   330
#define NOTE_F4   349
#define NOTE_FS4  370
#define NOTE_G4   392
#define NOTE_GS4  415
#define NOTE_A4   440
#define NOTE_AS4  466
#define NOTE_B4   494

#define NOTE_C5   523
#define NOTE_CS5  554
#define NOTE_D5   587
#define NOTE_DS5  622
#define NOTE_E5   659
#define NOTE_F5   698
#define NOTE_FS5  740
#define NOTE_G5   784
#define NOTE_GS5  831
#define NOTE_A5   880
#define NOTE_AS5  932
#define NOTE_B5   988

#define NOTE_C6   1047
#define NOTE_D6   1175
#define NOTE_E6   1319

enum MelodyType {
    MELODY_STAR_WARS = 1,
    MELODY_MARIO     = 2
};

struct MelodyStep {
    uint16_t freq;      // Frecuencia en Hz (0 para silencio)
    uint16_t duration;  // Duración en milisegundos
};

class BuzzerController {
public:
    explicit BuzzerController(uint8_t pin, uint8_t melodyType = MELODY_STAR_WARS);
    void begin();

    void setMelodyType(uint8_t melodyType);
    void playStartupMelody(int stopPin1 = -1, int stopPin2 = -1, int stopPin3 = -1);
    void startAlarm(unsigned long maxDurationMs = 60000);
    void stop();
    void beep(unsigned long durationMs = 40, uint16_t freq = 2000);

    bool isAlarming() const;
    void update();

private:
    const MelodyStep* getActiveMelody() const;
    uint16_t getActiveMelodyLength() const;

    uint8_t _pin;
    uint8_t _melodyType;
    bool _isAlarming;
    bool _isBeeping;

    unsigned long _alarmStartTime;
    unsigned long _alarmMaxDuration;
    unsigned long _stepEndTime;
    unsigned long _beepEndTime;
    uint16_t _melodyIndex;
};
