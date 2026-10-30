#include "settings_manager.h"
#include "config.h"
#include "buzzer_controller.h"

void DailyAlarm::setDefaults(uint8_t alarmId) {
    id = alarmId;
    enabled = false;
    hour = 7;
    minute = 0;
    days = ALARM_DAYS_WEEKDAYS;
    melody = 0; // 0 = Usar melodía predeterminada del sistema
    snprintf(label, sizeof(label), "Alarma %u", alarmId);
}

void formatAlarmDaysString(uint8_t days, char* buf, size_t maxLen) {
    if (!buf || maxLen == 0) return;
    uint8_t masked = days & 0x7F;
    if (masked == ALARM_DAYS_ALL) {
        snprintf(buf, maxLen, "Todos los dias");
    } else if (masked == ALARM_DAYS_WEEKDAYS) {
        snprintf(buf, maxLen, "Lun a Vie");
    } else if (masked == ALARM_DAYS_WEEKEND) {
        snprintf(buf, maxLen, "Fin de semana");
    } else if (masked == 0) {
        snprintf(buf, maxLen, "Desactivado");
    } else {
        static const char dayLetters[] = "DLMXJVS";
        char temp[32] = {0};
        size_t pos = 0;
        for (int i = 0; i < 7 && pos < sizeof(temp) - 2; i++) {
            if (masked & (1 << i)) {
                if (pos > 0) temp[pos++] = ' ';
                temp[pos++] = dayLetters[i];
            }
        }
        temp[pos] = '\0';
        snprintf(buf, maxLen, "%s", temp);
    }
}

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
    if (defaultPage < 1 || defaultPage > TOTAL_PAGES) defaultPage = 1;

    alarmMelody = ALARM_MELODY_TYPE;
    if (alarmMelody < 1 || alarmMelody > MELODY_MAX_TYPE) alarmMelody = 1;

    enabledPage1 = ENABLED_PAGE_1;
    enabledPage2 = ENABLED_PAGE_2;
    enabledPage3 = ENABLED_PAGE_3;
    enabledPage4 = ENABLED_PAGE_4;
    enabledPage5 = ENABLED_PAGE_5;

    for (uint8_t i = 0; i < MAX_DAILY_ALARMS; i++) {
        alarms[i].setDefaults(i + 1);
    }
}

bool AppSettings::isValid() const {
    if (strlen(wifiSsid) == 0) return false;
    if (strlen(locationLat) == 0 || strlen(locationLon) == 0) return false;
    if (defaultPage < 1 || defaultPage > TOTAL_PAGES) return false;
    if (alarmMelody < 1 || alarmMelody > MELODY_MAX_TYPE) return false;
    if (!enabledPage1 && !enabledPage2 && !enabledPage3 && !enabledPage4 && !enabledPage5) return false;
    return true;
}

bool AppSettings::isPageEnabled(uint8_t page) const {
    switch (page) {
        case 1: return enabledPage1;
        case 2: return enabledPage2;
        case 3: return enabledPage3;
        case 4: return enabledPage4;
        case 5: return enabledPage5;
        default: return false;
    }
}

void AppSettings::setPageEnabled(uint8_t page, bool enabled) {
    switch (page) {
        case 1: enabledPage1 = enabled; break;
        case 2: enabledPage2 = enabled; break;
        case 3: enabledPage3 = enabled; break;
        case 4: enabledPage4 = enabled; break;
        case 5: enabledPage5 = enabled; break;
        default: break;
    }
    // Salvaguarda: mantener activa al menos la página 1 si todas se desactivan
    if (!enabledPage1 && !enabledPage2 && !enabledPage3 && !enabledPage4 && !enabledPage5) {
        enabledPage1 = true;
    }
}

uint8_t AppSettings::getNextEnabledPage(uint8_t current) const {
    uint8_t p = current;
    for (int i = 0; i < TOTAL_PAGES; i++) {
        p = (p >= TOTAL_PAGES) ? 1 : (p + 1);
        if (isPageEnabled(p)) return p;
    }
    return current;
}

