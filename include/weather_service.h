#pragma once

#include <Arduino.h>

enum WeatherIconType {
    ICON_SUNNY = 0,
    ICON_CLEAR_NIGHT,
    ICON_PARTLY_CLOUDY,
    ICON_PARTLY_CLOUDY_NIGHT,
    ICON_CLOUDY,
    ICON_RAIN,
    ICON_SNOW,
    ICON_THUNDERSTORM,
    ICON_UNKNOWN
};

struct CurrentWeather {
    float temperature;
    int humidity;
    float uvIndex;
    int weatherCode;
    bool isDay;
    char conditionText[30];
    WeatherIconType iconType;
};

struct DailyForecast {
    char date[11];        // "YYYY-MM-DD"
    char dayName[15];     // "Domingo", "Lunes", etc.
    float tempMax;
    float tempMin;
    int weatherCode;
    int rainProbability;  // Probabilidad máxima (%)
    float precipitation;  // Precipitación acumulada (mm)
    float uvIndexMax;     // UV Máximo del día
    char conditionText[30];
    WeatherIconType iconType;
};

struct HourlyForecast {
    char timeStr[8];      // "HH:MM" o "+12h"
    float temperature;
    int weatherCode;
    bool isDay;
    int rainProbability;  // Probabilidad por hora (%)
    float precipitation;  // Precipitación por hora (mm)
    char conditionText[30];
    WeatherIconType iconType;
};

struct WeatherData {
    bool valid;
    unsigned long lastFetchTime;
    CurrentWeather current;
    DailyForecast today;       // Pronóstico del día de hoy
    DailyForecast forecast[2]; // Dos días posteriores (mañana y pasado mañana)
    HourlyForecast hourly[4];  // Cuatro bloques horarios futuros
};

const char* getWeatherDescription(int code);
WeatherIconType getWeatherIconType(int code, bool isDay = true);
const char* getUvCategory(float uv);
uint16_t getUvColor(float uv);

class WeatherService {
public:
    WeatherService();
    bool fetchWeather(const char* lat, const char* lon, WeatherData& outData, uint8_t hourlyStep = 1);
};
