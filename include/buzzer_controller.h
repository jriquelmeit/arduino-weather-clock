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
    unsigned long _lastToggleTime;
    unsigned long _beepEndTime;

    // Patrón de alarma: alternar ON/OFF
    static const unsigned long ALARM_BEEP_ON_MS = 150;
    static const unsigned long ALARM_BEEP_OFF_MS = 150;
};
