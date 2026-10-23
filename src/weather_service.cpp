#include "weather_service.h"
#include "time_service.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

const char* getWeatherDescription(int code) {
    switch (code) {
        case 0:  return "Despejado";
        case 1:  return "Mayormente claro";
        case 2:  return "Parcialmente nublado";
        case 3:  return "Nublado";
        case 45:
        case 48: return "Niebla";
        case 51: return "Llovizna ligera";
        case 53: return "Llovizna moderada";
        case 55: return "Llovizna densa";
        case 56:
        case 57: return "Llovizna helada";
        case 61: return "Lluvia ligera";
        case 63: return "Lluvia moderada";
        case 65: return "Lluvia fuerte";
        case 66:
        case 67: return "Lluvia helada";
        case 71: return "Nieve ligera";
        case 73: return "Nieve moderada";
        case 75: return "Nieve fuerte";
        case 77: return "Granizo de nieve";
        case 80: return "Chubascos ligeros";
        case 81: return "Chubascos moderados";
        case 82: return "Chubascos fuertes";
        case 85:
        case 86: return "Chubascos de nieve";
        case 95: return "Tormenta";
        case 96:
        case 99: return "Tormenta con granizo";
        default: return "Variable";
    }
}

WeatherIconType getWeatherIconType(int code) {
    switch (code) {
        case 0:
        case 1:
            return ICON_SUNNY;
        case 2:
            return ICON_PARTLY_CLOUDY;
        case 3:
        case 45:
        case 48:
            return ICON_CLOUDY;
        case 51:
        case 53:
        case 55:
        case 56:
        case 57:
        case 61:
        case 63:
        case 65:
        case 66:
        case 67:
        case 80:
        case 81:
        case 82:
            return ICON_RAIN;
        case 71:
        case 73:
        case 75:
        case 77:
        case 85:
        case 86:
            return ICON_SNOW;
        case 95:
        case 96:
        case 99:
            return ICON_THUNDERSTORM;
        default:
            return ICON_PARTLY_CLOUDY;
    }
}

WeatherService::WeatherService() {}

