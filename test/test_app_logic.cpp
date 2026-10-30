#include <unity.h>
#include <cstdint>
#include <cstring>
#include <cstdio>

// ============================================================================
// 1. MOCK & LÓGICA DE GESTIÓN DE MODO NOCHE Y REPOSO
// ============================================================================
struct SleepState {
    bool isSleeping;
    bool manualSleepActive;
    bool nightAutoSleepActive;
    bool manualWakeActive;
    unsigned long manualWakeTime;
    unsigned long lastSleepToggleTime;
};

bool isNightHour(uint8_t hour, uint8_t startHour = 23, uint8_t endHour = 7) {
    if (startHour > endHour) {
        return (hour >= startHour || hour < endHour);
    } else {
        return (hour >= startHour && hour < endHour);
    }
}

void processNightModeCycle(SleepState& state, uint8_t hour, unsigned long now, bool timerActive = false) {
    if (timerActive) return;

    bool isNight = isNightHour(hour);

    if (isNight) {
        if (!state.isSleeping) {
            if (state.manualWakeActive) {
                if (now - state.manualWakeTime >= 30000UL) {
                    state.manualWakeActive = false;
                    state.nightAutoSleepActive = true;
                    state.isSleeping = true;
                }
            } else {
                state.nightAutoSleepActive = true;
                state.isSleeping = true;
            }
        }
    } else {
        // Es de día
        state.manualWakeActive = false;
        // Solo despertar automáticamente si fue apagada por el modo noche programado
        if (state.nightAutoSleepActive && state.isSleeping) {
            state.nightAutoSleepActive = false;
            state.manualSleepActive = false;
            state.isSleeping = false;
        }
    }
}

bool handleButtonWakeEvent(SleepState& state, unsigned long now, uint8_t currentHour) {
    if (!state.isSleeping) return false;

    // Cooldown de 350 ms para evitar rebotes físicos
    if (now - state.lastSleepToggleTime < 350UL) {
        return false; // Ignorado por cooldown
    }

    state.manualSleepActive = false;
    state.nightAutoSleepActive = false;
    state.isSleeping = false;
    state.lastSleepToggleTime = now;

    if (isNightHour(currentHour)) {
        state.manualWakeActive = true;
        state.manualWakeTime = now;
    }
    return true;
}

void handleManualSleepButton(SleepState& state, unsigned long now) {
    state.manualSleepActive = true;
    state.nightAutoSleepActive = false;
    state.manualWakeActive = false;
    state.isSleeping = true;
    state.lastSleepToggleTime = now;
}

// ============================================================================
// 2. LÓGICA DE TRADUCCIÓN DE CLIMA Y DÍAS
// ============================================================================
enum WeatherIcon {
    ICON_SUNNY,
    ICON_CLEAR_NIGHT,
    ICON_PARTLY_CLOUDY,
    ICON_PARTLY_CLOUDY_NIGHT,
    ICON_CLOUDY,
    ICON_RAIN
};

WeatherIcon testWeatherIcon(int code, bool isDay) {
    switch (code) {
        case 0:
        case 1:
            return isDay ? ICON_SUNNY : ICON_CLEAR_NIGHT;
        case 2:
            return isDay ? ICON_PARTLY_CLOUDY : ICON_PARTLY_CLOUDY_NIGHT;
        case 3:
            return ICON_CLOUDY;
        case 61:
        case 63:
            return ICON_RAIN;
        default:
            return isDay ? ICON_PARTLY_CLOUDY : ICON_PARTLY_CLOUDY_NIGHT;
    }
}

const char* testUvCategory(float uv) {
    if (uv < 2.5f) return "Bajo";
    if (uv < 5.5f) return "Mod.";
    if (uv < 7.5f) return "Alto";
    if (uv < 10.5f) return "M.Alto";
    return "Extrem.";
}

// ============================================================================
// SUITE DE TESTS
// ============================================================================
void setUp(void) {}
void tearDown(void) {}