uint8_t AppSettings::getPrevEnabledPage(uint8_t current) const {
    uint8_t p = current;
    for (int i = 0; i < TOTAL_PAGES; i++) {
        p = (p <= 1) ? TOTAL_PAGES : (p - 1);
        if (isPageEnabled(p)) return p;
    }
    return current;
}

uint8_t AppSettings::getFirstEnabledPage() const {
    for (uint8_t p = 1; p <= TOTAL_PAGES; p++) {
        if (isPageEnabled(p)) return p;
    }
    return 1;
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

    if (page >= 1 && page <= TOTAL_PAGES) {
        _settings.defaultPage = page;
    }
    if (melody >= 1 && melody <= MELODY_MAX_TYPE) {
        _settings.alarmMelody = melody;
    }

    _settings.enabledPage1 = doc["enabled_page_1"] | ENABLED_PAGE_1;
    _settings.enabledPage2 = doc["enabled_page_2"] | ENABLED_PAGE_2;
    _settings.enabledPage3 = doc["enabled_page_3"] | ENABLED_PAGE_3;
    _settings.enabledPage4 = doc["enabled_page_4"] | ENABLED_PAGE_4;
    _settings.enabledPage5 = doc["enabled_page_5"] | ENABLED_PAGE_5;

    JsonArray alarmsArr = doc["alarms"].as<JsonArray>();
    if (alarmsArr) {
        uint8_t idx = 0;
        for (JsonObject a : alarmsArr) {
            if (idx >= MAX_DAILY_ALARMS) break;
            _settings.alarms[idx].id = a["id"] | (idx + 1);
            _settings.alarms[idx].enabled = a["enabled"] | false;
            _settings.alarms[idx].hour = a["hour"] | 7;
            _settings.alarms[idx].minute = a["minute"] | 0;
            _settings.alarms[idx].days = a["days"] | ALARM_DAYS_WEEKDAYS;
            _settings.alarms[idx].melody = a["melody"] | 0;
            const char* lbl = a["label"] | "";
            if (strlen(lbl) > 0) {
                strncpy(_settings.alarms[idx].label, lbl, ALARM_LABEL_MAX_LEN);
                _settings.alarms[idx].label[ALARM_LABEL_MAX_LEN] = '\0';
            }
            idx++;
        }
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
    doc["enabled_page_1"] = _settings.enabledPage1;
    doc["enabled_page_2"] = _settings.enabledPage2;
    doc["enabled_page_3"] = _settings.enabledPage3;
    doc["enabled_page_4"] = _settings.enabledPage4;
    doc["enabled_page_5"] = _settings.enabledPage5;

    JsonArray alarmsArr = doc["alarms"].to<JsonArray>();
    for (uint8_t i = 0; i < MAX_DAILY_ALARMS; i++) {
        JsonObject a = alarmsArr.add<JsonObject>();
        a["id"] = _settings.alarms[i].id;
        a["enabled"] = _settings.alarms[i].enabled;
        a["hour"] = _settings.alarms[i].hour;
        a["minute"] = _settings.alarms[i].minute;
        a["days"] = _settings.alarms[i].days;
        a["melody"] = _settings.alarms[i].melody;
        a["label"] = _settings.alarms[i].label;
    }

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
    if (page >= 1 && page <= TOTAL_PAGES) {
        _settings.defaultPage = page;
    }
}

void SettingsManager::setAlarmMelody(uint8_t melody) {
    if (melody >= 1 && melody <= MELODY_MAX_TYPE) {
        _settings.alarmMelody = melody;
    }
}

void SettingsManager::setPageEnabled(uint8_t page, bool enabled) {
    _settings.setPageEnabled(page, enabled);
}

bool SettingsManager::setAlarm(uint8_t index, const DailyAlarm& alarm) {
    if (index >= MAX_DAILY_ALARMS) return false;
    _settings.alarms[index] = alarm;
    _settings.alarms[index].id = index + 1;
    return true;
}

bool SettingsManager::toggleAlarm(uint8_t index) {
    if (index >= MAX_DAILY_ALARMS) return false;
    _settings.alarms[index].enabled = !_settings.alarms[index].enabled;
    return _settings.alarms[index].enabled;
}

#endif
