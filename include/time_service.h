#pragma once

#include <Arduino.h>
#include <time.h>

struct DateTimeInfo {
    int hour;
    int minute;
    int second;
    int day;
    int month;
    int year;
    int dayOfWeek; // 0 = Domingo, 1 = Lunes, ..., 6 = Sábado
    bool isDST;

    char timeStr[9];       // "HH:MM:SS"
    char shortTimeStr[6];  // "HH:MM"
    char shortDateStr[40]; // "Sábado, 24 de Octubre"
    char dateStr[64];      // "24 de Octubre 2026 - S. Antonio M. Claret"
    char dayName[12];      // "Sábado"
    char saintName[36];    // "S. Antonio M. Claret"
};

void initTimeService(const char* tzRule, const char* server1, const char* server2);
bool isTimeSynchronized();
bool getCurrentDateTime(DateTimeInfo& outInfo);

const char* getSpanishDayName(int wday);
const char* getSpanishShortDayName(int wday);
const char* getSpanishMonthName(int month);
void getSpanishDayFromIsoDate(const char* isoDate, char* outBuffer, size_t maxLen);
