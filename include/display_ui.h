#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "time_service.h"
#include "weather_service.h"

enum TimerDisplayState {
    TIMER_UI_IDLE = 0,
    TIMER_UI_RUNNING,
    TIMER_UI_ALARM
};

class DisplayUI {
public:
    DisplayUI(uint8_t cs, uint8_t dc, uint8_t rst);

    void begin(uint8_t rotation = 3);
    void showBootScreen(const char* ssid);
    void updateBootProgress(const char* stepMsg, uint8_t dots);

    // Dibuja la estructura base fija (marcos, cabecera, etiquetas estáticas)
    // fullRedraw = true redibuja toda la pantalla; false solo redibuja la zona central de contenido
    void drawBaseLayout(const char* locationName, uint8_t pageMode = 1, bool fullRedraw = true);

    // Métodos de actualización dinámica (flicker-free)
    void updateTime(const DateTimeInfo& timeInfo);
    void updateDate(const DateTimeInfo& timeInfo);
    void updateWeather(const WeatherData& weather, uint8_t pageMode = 1);
    void updateTimer(uint16_t remainingSec, uint16_t totalSec, TimerDisplayState state, bool forceRedraw = false);
    void updateWifiStatus(bool connected, int rssi, const IPAddress& ip);
    void updateStatusBar(const char* message);

    // Control de energía / apagado de pantalla
    void sleepDisplay();
    void wakeDisplay();
    bool isSleeping() const;

private:
    Adafruit_ILI9341 _tft;
    char _lastTimeStr[10];
    char _lastDateStr[40];
    char _lastDayName[16];
    char _lastTimerStr[10];
    TimerDisplayState _lastTimerState;
    int _lastProgressBarWidth;
    int _lastRssi;
    bool _lastWifiState;
    bool _isSleeping;

    void drawDailyForecast(const WeatherData& weather);
    void drawHourlyForecast(const WeatherData& weather);
    void drawWeatherIcon(int x, int y, WeatherIconType type, bool small);
    void drawWaterDrop(int x, int y, uint16_t color);
    void drawWifiIndicator(int x, int y, bool connected, int rssi);
};