bool WeatherService::fetchWeather(const char* lat, const char* lon, WeatherData& outData, uint8_t hourlyStep) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("[Clima] Error: Wi-Fi desconectado"));
        return false;
    }

    if (hourlyStep < 1) hourlyStep = 1;
    int forecastHours = (4 * hourlyStep) + 1;

    WiFiClient client;
    HTTPClient http;

    // Timeout de 6 segundos
    http.setTimeout(6000);

    String url = "http://api.open-meteo.com/v1/forecast?latitude=";
    url += lat;
    url += "&longitude=";
    url += lon;
    url += "&current=temperature_2m,relative_humidity_2m,weather_code";
    url += "&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max,precipitation_sum";
    url += "&hourly=temperature_2m,weather_code,precipitation_probability,precipitation";
    url += "&forecast_hours=";
    url += String(forecastHours);
    url += "&forecast_days=3";
    url += "&timezone=auto";

    Serial.print(F("[Clima] Solicitando API Open-Meteo: "));
    Serial.println(url);

    if (!http.begin(client, url)) {
        Serial.println(F("[Clima] Error: no se pudo iniciar conexión HTTP"));
        return false;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[Clima] Error HTTP: %d (%s)\n", httpCode, http.errorToString(httpCode).c_str());
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    Serial.printf("[Clima] Respuesta recibida (%u bytes)\n", payload.length());

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.printf("[Clima] Error al parsear JSON: %s\n", error.c_str());
        return false;
    }

    // Parsear clima actual
    outData.current.temperature = doc["current"]["temperature_2m"] | 0.0f;
    outData.current.humidity = doc["current"]["relative_humidity_2m"] | 0;
    outData.current.weatherCode = doc["current"]["weather_code"] | 0;
    snprintf(outData.current.conditionText, sizeof(outData.current.conditionText), "%s",
             getWeatherDescription(outData.current.weatherCode));
    outData.current.iconType = getWeatherIconType(outData.current.weatherCode);

    // Parsear pronóstico de hoy (índice 0 de daily)
    const char* todayStr = doc["daily"]["time"][0] | "";
    snprintf(outData.today.date, sizeof(outData.today.date), "%s", todayStr);
    getSpanishDayFromIsoDate(todayStr, outData.today.dayName, sizeof(outData.today.dayName));
    outData.today.tempMax = doc["daily"]["temperature_2m_max"][0] | 0.0f;
    outData.today.tempMin = doc["daily"]["temperature_2m_min"][0] | 0.0f;
    outData.today.weatherCode = doc["daily"]["weather_code"][0] | 0;
    outData.today.rainProbability = doc["daily"]["precipitation_probability_max"][0] | 0;
    outData.today.precipitation = doc["daily"]["precipitation_sum"][0] | 0.0f;
    snprintf(outData.today.conditionText, sizeof(outData.today.conditionText), "%s",
             getWeatherDescription(outData.today.weatherCode));
    outData.today.iconType = getWeatherIconType(outData.today.weatherCode);

    // Parsear pronóstico de los dos días posteriores (índices 1 y 2 de daily)
    // Índice 0 es hoy, índice 1 es mañana, índice 2 es pasado mañana
    for (int i = 0; i < 2; i++) {
        int dayIndex = i + 1;
        const char* dStr = doc["daily"]["time"][dayIndex] | "";
        snprintf(outData.forecast[i].date, sizeof(outData.forecast[i].date), "%s", dStr);
        getSpanishDayFromIsoDate(dStr, outData.forecast[i].dayName, sizeof(outData.forecast[i].dayName));

        outData.forecast[i].tempMax = doc["daily"]["temperature_2m_max"][dayIndex] | 0.0f;
        outData.forecast[i].tempMin = doc["daily"]["temperature_2m_min"][dayIndex] | 0.0f;
        outData.forecast[i].weatherCode = doc["daily"]["weather_code"][dayIndex] | 0;
        outData.forecast[i].rainProbability = doc["daily"]["precipitation_probability_max"][dayIndex] | 0;
        outData.forecast[i].precipitation = doc["daily"]["precipitation_sum"][dayIndex] | 0.0f;
        snprintf(outData.forecast[i].conditionText, sizeof(outData.forecast[i].conditionText), "%s",
                 getWeatherDescription(outData.forecast[i].weatherCode));
        outData.forecast[i].iconType = getWeatherIconType(outData.forecast[i].weatherCode);
    }

    // Parsear pronóstico por horas (4 bloques con paso configurable)
    for (int i = 0; i < 4; i++) {
        int hIndex = (i + 1) * hourlyStep;
        const char* tStr = doc["hourly"]["time"][hIndex] | "";
        if (strlen(tStr) >= 16) {
            // "YYYY-MM-DDTHH:MM" -> copiar "HH:MM"
            snprintf(outData.hourly[i].timeStr, sizeof(outData.hourly[i].timeStr), "%.5s", tStr + 11);
        } else {
            snprintf(outData.hourly[i].timeStr, sizeof(outData.hourly[i].timeStr), "+%dh", (i + 1) * hourlyStep);
        }

        outData.hourly[i].temperature = doc["hourly"]["temperature_2m"][hIndex] | 0.0f;
        outData.hourly[i].weatherCode = doc["hourly"]["weather_code"][hIndex] | 0;
        outData.hourly[i].rainProbability = doc["hourly"]["precipitation_probability"][hIndex] | 0;
        outData.hourly[i].precipitation = doc["hourly"]["precipitation"][hIndex] | 0.0f;
        snprintf(outData.hourly[i].conditionText, sizeof(outData.hourly[i].conditionText), "%s",
                 getWeatherDescription(outData.hourly[i].weatherCode));
        outData.hourly[i].iconType = getWeatherIconType(outData.hourly[i].weatherCode);
    }

    outData.valid = true;
    outData.lastFetchTime = millis();

    Serial.println(F("[Clima] Datos meteorologicos actualizados con exito."));
    Serial.printf("  Actual: %.1f C, Hum: %d%%, Cond: %s\n",
                  outData.current.temperature, outData.current.humidity, outData.current.conditionText);
    Serial.printf("  Manana (%s): Min: %.1f C, Max: %.1f C, Lluvia: %d%% (%.1fmm), %s\n",
                  outData.forecast[0].dayName, outData.forecast[0].tempMin, outData.forecast[0].tempMax,
                  outData.forecast[0].rainProbability, outData.forecast[0].precipitation, outData.forecast[0].conditionText);
    Serial.printf("  Pasado Manana (%s): Min: %.1f C, Max: %.1f C, Lluvia: %d%% (%.1fmm), %s\n",
                  outData.forecast[1].dayName, outData.forecast[1].tempMin, outData.forecast[1].tempMax,
                  outData.forecast[1].rainProbability, outData.forecast[1].precipitation, outData.forecast[1].conditionText);
    for (int i = 0; i < 4; i++) {
        Serial.printf("  Horario (%s): %.1f C, Lluvia: %d%% (%.1fmm), %s\n",
                      outData.hourly[i].timeStr, outData.hourly[i].temperature,
                      outData.hourly[i].rainProbability, outData.hourly[i].precipitation,
                      outData.hourly[i].conditionText);
    }

    return true;
}