// Test 1: Verificar que el apagado manual durante el día NO parpadea ni se vuelve a encender
void test_manual_sleep_during_day_stays_off(void) {
    SleepState state = { false, false, false, false, 0, 0 };

    // Usuario pulsa botón a las 14:00 (día)
    unsigned long now = 10000;
    handleManualSleepButton(state, now);

    TEST_ASSERT_TRUE(state.isSleeping);
    TEST_ASSERT_TRUE(state.manualSleepActive);
    TEST_ASSERT_FALSE(state.nightAutoSleepActive);

    // Pasan 10 ciclos de reloj diurno (14:00)
    for (int i = 1; i <= 10; i++) {
        now += 1000;
        processNightModeCycle(state, 14, now);
        // Debe PERMANECER apagada (aquí fallaba antes)
        TEST_ASSERT_TRUE(state.isSleeping);
        TEST_ASSERT_TRUE(state.manualSleepActive);
    }
}

// Test 2: Cooldown de 350ms previene despertar por rebote del botón
void test_sleep_toggle_cooldown_prevents_bounce(void) {
    SleepState state = { false, false, false, false, 0, 0 };

    unsigned long now = 10000;
    handleManualSleepButton(state, now);
    TEST_ASSERT_TRUE(state.isSleeping);

    // Micro-rebote 50ms después
    bool wokeUp = handleButtonWakeEvent(state, now + 50, 14);
    TEST_ASSERT_FALSE(wokeUp);
    TEST_ASSERT_TRUE(state.isSleeping);

    // Pulsación legítima 400ms después
    wokeUp = handleButtonWakeEvent(state, now + 400, 14);
    TEST_ASSERT_TRUE(wokeUp);
    TEST_ASSERT_FALSE(state.isSleeping);
    TEST_ASSERT_FALSE(state.manualSleepActive);
}

// Test 3: Modo nocturno automático a las 23:00 y despertar a las 07:00
void test_scheduled_night_mode_sleep_and_wake(void) {
    SleepState state = { false, false, false, false, 0, 0 };
    unsigned long now = 50000;

    // A las 22:00 sigue encendida
    processNightModeCycle(state, 22, now);
    TEST_ASSERT_FALSE(state.isSleeping);

    // A las 23:00 debe apagarse automáticamente
    processNightModeCycle(state, 23, now);
    TEST_ASSERT_TRUE(state.isSleeping);
    TEST_ASSERT_TRUE(state.nightAutoSleepActive);

    // A las 03:00 sigue apagada
    processNightModeCycle(state, 3, now + 100000);
    TEST_ASSERT_TRUE(state.isSleeping);

    // A las 07:00 (amanecer) debe despertar automáticamente
    processNightModeCycle(state, 7, now + 200000);
    TEST_ASSERT_FALSE(state.isSleeping);
    TEST_ASSERT_FALSE(state.nightAutoSleepActive);
}

// Test 4: Despertar temporal de noche (30s timeout)
void test_manual_wake_during_night_timeout(void) {
    SleepState state = { false, false, false, false, 0, 0 };
    unsigned long now = 10000;

    // Entrar en noche (02:00)
    processNightModeCycle(state, 2, now);
    TEST_ASSERT_TRUE(state.isSleeping);

    // Usuario pulsa botón de noche para ver la hora
    now += 1000;
    bool woke = handleButtonWakeEvent(state, now, 2);
    TEST_ASSERT_TRUE(woke);
    TEST_ASSERT_FALSE(state.isSleeping);
    TEST_ASSERT_TRUE(state.manualWakeActive);

    // 15 segundos después aún está despierta
    now += 15000;
    processNightModeCycle(state, 2, now);
    TEST_ASSERT_FALSE(state.isSleeping);

    // A los 31 segundos se vuelve a apagar sola
    now += 16000; // Total 31s
    processNightModeCycle(state, 2, now);
    TEST_ASSERT_TRUE(state.isSleeping);
    TEST_ASSERT_FALSE(state.manualWakeActive);
    TEST_ASSERT_TRUE(state.nightAutoSleepActive);
}

// Test 5: Mapeo de iconos de clima de día y noche
void test_weather_icon_day_night(void) {
    // Código 0 (despejado)
    TEST_ASSERT_EQUAL_INT(ICON_SUNNY, testWeatherIcon(0, true));
    TEST_ASSERT_EQUAL_INT(ICON_CLEAR_NIGHT, testWeatherIcon(0, false));

    // Código 2 (parcialmente nublado)
    TEST_ASSERT_EQUAL_INT(ICON_PARTLY_CLOUDY, testWeatherIcon(2, true));
    TEST_ASSERT_EQUAL_INT(ICON_PARTLY_CLOUDY_NIGHT, testWeatherIcon(2, false));

    // Código 3 (nublado: igual de día y noche)
    TEST_ASSERT_EQUAL_INT(ICON_CLOUDY, testWeatherIcon(3, true));
    TEST_ASSERT_EQUAL_INT(ICON_CLOUDY, testWeatherIcon(3, false));

    // Código 61 (lluvia)
    TEST_ASSERT_EQUAL_INT(ICON_RAIN, testWeatherIcon(61, true));
}

