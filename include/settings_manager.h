#pragma once

#include <cstdint>
#include <cstring>

// Longitudes máximas de campos
#define SETTINGS_SSID_MAX_LEN      32
#define SETTINGS_PASS_MAX_LEN      64
#define SETTINGS_LOC_NAME_MAX_LEN  32
#define SETTINGS_COORD_MAX_LEN     16

struct AppSettings {
    char wifiSsid[SETTINGS_SSID_MAX_LEN + 1];
    char wifiPassword[SETTINGS_PASS_MAX_LEN + 1];
    char locationName[SETTINGS_LOC_NAME_MAX_LEN + 1];
    char locationLat[SETTINGS_COORD_MAX_LEN + 1];
    char locationLon[SETTINGS_COORD_MAX_LEN + 1];
    uint8_t defaultPage;
    uint8_t alarmMelody;

    void setDefaults();
    bool isValid() const;
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

private:
    AppSettings _settings;
    bool _fsMounted;
    static constexpr const char* CONFIG_FILE = "/config.json";
};
#endif
