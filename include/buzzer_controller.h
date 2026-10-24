#pragma once

#include <Arduino.h>

class BuzzerController {
public:
    explicit BuzzerController(uint8_t pin);
    void begin();

    void startAlarm(unsigned long maxDurationMs = 30000);
    void stop();
    void beep(unsigned long durationMs = 60);

    bool isAlarming() const;
    void update();

private:
    uint8_t _pin;
    bool _isAlarming;
    bool _isBeeping;
    bool _pinState;

    unsigned long _alarmStartTime;
    unsigned long _alarmMaxDuration;
    unsigned long _stepEndTime;
    unsigned long _beepEndTime;
    uint8_t _rhythmIndex;
};