// Test 6: Categorías de índice UV
void test_uv_categories(void) {
    TEST_ASSERT_EQUAL_STRING("Bajo", testUvCategory(1.5f));
    TEST_ASSERT_EQUAL_STRING("Mod.", testUvCategory(3.8f));
    TEST_ASSERT_EQUAL_STRING("Alto", testUvCategory(6.2f));
    TEST_ASSERT_EQUAL_STRING("M.Alto", testUvCategory(8.9f));
    TEST_ASSERT_EQUAL_STRING("Extrem.", testUvCategory(11.5f));
}

// ============================================================================
// 3. TESTS DE PORTAL DE CONFIGURACIÓN Y WEB SERVER
// ============================================================================
bool shouldEnterConfigPortal(bool btnActionLowAtBoot, unsigned long holdDurationMs, bool hasValidWifi) {
    if (!hasValidWifi) return true;
    if (btnActionLowAtBoot && holdDurationMs >= 800) return true;
    return false;
}

bool validateConfigData(const char* ssid, const char* lat, const char* lon, uint8_t page, uint8_t melody) {
    if (!ssid || strlen(ssid) == 0) return false;
    if (!lat || strlen(lat) == 0) return false;
    if (!lon || strlen(lon) == 0) return false;
    if (page < 1 || page > 5) return false;
    if (melody < 1 || melody > 13) return false;
    return true;
}

bool validateWebApiTimer(const char* action, int hours, int minutes, uint32_t& outSec) {
    if (strcmp(action, "start") == 0 || strcmp(action, "set") == 0) {
        uint32_t total = (hours * 3600) + (minutes * 60);
        if (total < 5) total = 5;
        if (total > 86400) total = 86400; // Máx 24h
        outSec = total;
        return true;
    }
    if (strcmp(action, "stop") == 0) {
        return true;
    }
    return false;
}

void formatTimerString(uint32_t totalSec, uint32_t remSec, char* buf, size_t bufLen) {
    uint32_t h = remSec / 3600;
    uint32_t m = (remSec % 3600) / 60;
    uint32_t s = remSec % 60;
    if (remSec >= 3600 || totalSec >= 3600) {
        snprintf(buf, bufLen, "%02u:%02u:%02u", h, m, s);
    } else {
        snprintf(buf, bufLen, "%02u:%02u", m, s);
    }
}

bool validateWebApiPage(int page, uint8_t& outPage) {
    if (page >= 1 && page <= 5) {
        outPage = static_cast<uint8_t>(page);
        return true;
    }
    return false;
}

void test_boot_portal_button_detection(void) {
    // Si no hay Wi-Fi configurado, siempre entra al portal
    TEST_ASSERT_TRUE(shouldEnterConfigPortal(false, 0, false));

    // Si hay Wi-Fi pero botón no fue presionado, arranque normal
    TEST_ASSERT_FALSE(shouldEnterConfigPortal(false, 0, true));

    // Si botón fue presionado pero solo 300ms (ruido/toque accidental), arranque normal
    TEST_ASSERT_FALSE(shouldEnterConfigPortal(true, 300, true));

    // Si botón fue sostenido >= 800ms al arrancar, entra al portal
    TEST_ASSERT_TRUE(shouldEnterConfigPortal(true, 850, true));
    TEST_ASSERT_TRUE(shouldEnterConfigPortal(true, 1200, true));
}

