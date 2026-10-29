#pragma once

#if defined(ESP8266)
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <functional>
#include "settings_manager.h"

enum WebServerMode {
    WEB_MODE_NONE = 0,
    WEB_MODE_PORTAL_AP,
    WEB_MODE_STATION
};

struct WebServerCallbacks {
    // Control de pantalla
    std::function<void(uint8_t)> onPageChange;
    std::function<void()> onToggleSleep;
    std::function<bool()> isSleeping;
    std::function<uint8_t()> getCurrentPage;

    // Control de temporizador
    std::function<void(uint32_t)> onTimerStart;
    std::function<void()> onTimerStop;
    std::function<void(uint32_t)> onTimerSetSeconds;
    std::function<void(uint32_t&, uint32_t&, uint8_t&)> getTimerStatus;

    // Control de buzzer / alarma
    std::function<void(uint8_t)> onSetMelody;
    std::function<void()> onTestAlarm;
    std::function<void()> onStopAlarm;
    std::function<bool()> isAlarming;
    std::function<uint8_t()> getCurrentMelody;

    // Clima y reloj
    std::function<void(char* timeBuf, size_t timeLen, float& temp, int& hum, char* condBuf, size_t condLen)> getWeatherInfo;

    // Guardado de configuración
    std::function<bool(const char* ssid, const char* pass, const char* loc, const char* lat, const char* lon,
                       uint8_t melody, uint8_t page,
                       bool p1, bool p2, bool p3, bool p4, bool p5)> onSaveConfig;

    // Despertador y páginas
    std::function<void(DailyAlarm* outAlarms, uint8_t& count)> onGetAlarms;
    std::function<bool(uint8_t id, bool enabled, uint8_t hour, uint8_t minute, uint8_t days, const char* label)> onSetAlarm;
    std::function<bool(uint8_t id)> onToggleAlarm;
    std::function<void(uint8_t page, bool enabled)> onSetPageEnabled;
    std::function<bool(uint8_t page)> isPageEnabled;
};

class WebServerService {
public:
    WebServerService(SettingsManager& settingsMgr, const WebServerCallbacks& callbacks);

    void beginPortalAP(const char* apSsid = "WeatherClock-Setup");
    void beginStationServer();
    void stop();

    void update();
    WebServerMode getMode() const { return _mode; }

private:
    SettingsManager& _settings;
    WebServerCallbacks _cb;
    ESP8266WebServer _server;
    DNSServer _dnsServer;
    WebServerMode _mode;
    unsigned long _rebootRequestedTime;
    bool _rebootRequested;

    void setupCommonRoutes();
    void setupPortalRoutes();
    void setupStationRoutes();

    void handleRoot();
    void handleSetupPage();
    void handleScanWifi();
    void handleSaveConfig();

    // API Handlers
    void handleApiStatus();
    void handleApiPage();
    void handleApiSleep();
    void handleApiTimer();
    void handleApiAlarm();
    void handleApiAlarms();
    void handleApiPages();

    // Portal cautivo
    bool isCaptivePortalRequest();
    void handleCaptivePortalRedirect();
};

#endif
