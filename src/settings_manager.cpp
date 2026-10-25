#include "settings_manager.h"
#include "config.h"

void AppSettings::setDefaults() {
    strncpy(wifiSsid, WIFI_SSID, sizeof(wifiSsid) - 1);
    wifiSsid[sizeof(wifiSsid) - 1] = '\0';

    strncpy(wifiPassword, WIFI_PASSWORD, sizeof(wifiPassword) - 1);
    wifiPassword[sizeof(wifiPassword) - 1] = '\0';

    strncpy(locationName, LOCATION_NAME, sizeof(locationName) - 1);
    locationName[sizeof(locationName) - 1] = '\0';

    strncpy(locationLat, LOCATION_LAT, sizeof(locationLat) - 1);
    locationLat[sizeof(locationLat) - 1] = '\0';

    strncpy(locationLon, LOCATION_LON, sizeof(locationLon) - 1);
    locationLon[sizeof(locationLon) - 1] = '\0';

    defaultPage = DISPLAY_PAGE_MODE;
    if (defaultPage < 1 || defaultPage > 4) defaultPage = 1;

    alarmMelody = ALARM_MELODY_TYPE;
    if (alarmMelody < 1 || alarmMelody > 2) alarmMelody = 1;
}

bool AppSettings::isValid() const {
    if (strlen(wifiSsid) == 0) return false;
    if (strlen(locationLat) == 0 || strlen(locationLon) == 0) return false;
    if (defaultPage < 1 || defaultPage > 4) return false;
    if (alarmMelody < 1 || alarmMelody > 2) return false;
    return true;
}

#if defined(ESP8266)

SettingsManager::SettingsManager() : _fsMounted(false) {
    _settings.setDefaults();
}

bool SettingsManager::begin() {
    if (!LittleFS.begin()) {
        Serial.println(F("[Settings] LittleFS mount fallo, formateando..."));
        if (!LittleFS.format() || !LittleFS.begin()) {
            Serial.println(F("[Settings] Error critico montando LittleFS"));
            _fsMounted = false;
            return false;
        }
    }
    _fsMounted = true;
    Serial.println(F("[Settings] LittleFS montado correctamente."));

    if (!load()) {
        Serial.println(F("[Settings] No hay config.json valido, guardando valores iniciales..."));
        save();
    }
    return true;
}

bool SettingsManager::load() {
    if (!_fsMounted) return false;
    if (!LittleFS.exists(CONFIG_FILE)) {
        Serial.println(F("[Settings] Archivo config.json no existe."));
        return false;
    }

    File file = LittleFS.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println(F("[Settings] Error abriendo config.json para lectura"));
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.printf("[Settings] Error parseando JSON: %s\n", error.c_str());
        return false;
    }

    const char* ssid = doc["ssid"] | "";
    const char* pass = doc["password"] | "";
    const char* loc = doc["location"] | "";
    const char* lat = doc["lat"] | "";
    const char* lon = doc["lon"] | "";
    uint8_t page = doc["page"] | DISPLAY_PAGE_MODE;
    uint8_t melody = doc["melody"] | ALARM_MELODY_TYPE;

    if (strlen(ssid) > 0) {
        strncpy(_settings.wifiSsid, ssid, sizeof(_settings.wifiSsid) - 1);
        _settings.wifiSsid[sizeof(_settings.wifiSsid) - 1] = '\0';
    }
    strncpy(_settings.wifiPassword, pass, sizeof(_settings.wifiPassword) - 1);
    _settings.wifiPassword[sizeof(_settings.wifiPassword) - 1] = '\0';

    if (strlen(loc) > 0) {
        strncpy(_settings.locationName, loc, sizeof(_settings.locationName) - 1);
        _settings.locationName[sizeof(_settings.locationName) - 1] = '\0';
    }
    if (strlen(lat) > 0) {
        strncpy(_settings.locationLat, lat, sizeof(_settings.locationLat) - 1);
        _settings.locationLat[sizeof(_settings.locationLat) - 1] = '\0';
    }
    if (strlen(lon) > 0) {
        strncpy(_settings.locationLon, lon, sizeof(_settings.locationLon) - 1);
        _settings.locationLon[sizeof(_settings.locationLon) - 1] = '\0';
    }

    if (page >= 1 && page <= 4) {
        _settings.defaultPage = page;
    }
    if (melody >= 1 && melody <= 2) {
        _settings.alarmMelody = melody;
    }

    Serial.printf("[Settings] Configuracion cargada: SSID=%s, Ciudad=%s, Lat=%s, Lon=%s, Pagina=%d, Melodia=%d\n",
                  _settings.wifiSsid, _settings.locationName, _settings.locationLat, _settings.locationLon,
                  _settings.defaultPage, _settings.alarmMelody);
    return true;
}

bool SettingsManager::save() {
    if (!_fsMounted) return false;

    File file = LittleFS.open(CONFIG_FILE, "w");
    if (!file) {
        Serial.println(F("[Settings] Error abriendo config.json para escritura"));
        return false;
    }

    JsonDocument doc;
    doc["ssid"] = _settings.wifiSsid;
    doc["password"] = _settings.wifiPassword;
    doc["location"] = _settings.locationName;
    doc["lat"] = _settings.locationLat;
    doc["lon"] = _settings.locationLon;
    doc["page"] = _settings.defaultPage;
    doc["melody"] = _settings.alarmMelody;

    if (serializeJson(doc, file) == 0) {
        Serial.println(F("[Settings] Error escribiendo JSON a LittleFS"));
        file.close();
        return false;
    }

    file.close();
    Serial.println(F("[Settings] Configuracion guardada exitosamente en LittleFS."));
    return true;
}

void SettingsManager::resetToDefaults() {
    _settings.setDefaults();
    save();
}

void SettingsManager::setWifi(const char* ssid, const char* pass) {
    if (ssid) {
        strncpy(_settings.wifiSsid, ssid, sizeof(_settings.wifiSsid) - 1);
        _settings.wifiSsid[sizeof(_settings.wifiSsid) - 1] = '\0';
    }
    if (pass) {
        strncpy(_settings.wifiPassword, pass, sizeof(_settings.wifiPassword) - 1);
        _settings.wifiPassword[sizeof(_settings.wifiPassword) - 1] = '\0';
    }
}

void SettingsManager::setLocation(const char* name, const char* lat, const char* lon) {
    if (name) {
        strncpy(_settings.locationName, name, sizeof(_settings.locationName) - 1);
        _settings.locationName[sizeof(_settings.locationName) - 1] = '\0';
    }
    if (lat) {
        strncpy(_settings.locationLat, lat, sizeof(_settings.locationLat) - 1);
        _settings.locationLat[sizeof(_settings.locationLat) - 1] = '\0';
    }
    if (lon) {
        strncpy(_settings.locationLon, lon, sizeof(_settings.locationLon) - 1);
        _settings.locationLon[sizeof(_settings.locationLon) - 1] = '\0';
    }
}

void SettingsManager::setDefaultPage(uint8_t page) {
    if (page >= 1 && page <= 4) {
        _settings.defaultPage = page;
    }
}

void SettingsManager::setAlarmMelody(uint8_t melody) {
    if (melody >= 1 && melody <= 2) {
        _settings.alarmMelody = melody;
    }
}

#endif