void test_settings_validation(void) {
    // Caso válido
    TEST_ASSERT_TRUE(validateConfigData("Mi_Wifi", "-33.44", "-70.65", 1, 1));
    TEST_ASSERT_TRUE(validateConfigData("Mi_Wifi", "-33.44", "-70.65", 4, 2));

    // SSID vacío -> inválido
    TEST_ASSERT_FALSE(validateConfigData("", "-33.44", "-70.65", 1, 1));

    // Coordenadas vacías -> inválido
    TEST_ASSERT_FALSE(validateConfigData("Mi_Wifi", "", "-70.65", 1, 1));
    TEST_ASSERT_FALSE(validateConfigData("Mi_Wifi", "-33.44", "", 1, 1));

    // Página fuera de rango (0 o 6) -> inválido
    TEST_ASSERT_FALSE(validateConfigData("Mi_Wifi", "-33.44", "-70.65", 0, 1));
    TEST_ASSERT_FALSE(validateConfigData("Mi_Wifi", "-33.44", "-70.65", 6, 1));
    TEST_ASSERT_TRUE(validateConfigData("Mi_Wifi", "-33.44", "-70.65", 5, 1));

    // Melodía fuera de rango (0 o 14) -> inválido
    TEST_ASSERT_FALSE(validateConfigData("Mi_Wifi", "-33.44", "-70.65", 1, 0));
    TEST_ASSERT_FALSE(validateConfigData("Mi_Wifi", "-33.44", "-70.65", 1, 14));

    // Melodías válidas (1 a 13)
    TEST_ASSERT_TRUE(validateConfigData("Mi_Wifi", "-33.44", "-70.65", 1, 1));
    TEST_ASSERT_TRUE(validateConfigData("Mi_Wifi", "-33.44", "-70.65", 1, 7));
    TEST_ASSERT_TRUE(validateConfigData("Mi_Wifi", "-33.44", "-70.65", 1, 13));
}

void test_web_api_timer_validation(void) {
    uint32_t sec = 0;

    // Acción válida con 15 minutos (0h 15m = 900s)
    TEST_ASSERT_TRUE(validateWebApiTimer("start", 0, 15, sec));
    TEST_ASSERT_EQUAL_UINT32(900, sec);

    // Acción válida con 1 hora y 30 minutos (1h 30m = 5400s)
    TEST_ASSERT_TRUE(validateWebApiTimer("start", 1, 30, sec));
    TEST_ASSERT_EQUAL_UINT32(5400, sec);

    // Clamp de tiempo mínimo (< 5s -> 5s)
    TEST_ASSERT_TRUE(validateWebApiTimer("start", 0, 0, sec));
    TEST_ASSERT_EQUAL_UINT32(5, sec);

    // Clamp de tiempo máximo (> 24h -> 86400s)
    TEST_ASSERT_TRUE(validateWebApiTimer("set", 30, 0, sec));
    TEST_ASSERT_EQUAL_UINT32(86400, sec);

    // Acción stop no requiere parámetros
    TEST_ASSERT_TRUE(validateWebApiTimer("stop", 0, 0, sec));

    // Acción desconocida
    TEST_ASSERT_FALSE(validateWebApiTimer("invalid_cmd", 0, 10, sec));

    // Prueba de formateo en pantalla
    char buf[16];
    formatTimerString(900, 900, buf, sizeof(buf)); // 15 min
    TEST_ASSERT_EQUAL_STRING("15:00", buf);

    formatTimerString(5400, 5400, buf, sizeof(buf)); // 1h 30m
    TEST_ASSERT_EQUAL_STRING("01:30:00", buf);

    formatTimerString(5400, 125, buf, sizeof(buf)); // 1h 30m total, 2m 5s restante
    TEST_ASSERT_EQUAL_STRING("00:02:05", buf);
}

void test_web_api_page_validation(void) {
    uint8_t page = 0;
    TEST_ASSERT_TRUE(validateWebApiPage(1, page));
    TEST_ASSERT_EQUAL_UINT8(1, page);
    TEST_ASSERT_TRUE(validateWebApiPage(3, page));
    TEST_ASSERT_EQUAL_UINT8(3, page);
    TEST_ASSERT_TRUE(validateWebApiPage(4, page));
    TEST_ASSERT_EQUAL_UINT8(4, page);
    TEST_ASSERT_TRUE(validateWebApiPage(5, page));
    TEST_ASSERT_EQUAL_UINT8(5, page);

    TEST_ASSERT_FALSE(validateWebApiPage(0, page));
    TEST_ASSERT_FALSE(validateWebApiPage(6, page));
}

// ============================================================================
// 4. TESTS DE ALARMAS DIARIAS Y NAVEGACIÓN DINÁMICA POR PÁGINAS ACTIVAS
// ============================================================================
#define TEST_ALARM_DAYS_ALL        0x7F // 127: D L M X J V S
#define TEST_ALARM_DAYS_WEEKDAYS   0x3E // 62: Lun a Vie (bits 1..5)
#define TEST_ALARM_DAYS_WEEKEND    0x41 // 65: Sab (bit 6) y Dom (bit 0)

