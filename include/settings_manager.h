#pragma once

#include <cstdint>
#include <cstring>
#include <cstdio>

// Longitudes máximas de campos
#define SETTINGS_SSID_MAX_LEN      32
#define SETTINGS_PASS_MAX_LEN      64
#define SETTINGS_LOC_NAME_MAX_LEN  32
#define SETTINGS_COORD_MAX_LEN     16

#define MAX_DAILY_ALARMS           10
#define ALARM_LABEL_MAX_LEN        16

// Máscaras de días para alarmas (Bit 0: Dom, Bit 1: Lun, ..., Bit 6: Sab)
#define ALARM_DAYS_ALL             0x7F // 127: D L M X J V S
#define ALARM_DAYS_WEEKDAYS        0x3E // 62: Lun a Vie (bits 1..5)
#define ALARM_DAYS_WEEKEND         0x41 // 65: Sab y Dom (bits 6, 0)

inline bool isAlarmActiveOnDay(uint8_t daysMask, int dayOfWeek) {
    if (dayOfWeek < 0 || dayOfWeek > 6) return false;
    return (daysMask & (1 << dayOfWeek)) != 0;
}

void formatAlarmDaysString(uint8_t days, char* buf, size_t maxLen);

struct DailyAlarm {
    uint8_t id;         // 1..10
    bool enabled;
    uint8_t hour;       // 0..23
    uint8_t minute;     // 0..59
    uint8_t days;       // bitmask 0..127
    char label[ALARM_LABEL_MAX_LEN + 1];

    void setDefaults(uint8_t alarmId);
};

struct AppSettings {
    char wifiSsid[SETTINGS_SSID_MAX_LEN + 1];
    char wifiPassword[SETTINGS_PASS_MAX_LEN + 1];
    char locationName[SETTINGS_LOC_NAME_MAX_LEN + 1];
    char locationLat[SETTINGS_COORD_MAX_LEN + 1];
    char locationLon[SETTINGS_COORD_MAX_LEN + 1];
    uint8_t defaultPage;
    uint8_t alarmMelody;

    // Páginas habilitadas
    bool enabledPage1;
    bool enabledPage2;
    bool enabledPage3;
    bool enabledPage4;
    bool enabledPage5;

    // Despertador
    DailyAlarm alarms[MAX_DAILY_ALARMS];

    void setDefaults();
    bool isValid() const;

    bool isPageEnabled(uint8_t page) const;
    void setPageEnabled(uint8_t page, bool enabled);
    uint8_t getNextEnabledPage(uint8_t current) const;
    uint8_t getPrevEnabledPage(uint8_t current) const;
    uint8_t getFirstEnabledPage() const;
};

#if defined(ESP8266)
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

class SettingsManager {
public:
    SettingsManager();

    bool begin();
    bool load();
    bool save();
    void resetToDefaults();

    AppSettings& get() { return _settings; }
    const AppSettings& get() const { return _settings; }

    void setWifi(const char* ssid, const char* pass);
    void setLocation(const char* name, const char* lat, const char* lon);
    void setDefaultPage(uint8_t page);
    void setAlarmMelody(uint8_t melody);
    void setPageEnabled(uint8_t page, bool enabled);
    bool setAlarm(uint8_t index, const DailyAlarm& alarm);
    bool toggleAlarm(uint8_t index);

private:
    AppSettings _settings;
    bool _fsMounted;
    static constexpr const char* CONFIG_FILE = "/config.json";
};
#endif
