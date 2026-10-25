#include <unity.h>
#include <cstdint>
#include <cstring>

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

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_manual_sleep_during_day_stays_off);
    RUN_TEST(test_sleep_toggle_cooldown_prevents_bounce);
    RUN_TEST(test_scheduled_night_mode_sleep_and_wake);
    RUN_TEST(test_manual_wake_during_night_timeout);
    RUN_TEST(test_weather_icon_day_night);
    RUN_TEST(test_uv_categories);
    return UNITY_END();
}