bool testIsAlarmActiveOnDay(uint8_t daysMask, int dayOfWeek) {
    if (dayOfWeek < 0 || dayOfWeek > 6) return false;
    return (daysMask & (1 << dayOfWeek)) != 0;
}

void testFormatAlarmDays(uint8_t days, char* buf, size_t maxLen) {
    if (!buf || maxLen == 0) return;
    uint8_t masked = days & 0x7F;
    if (masked == TEST_ALARM_DAYS_ALL) {
        snprintf(buf, maxLen, "Todos los dias");
    } else if (masked == TEST_ALARM_DAYS_WEEKDAYS) {
        snprintf(buf, maxLen, "Lun a Vie");
    } else if (masked == TEST_ALARM_DAYS_WEEKEND) {
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

struct TestAlarm {
    uint8_t id;
    bool enabled;
    uint8_t hour;
    uint8_t minute;
    uint8_t days;
};

bool testShouldTriggerAlarm(const TestAlarm& alarm, uint8_t curHour, uint8_t curMin, int curDayOfWeek) {
    if (!alarm.enabled) return false;
    if (!testIsAlarmActiveOnDay(alarm.days, curDayOfWeek)) return false;
    return (alarm.hour == curHour && alarm.minute == curMin);
}

uint8_t testGetNextEnabledPage(uint8_t current, const bool* pagesEnabled, uint8_t totalPages = 5) {
    uint8_t p = current;
    for (int i = 0; i < totalPages; i++) {
        p = (p >= totalPages) ? 1 : (p + 1);
        if (pagesEnabled[p - 1]) return p;
    }
    return current;
}

uint8_t testGetPrevEnabledPage(uint8_t current, const bool* pagesEnabled, uint8_t totalPages = 5) {
    uint8_t p = current;
    for (int i = 0; i < totalPages; i++) {
        p = (p <= 1) ? totalPages : (p - 1);
        if (pagesEnabled[p - 1]) return p;
    }
    return current;
}

bool validateWebApiAlarm(int id, int hour, int minute, int days, int melody = 0) {
    if (id < 1 || id > 10) return false;
    if (hour < 0 || hour > 23) return false;
    if (minute < 0 || minute > 59) return false;
    if (days < 0 || days > 127) return false;
    if (melody < 0 || melody > 13) return false;
    return true;
}

void test_alarm_days_mask_and_matching(void) {
    // 0: Domingo, 1: Lunes, 2: Martes, 3: Miércoles, 4: Jueves, 5: Viernes, 6: Sábado

    // Todos los días
    for (int day = 0; day <= 6; day++) {
        TEST_ASSERT_TRUE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_ALL, day));
    }

    // Lunes a Viernes
    TEST_ASSERT_FALSE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_WEEKDAYS, 0)); // Domingo
    TEST_ASSERT_TRUE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_WEEKDAYS, 1));  // Lunes
    TEST_ASSERT_TRUE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_WEEKDAYS, 2));  // Martes
    TEST_ASSERT_TRUE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_WEEKDAYS, 3));  // Miércoles
    TEST_ASSERT_TRUE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_WEEKDAYS, 4));  // Jueves
    TEST_ASSERT_TRUE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_WEEKDAYS, 5));  // Viernes
    TEST_ASSERT_FALSE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_WEEKDAYS, 6)); // Sábado

    // Fin de semana
    TEST_ASSERT_TRUE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_WEEKEND, 0));  // Domingo
    TEST_ASSERT_FALSE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_WEEKEND, 1)); // Lunes
    TEST_ASSERT_FALSE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_WEEKEND, 5)); // Viernes
    TEST_ASSERT_TRUE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_WEEKEND, 6));  // Sábado

    // Máscara personalizada: Martes (bit 2) y Jueves (bit 4) = (1<<2)|(1<<4) = 4 + 16 = 20
    uint8_t customMask = (1 << 2) | (1 << 4);
    TEST_ASSERT_FALSE(testIsAlarmActiveOnDay(customMask, 0));
    TEST_ASSERT_FALSE(testIsAlarmActiveOnDay(customMask, 1));
    TEST_ASSERT_TRUE(testIsAlarmActiveOnDay(customMask, 2));
    TEST_ASSERT_FALSE(testIsAlarmActiveOnDay(customMask, 3));
    TEST_ASSERT_TRUE(testIsAlarmActiveOnDay(customMask, 4));
    TEST_ASSERT_FALSE(testIsAlarmActiveOnDay(customMask, 5));
    TEST_ASSERT_FALSE(testIsAlarmActiveOnDay(customMask, 6));

    // Día inválido fuera de rango
    TEST_ASSERT_FALSE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_ALL, -1));
    TEST_ASSERT_FALSE(testIsAlarmActiveOnDay(TEST_ALARM_DAYS_ALL, 7));
}

