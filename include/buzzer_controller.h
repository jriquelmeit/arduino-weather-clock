#pragma once

#include <Arduino.h>

// Definiciones de frecuencias para notas musicales estándar (Hz)
// Octava 3
#define NOTE_C3   131
#define NOTE_CS3  139
#define NOTE_D3   147
#define NOTE_DS3  156
#define NOTE_E3   165
#define NOTE_F3   175
#define NOTE_FS3  185
#define NOTE_G3   196
#define NOTE_GS3  208
#define NOTE_A3   220
#define NOTE_AS3  233
#define NOTE_B3   247

// Octava 4
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

// Octava 5
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

// Octava 6
#define NOTE_C6   1047
#define NOTE_CS6  1109
#define NOTE_D6   1175
#define NOTE_DS6  1245
#define NOTE_E6   1319
#define NOTE_F6   1397
#define NOTE_FS6  1480
#define NOTE_G6   1568
#define NOTE_GS6  1661
#define NOTE_A6   1760
#define NOTE_AS6  1865
#define NOTE_B6   1976

// Octava 7
#define NOTE_C7   2093
#define NOTE_D7   2349
#define NOTE_E7   2637

enum MelodyType {
    MELODY_STAR_WARS   = 1,  // Star Wars (Marcha Imperial)
    MELODY_MARIO       = 2,  // Super Mario Bros (Overworld)
    MELODY_ZELDA       = 3,  // The Legend of Zelda (Main Theme)
    MELODY_TETRIS      = 4,  // Tetris (Korobeiniki)
    MELODY_PACMAN      = 5,  // Pac-Man (Intro Arcade)
    MELODY_NOKIA       = 6,  // Nokia Tune (Gran Vals)
    MELODY_CANTINA     = 7,  // Star Wars: Cantina Band
    MELODY_DOOM        = 8,  // Doom (E1M1 At Doom's Gate)
    MELODY_CASIO_BEEP  = 9,  // Alarma Digital Casio Beep
    MELODY_BTTF        = 10, // Volver al Futuro (Back to the Future)
    MELODY_SIMPSONS    = 11, // Los Simpson (The Simpsons Theme)
    MELODY_SONIC       = 12, // Sonic the Hedgehog (Green Hill Zone)
    MELODY_ALIEN       = 13  // Alien (Tema Principal - Jerry Goldsmith)
};
#define MELODY_MAX_TYPE 13

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