void test_alarm_days_string_formatting(void) {
    char buf[32];

    testFormatAlarmDays(TEST_ALARM_DAYS_ALL, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("Todos los dias", buf);

    testFormatAlarmDays(TEST_ALARM_DAYS_WEEKDAYS, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("Lun a Vie", buf);

    testFormatAlarmDays(TEST_ALARM_DAYS_WEEKEND, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("Fin de semana", buf);

    testFormatAlarmDays(0, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("Desactivado", buf);

    // Lunes, Miércoles, Viernes = (1<<1) | (1<<3) | (1<<5)
    uint8_t lmv = (1 << 1) | (1 << 3) | (1 << 5);
    testFormatAlarmDays(lmv, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("L X V", buf);
}

void test_alarm_trigger_detection(void) {
    TestAlarm a1 = { 1, true, 7, 0, TEST_ALARM_DAYS_WEEKDAYS };

    // Coincidencia exacta un Lunes (1) a las 07:00 -> Dispara
    TEST_ASSERT_TRUE(testShouldTriggerAlarm(a1, 7, 0, 1));

    // Coincidencia de hora pero en Sábado (6) -> No dispara
    TEST_ASSERT_FALSE(testShouldTriggerAlarm(a1, 7, 0, 6));

    // Mismo día pero 07:01 -> No dispara
    TEST_ASSERT_FALSE(testShouldTriggerAlarm(a1, 7, 1, 1));

    // Mismo día pero 06:59 -> No dispara
    TEST_ASSERT_FALSE(testShouldTriggerAlarm(a1, 6, 59, 1));

    // Alarma deshabilitada -> No dispara
    a1.enabled = false;
    TEST_ASSERT_FALSE(testShouldTriggerAlarm(a1, 7, 0, 1));
}

void test_alarm_night_mode_dismiss_restore(void) {
    SleepState state = { true, false, true, false, 0, 0 }; // Pantalla durmiendo por noche (03:00)
    uint8_t nightHour = 3;

    // 1. Suena la alarma de noche: la pantalla despierta
    bool alarmWokeFromSleep = state.isSleeping;
    state.isSleeping = false;
    TEST_ASSERT_FALSE(state.isSleeping);
    TEST_ASSERT_TRUE(alarmWokeFromSleep);

    // 2. Usuario silencia la alarma mientras continúa en horario de noche
    bool isNight = isNightHour(nightHour, 23, 7);
    TEST_ASSERT_TRUE(isNight);

    if (isNight && alarmWokeFromSleep) {
        state.isSleeping = true;
        state.nightAutoSleepActive = true;
    }

    // Comprobamos que volvió a apagarse automáticamente
    TEST_ASSERT_TRUE(state.isSleeping);
    TEST_ASSERT_TRUE(state.nightAutoSleepActive);

    // 3. Caso alternativo: la alarma suena de día (08:30) y se silencia
    state.isSleeping = false;
    state.nightAutoSleepActive = false;
    uint8_t dayHour = 8;
    bool isDayNight = isNightHour(dayHour, 23, 7);
    TEST_ASSERT_FALSE(isDayNight);

    // Al silenciarla de día, no se apaga la pantalla
    if (isDayNight) {
        state.isSleeping = true;
    }
    TEST_ASSERT_FALSE(state.isSleeping);
}

void test_page_enabled_navigation_skipping(void) {
    // Caso 1: Todas las páginas habilitadas [1, 2, 3, 4, 5]
    bool allPages[5] = { true, true, true, true, true };
    TEST_ASSERT_EQUAL_UINT8(2, testGetNextEnabledPage(1, allPages));
    TEST_ASSERT_EQUAL_UINT8(3, testGetNextEnabledPage(2, allPages));
    TEST_ASSERT_EQUAL_UINT8(4, testGetNextEnabledPage(3, allPages));
    TEST_ASSERT_EQUAL_UINT8(5, testGetNextEnabledPage(4, allPages));
    TEST_ASSERT_EQUAL_UINT8(1, testGetNextEnabledPage(5, allPages));

    TEST_ASSERT_EQUAL_UINT8(5, testGetPrevEnabledPage(1, allPages));
    TEST_ASSERT_EQUAL_UINT8(4, testGetPrevEnabledPage(5, allPages));

    // Caso 2: Páginas 2 y 4 deshabilitadas [1, -, 3, -, 5]
    bool pagesSubset[5] = { true, false, true, false, true };
    TEST_ASSERT_EQUAL_UINT8(3, testGetNextEnabledPage(1, pagesSubset));
    TEST_ASSERT_EQUAL_UINT8(5, testGetNextEnabledPage(3, pagesSubset));
    TEST_ASSERT_EQUAL_UINT8(1, testGetNextEnabledPage(5, pagesSubset));

    TEST_ASSERT_EQUAL_UINT8(5, testGetPrevEnabledPage(1, pagesSubset));
    TEST_ASSERT_EQUAL_UINT8(3, testGetPrevEnabledPage(5, pagesSubset));
    TEST_ASSERT_EQUAL_UINT8(1, testGetPrevEnabledPage(3, pagesSubset));

    // Caso 3: Solo Página 5 habilitada [-, -, -, -, 5]
    bool singlePage[5] = { false, false, false, false, true };
    TEST_ASSERT_EQUAL_UINT8(5, testGetNextEnabledPage(5, singlePage));
    TEST_ASSERT_EQUAL_UINT8(5, testGetPrevEnabledPage(5, singlePage));
    TEST_ASSERT_EQUAL_UINT8(5, testGetNextEnabledPage(1, singlePage));
}

void test_alarm_api_validation(void) {
    // Parámetros válidos
    TEST_ASSERT_TRUE(validateWebApiAlarm(1, 7, 0, 127, 0));
    TEST_ASSERT_TRUE(validateWebApiAlarm(1, 7, 0, 127, 2)); // Super Mario
    TEST_ASSERT_TRUE(validateWebApiAlarm(10, 23, 59, 62, 13)); // Alien
    TEST_ASSERT_TRUE(validateWebApiAlarm(5, 0, 0, 0, 1));

    // Melodía inválida (<0 o >13)
    TEST_ASSERT_FALSE(validateWebApiAlarm(1, 7, 0, 127, -1));
    TEST_ASSERT_FALSE(validateWebApiAlarm(1, 7, 0, 127, 14));

    // ID inválido (0 o 11)
    TEST_ASSERT_FALSE(validateWebApiAlarm(0, 7, 0, 127));
    TEST_ASSERT_FALSE(validateWebApiAlarm(11, 7, 0, 127));

    // Hora inválida
    TEST_ASSERT_FALSE(validateWebApiAlarm(1, -1, 0, 127));
    TEST_ASSERT_FALSE(validateWebApiAlarm(1, 24, 0, 127));

    // Minuto inválido
    TEST_ASSERT_FALSE(validateWebApiAlarm(1, 7, -1, 127));
    TEST_ASSERT_FALSE(validateWebApiAlarm(1, 7, 60, 127));

    // Días fuera de rango 0..127
    TEST_ASSERT_FALSE(validateWebApiAlarm(1, 7, 0, -1));
    TEST_ASSERT_FALSE(validateWebApiAlarm(1, 7, 0, 128));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_manual_sleep_during_day_stays_off);
    RUN_TEST(test_sleep_toggle_cooldown_prevents_bounce);
    RUN_TEST(test_scheduled_night_mode_sleep_and_wake);
    RUN_TEST(test_manual_wake_during_night_timeout);
    RUN_TEST(test_weather_icon_day_night);
    RUN_TEST(test_uv_categories);
    RUN_TEST(test_boot_portal_button_detection);
    RUN_TEST(test_settings_validation);
    RUN_TEST(test_web_api_timer_validation);
    RUN_TEST(test_web_api_page_validation);
    RUN_TEST(test_alarm_days_mask_and_matching);
    RUN_TEST(test_alarm_days_string_formatting);
    RUN_TEST(test_alarm_trigger_detection);
    RUN_TEST(test_alarm_night_mode_dismiss_restore);
    RUN_TEST(test_page_enabled_navigation_skipping);
    RUN_TEST(test_alarm_api_validation);
    return UNITY_END();
}
