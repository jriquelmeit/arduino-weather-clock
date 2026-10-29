#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "config.h"
#include "settings_manager.h"
#include "web_server_service.h"
#include "time_service.h"
#include "weather_service.h"
#include "display_ui.h"
#include "input_controller.h"
#include "buzzer_controller.h"

// Instancias principales
static SettingsManager settingsMgr;
static DisplayUI ui(TFT_CS, TFT_DC, TFT_RST, TFT_LED_PIN);
static WeatherService weatherService;
static WeatherData weatherData;
static InputController inputCtrl(PIN_BTN_PREV, PIN_BTN_NEXT, PIN_BTN_ACTION, TIMER_LONG_PRESS_MS);
static BuzzerController buzzer(PIN_BUZZER, ALARM_MELODY_TYPE);

static uint8_t currentPage = DISPLAY_PAGE_MODE;
static bool isPortalMode = false;

// Estado del Temporizador (Página 4)
static TimerDisplayState timerState = TIMER_UI_IDLE;
static uint32_t timerSelectedSec = TIMER_DEFAULT_MINUTES * 60;
static uint32_t timerTotalSec = TIMER_DEFAULT_MINUTES * 60;
static uint32_t timerRemainingSec = TIMER_DEFAULT_MINUTES * 60;
static unsigned long lastTimerTick = 0;

// Estado del Despertador (Página 5)
static uint8_t selectedAlarmIndex = 0;
static bool alarmEditMode = false;
static AlarmEditField alarmEditField = ALARM_FIELD_NONE;

// Estado de disparo de la alarma diaria
static bool dailyAlarmActive = false;
static unsigned long dailyAlarmStartTime = 0;
static int lastDailyAlarmMinute = -1;
static bool alarmWokeFromSleep = false;
static char currentAlarmLabel[ALARM_LABEL_MAX_LEN + 1] = "";
static char currentAlarmTimeStr[8] = "";

// Modo noche y reposo
static bool manualSleepActive = false;
static bool nightAutoSleepActive = false;
static bool manualWakeActive = false;
static unsigned long manualWakeTime = 0;
static unsigned long lastSleepToggleTime = 0;

// Temporizadores periódicos
static unsigned long lastClockTick = 0;
static unsigned long lastWeatherAttempt = 0;
static unsigned long lastWifiStatusCheck = 0;
static bool initialWeatherFetched = false;

void switchPage(uint8_t newPage, bool fullRedraw = false);
void triggerDailyAlarm(const DailyAlarm& alarm, const DateTimeInfo& dt);
void stopDailyAlarm();
void checkDailyAlarms(const DateTimeInfo& dt);

// Declaración previa de WebServerCallbacks
static WebServerCallbacks buildServerCallbacks();
static WebServerService webServer(settingsMgr, buildServerCallbacks());

void triggerDailyAlarm(const DailyAlarm& alarm, const DateTimeInfo& dt) {
    dailyAlarmActive = true;
    dailyAlarmStartTime = millis();
    alarmWokeFromSleep = ui.isSleeping() || nightAutoSleepActive || manualSleepActive;

    strncpy(currentAlarmLabel, alarm.label, sizeof(currentAlarmLabel) - 1);
    currentAlarmLabel[sizeof(currentAlarmLabel) - 1] = '\0';
    snprintf(currentAlarmTimeStr, sizeof(currentAlarmTimeStr), "%02d:%02d", alarm.hour, alarm.minute);

    Serial.printf("[Alarma] !DESPERTADOR ACTIVADO! Alarma [%u] %s - %s\n", alarm.id, currentAlarmTimeStr, currentAlarmLabel);

    if (ui.isSleeping()) {
        ui.wakeDisplay();
    }
    ui.showAlarmAlert(currentAlarmTimeStr, currentAlarmLabel);
    buzzer.startAlarm(ALARM_MAX_RING_DURATION_MS);
}

void stopDailyAlarm() {
    if (!dailyAlarmActive) return;
    Serial.println(F("[Alarma] Despertador silenciado."));
    dailyAlarmActive = false;
    buzzer.stop();

    DateTimeInfo dt;
    bool isNight = false;
    if (getCurrentDateTime(dt)) {
        if (NIGHT_MODE_START_HOUR > NIGHT_MODE_END_HOUR) {
            isNight = (dt.hour >= NIGHT_MODE_START_HOUR || dt.hour < NIGHT_MODE_END_HOUR);
        } else {
            isNight = (dt.hour >= NIGHT_MODE_START_HOUR && dt.hour < NIGHT_MODE_END_HOUR);
        }
    }

    if (NIGHT_MODE_ENABLED && isNight && (alarmWokeFromSleep || nightAutoSleepActive)) {
        Serial.println(F("[Alarma] Regresando a modo noche (pantalla apagada)."));
        nightAutoSleepActive = true;
        manualWakeActive = false;
        manualSleepActive = false;
        ui.sleepDisplay();
    } else {
        alarmWokeFromSleep = false;
        manualWakeActive = false;
        ui.wakeDisplay();
        switchPage(currentPage, false);
    }
}

void checkDailyAlarms(const DateTimeInfo& dt) {
    if (dt.minute == lastDailyAlarmMinute) return;
    lastDailyAlarmMinute = dt.minute;

    const DailyAlarm* alarms = settingsMgr.get().alarms;
    for (uint8_t i = 0; i < MAX_DAILY_ALARMS; i++) {
        if (alarms[i].enabled &&
            isAlarmActiveOnDay(alarms[i].days, dt.dayOfWeek) &&
            alarms[i].hour == dt.hour &&
            alarms[i].minute == dt.minute) {
            triggerDailyAlarm(alarms[i], dt);
            break;
        }
    }
}

void switchPage(uint8_t newPage, bool fullRedraw) {
    uint8_t prevPage = currentPage;
    currentPage = newPage;
    bool forceFull = fullRedraw || (newPage == 3) || (prevPage == 3);
    ui.drawBaseLayout(settingsMgr.get().locationName, currentPage, forceFull);

    DateTimeInfo dt;
    bool hasDt = getCurrentDateTime(dt);
    if (hasDt) {
        ui.updateDate(dt);
        if (currentPage != 4 && currentPage != 5) {
            ui.updateTime(dt);
        }
    }

    if (currentPage == 5) {
        ui.updateAlarmUI(settingsMgr.get().alarms, MAX_DAILY_ALARMS, selectedAlarmIndex, alarmEditMode, alarmEditField, true);
    } else if (currentPage == 4) {
        ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
    } else {
        if (weatherData.valid) {
            ui.updateWeather(weatherData, currentPage);
        }
    }

    if (forceFull) {
        ui.updateWifiStatus(WiFi.status() == WL_CONNECTED, WiFi.RSSI(), WiFi.localIP());
        char statusMsg[64];
        if (hasDt) {
            snprintf(statusMsg, sizeof(statusMsg), "IP:%s | RSSI:%ddBm | Act:%s",
                     WiFi.localIP().toString().c_str(),
                     WiFi.RSSI(),
                     dt.shortTimeStr);
        } else {
            snprintf(statusMsg, sizeof(statusMsg), "IP:%s | RSSI:%ddBm | Clima OK",
                     WiFi.localIP().toString().c_str(),
                     WiFi.RSSI());
        }
        ui.updateStatusBar(statusMsg);
    }
}

static WebServerCallbacks buildServerCallbacks() {
    WebServerCallbacks cb;

    cb.onPageChange = [](uint8_t page) {
        if (page >= 1 && page <= TOTAL_PAGES && settingsMgr.get().isPageEnabled(page)) {
            switchPage(page, false);
        }
    };

    cb.onToggleSleep = []() {
        if (ui.isSleeping()) {
            manualSleepActive = false;
            nightAutoSleepActive = false;
            DateTimeInfo dt;
            if (getCurrentDateTime(dt)) {
                if (currentPage != 4 && currentPage != 5) {
                    ui.updateTime(dt);
                }
                ui.updateDate(dt);
            }
            ui.wakeDisplay();
        } else {
            manualSleepActive = true;
            nightAutoSleepActive = false;
            manualWakeActive = false;
            ui.sleepDisplay();
        }
    };

    cb.isSleeping = []() -> bool {
        return ui.isSleeping();
    };

    cb.getCurrentPage = []() -> uint8_t {
        return currentPage;
    };

    cb.onTimerStart = [](uint32_t sec) {
        if (sec < 5) sec = 5;
        if (sec > 86400) sec = 86400; // Máximo 24 horas
        timerSelectedSec = sec;
        timerTotalSec = sec;
        timerRemainingSec = timerTotalSec;
        timerState = TIMER_UI_RUNNING;
        lastTimerTick = millis();
        if (currentPage == 4) {
            ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
        }
    };

    cb.onTimerStop = []() {
        timerState = TIMER_UI_IDLE;
        buzzer.stop();
        timerRemainingSec = timerSelectedSec;
        timerTotalSec = timerRemainingSec;
        if (currentPage == 4) {
            ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
        }
    };

    cb.onTimerSetSeconds = [](uint32_t sec) {
        if (sec < 5) sec = 5;
        if (sec > 86400) sec = 86400;
        timerSelectedSec = sec;
        timerTotalSec = sec;
        timerRemainingSec = timerTotalSec;
        if (currentPage == 4) {
            ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
        }
    };

    cb.getTimerStatus = [](uint32_t& rem, uint32_t& tot, uint8_t& st) {
        rem = timerRemainingSec;
        tot = timerTotalSec;
        st = static_cast<uint8_t>(timerState);
    };

    cb.onSetMelody = [](uint8_t melody) {
        buzzer.setMelodyType(melody);
    };

    cb.onTestAlarm = []() {
        buzzer.startAlarm(8000); // Probar alarma durante 8 segundos
    };

    cb.onStopAlarm = []() {
        if (dailyAlarmActive) {
            stopDailyAlarm();
        } else {
            buzzer.stop();
        }
    };

    cb.isAlarming = []() -> bool {
        return buzzer.isAlarming() || dailyAlarmActive;
    };

    cb.getCurrentMelody = []() -> uint8_t {
        return settingsMgr.get().alarmMelody;
    };

    cb.getWeatherInfo = [](char* timeBuf, size_t timeLen, float& temp, int& hum, char* condBuf, size_t condLen) {
        DateTimeInfo dt;
        if (getCurrentDateTime(dt)) {
            snprintf(timeBuf, timeLen, "%s", dt.timeStr);
        } else {
            snprintf(timeBuf, timeLen, "--:--");
        }

        if (weatherData.valid) {
            temp = weatherData.current.temperature;
            hum = weatherData.current.humidity;
            snprintf(condBuf, condLen, "%s", weatherData.current.conditionText);
        } else {
            temp = 0.0f;
            hum = 0;
            snprintf(condBuf, condLen, "Sin datos de clima");
        }
    };

    cb.onSaveConfig = [](const char* ssid, const char* pass, const char* loc, const char* lat, const char* lon,
                         uint8_t melody, uint8_t page,
                         bool p1, bool p2, bool p3, bool p4, bool p5) -> bool {
        settingsMgr.setWifi(ssid, pass);
        settingsMgr.setLocation(loc, lat, lon);
        settingsMgr.setAlarmMelody(melody);
        settingsMgr.setDefaultPage(page);
        settingsMgr.setPageEnabled(1, p1);
        settingsMgr.setPageEnabled(2, p2);
        settingsMgr.setPageEnabled(3, p3);
        settingsMgr.setPageEnabled(4, p4);
        settingsMgr.setPageEnabled(5, p5);
        return settingsMgr.save();
    };

    cb.onGetAlarms = [](DailyAlarm* outAlarms, uint8_t& count) {
        count = MAX_DAILY_ALARMS;
        if (outAlarms) {
            memcpy(outAlarms, settingsMgr.get().alarms, sizeof(DailyAlarm) * MAX_DAILY_ALARMS);
        }
    };

    cb.onSetAlarm = [](uint8_t id, bool enabled, uint8_t hour, uint8_t minute, uint8_t days, const char* label) -> bool {
        if (id < 1 || id > MAX_DAILY_ALARMS) return false;
        DailyAlarm a;
        a.id = id;
        a.enabled = enabled;
        a.hour = hour;
        a.minute = minute;
        a.days = days;
        strncpy(a.label, label ? label : "", ALARM_LABEL_MAX_LEN);
        a.label[ALARM_LABEL_MAX_LEN] = '\0';
        bool ok = settingsMgr.setAlarm(id - 1, a);
        if (ok) {
            settingsMgr.save();
            if (currentPage == 5) {
                ui.updateAlarmUI(settingsMgr.get().alarms, MAX_DAILY_ALARMS, selectedAlarmIndex, alarmEditMode, alarmEditField);
            }
        }
        return ok;
    };

    cb.onToggleAlarm = [](uint8_t id) -> bool {
        if (id < 1 || id > MAX_DAILY_ALARMS) return false;
        bool newState = settingsMgr.toggleAlarm(id - 1);
        settingsMgr.save();
        if (currentPage == 5) {
            ui.updateAlarmUI(settingsMgr.get().alarms, MAX_DAILY_ALARMS, selectedAlarmIndex, alarmEditMode, alarmEditField);
        }
        return newState;
    };

    cb.onSetPageEnabled = [](uint8_t page, bool enabled) {
        settingsMgr.setPageEnabled(page, enabled);
        settingsMgr.save();
    };

    cb.isPageEnabled = [](uint8_t page) -> bool {
        return settingsMgr.get().isPageEnabled(page);
    };

    return cb;
}

void handleNightMode(const DateTimeInfo& dt) {
    if (!NIGHT_MODE_ENABLED) return;

    // Si el timer está corriendo o alguna alarma suena, no apagar pantalla
    if (timerState == TIMER_UI_RUNNING || timerState == TIMER_UI_ALARM || dailyAlarmActive) return;

    bool isNight;
    if (NIGHT_MODE_START_HOUR > NIGHT_MODE_END_HOUR) {
        isNight = (dt.hour >= NIGHT_MODE_START_HOUR || dt.hour < NIGHT_MODE_END_HOUR);
    } else {
        isNight = (dt.hour >= NIGHT_MODE_START_HOUR && dt.hour < NIGHT_MODE_END_HOUR);
    }

    unsigned long now = millis();
    if (isNight) {
        if (!ui.isSleeping()) {
            if (manualWakeActive) {
                if (now - manualWakeTime >= NIGHT_MODE_WAKE_TIMEOUT_MS) {
                    manualWakeActive = false;
                    nightAutoSleepActive = true;
                    ui.sleepDisplay();
                }
            } else {
                nightAutoSleepActive = true;
                ui.sleepDisplay();
            }
        }
    } else {
        // Es de día
        manualWakeActive = false;
        if (nightAutoSleepActive && ui.isSleeping()) {
            nightAutoSleepActive = false;
            manualSleepActive = false;
            DateTimeInfo dtNow;
            if (getCurrentDateTime(dtNow)) {
                if (currentPage != 4 && currentPage != 5) {
                    ui.updateTime(dtNow);
                }
                ui.updateDate(dtNow);
            }
            ui.wakeDisplay();
        }
    }
}

bool connectWiFi(const char* ssid, const char* pass) {
    Serial.println(F("\n========================================"));
    Serial.printf("Conectando a red Wi-Fi: %s\n", ssid);
    Serial.println(F("========================================"));

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    uint8_t dots = 0;
    unsigned long startAttempt = millis();

    while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt < 15000)) {
        delay(100);
        Serial.print(F("."));
        if ((dots % 4) == 0) {
            ui.updateBootProgress("Esperando conexion", dots / 4);
        }
        dots++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("\n[Wi-Fi] Conectado con exito!"));
        Serial.printf("  IP Asignada: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("  Senal (RSSI): %d dBm\n", WiFi.RSSI());
        ui.updateBootProgress("Wi-Fi Conectado OK", 0);
        return true;
    } else {
        Serial.println(F("\n[Wi-Fi] Tiempo de espera agotado."));
        ui.updateBootProgress("Sin conexion directa", 0);
        return false;
    }
}

void setup() {
    Serial.begin(115200);
    delay(100);

    // 0. Detección de Botón 3 presionado al enchufar / encender para modo Portal AP
    pinMode(PIN_BTN_ACTION, INPUT_PULLUP);
    delay(50);
    bool btnActionPressedAtBoot = false;
    if (digitalRead(PIN_BTN_ACTION) == LOW) {
        Serial.println(F("[Boot] Boton 3 detectado en LOW. Verificando pulsacion sostenida..."));
        unsigned long holdStart = millis();
        while (digitalRead(PIN_BTN_ACTION) == LOW && (millis() - holdStart < 800)) {
            delay(20);
        }
        if (millis() - holdStart >= 800) {
            btnActionPressedAtBoot = true;
            Serial.println(F("[Boot] ¡Boton 3 sostenido durante el arranque! Activando modo Portal AP."));
        }
    }

    Serial.println(F("\n=============================================="));
    Serial.println(F("   ESP8266 Weather Clock - TFT ILI9341 SPI   "));
    Serial.println(F("=============================================="));

    // 1. Inicializar pantalla TFT
    ui.begin(TFT_ROTATION);

    // 2. Inicializar sistema de almacenamiento y configuraciones persistentes
    settingsMgr.begin();
    currentPage = settingsMgr.get().defaultPage;
    if (!settingsMgr.get().isPageEnabled(currentPage)) {
        currentPage = settingsMgr.get().getFirstEnabledPage();
    }
    buzzer.setMelodyType(settingsMgr.get().alarmMelody);

    // 3. Inicializar botones y buzzer
    inputCtrl.begin();
    buzzer.begin();

    // 4. Evaluar si entrar en Modo Portal de Configuración AP
    bool noValidWifiConfig = (strlen(settingsMgr.get().wifiSsid) == 0);
    if (btnActionPressedAtBoot || noValidWifiConfig) {
        isPortalMode = true;
        buzzer.beep(120, 1500);
        ui.showConfigPortalScreen("WeatherClock-Setup", "192.168.4.1");
        webServer.beginPortalAP("WeatherClock-Setup");
        return;
    }

    // Melodía de bienvenida opcional
    if (AUDIO_STARTUP_MELODY) {
        Serial.printf("[Audio] Reproduciendo melodia de inicio (Melodia: %d)...\n", settingsMgr.get().alarmMelody);
        buzzer.playStartupMelody(PIN_BTN_PREV, PIN_BTN_NEXT, PIN_BTN_ACTION);
    }

    // 5. Conectar a red Wi-Fi
    ui.showBootScreen(settingsMgr.get().wifiSsid);
    bool wifiOk = connectWiFi(settingsMgr.get().wifiSsid, settingsMgr.get().wifiPassword);

    // Si falló la conexión por completo y se desea portal de contingencia:
    if (!wifiOk) {
        Serial.println(F("[Wi-Fi] Fallo de conexion. Continuando con reloj offline..."));
    }

    // 6. Inicializar servicio de tiempo (NTP)
    Serial.println(F("[NTP] Configurando sincronizacion horaria y zona horaria..."));
    initTimeService(TIMEZONE_RULE, NTP_SERVER_1, NTP_SERVER_2);

    unsigned long ntpWait = millis();
    uint8_t step = 0;
    while (!isTimeSynchronized() && (millis() - ntpWait < 4000)) {
        delay(100);
        if ((step % 3) == 0) {
            ui.updateBootProgress("Sincronizando hora NTP", step / 3);
        }
        step++;
    }

    if (isTimeSynchronized()) {
        Serial.println(F("[NTP] Hora sincronizada correctamente."));
    } else {
        Serial.println(F("[NTP] Esperando respuesta de servidor de tiempo..."));
    }

    // 7. Iniciar Servidor Web en modo Estación (accesible en la red local)
    if (wifiOk) {
        webServer.beginStationServer();
    }

    // 8. Dibujar plantilla inicial (redibujado completo)
    switchPage(currentPage, true);

    // 9. Carga inicial del pronóstico meteorológico si hay Wi-Fi
    if (WiFi.status() == WL_CONNECTED) {
        ui.updateStatusBar("Descargando pronostico Open-Meteo...");
        if (weatherService.fetchWeather(settingsMgr.get().locationLat,
                                       settingsMgr.get().locationLon,
                                       weatherData,
                                       HOURLY_FORECAST_STEP)) {
            if (currentPage != 4) {
                ui.updateWeather(weatherData, currentPage);
            }
            initialWeatherFetched = true;
            ui.updateStatusBar("Pronostico cargado OK");
        }
    }
}

void loop() {
    // Si estamos en Modo Portal de Configuración AP
    if (isPortalMode) {
        webServer.update();
        buzzer.update();
        delay(5);
        return;
    }

    unsigned long now = millis();
    bool isConnected = (WiFi.status() == WL_CONNECTED);

    // Actualizar máquinas de estado
    buzzer.update();
    webServer.update();

    // ----------------------------------------------------
    // GESTIÓN DE ALARMA DESPERTADOR (Auto-off por duración)
    // ----------------------------------------------------
    if (dailyAlarmActive) {
        if (now - dailyAlarmStartTime >= ALARM_MAX_RING_DURATION_MS) {
            stopDailyAlarm();
        }
    }

    // ----------------------------------------------------
    // GESTIÓN DE BOTONES
    // ----------------------------------------------------
    ButtonEvent evPrev = inputCtrl.updatePrev();
    ButtonEvent evNext = inputCtrl.updateNext();
    ButtonEvent evAction = inputCtrl.updateAction();

    bool anyButtonActivity = (evPrev != BTN_EVENT_NONE || evNext != BTN_EVENT_NONE || evAction != BTN_EVENT_NONE);

    // Si la alarma del despertador o del temporizador está sonando, cualquier botón la silencia
    if (anyButtonActivity && (dailyAlarmActive || buzzer.isAlarming())) {
        if (dailyAlarmActive) {
            stopDailyAlarm();
        } else {
            buzzer.stop();
            timerState = TIMER_UI_IDLE;
            timerRemainingSec = timerSelectedSec;
            timerTotalSec = timerRemainingSec;
            if (currentPage == 4) {
                ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
            }
        }
        return;
    }

    // Si la pantalla estaba apagada y se pulsa cualquier botón, despertar
    if (anyButtonActivity && ui.isSleeping()) {
        if (now - lastSleepToggleTime >= 350) {
            manualSleepActive = false;
            nightAutoSleepActive = false;
            lastSleepToggleTime = now;

            DateTimeInfo dt;
            if (getCurrentDateTime(dt)) {
                bool isNight;
                if (NIGHT_MODE_START_HOUR > NIGHT_MODE_END_HOUR) {
                    isNight = (dt.hour >= NIGHT_MODE_START_HOUR || dt.hour < NIGHT_MODE_END_HOUR);
                } else {
                    isNight = (dt.hour >= NIGHT_MODE_START_HOUR && dt.hour < NIGHT_MODE_END_HOUR);
                }
                if (isNight) {
                    manualWakeActive = true;
                    manualWakeTime = now;
                }
                if (currentPage != 4 && currentPage != 5) {
                    ui.updateTime(dt);
                }
                ui.updateDate(dt);
            }

            ui.wakeDisplay();
            if (BUZZER_KEY_BEEP) buzzer.beep(30);
        }
        return;
    }

    // Botón Siguiente (Next)
    if (evNext == BTN_EVENT_SHORT_CLICK) {
        if (currentPage == 5 && alarmEditMode) {
            DailyAlarm& a = settingsMgr.get().alarms[selectedAlarmIndex];
            if (alarmEditField == ALARM_FIELD_ENABLE) {
                a.enabled = !a.enabled;
            } else if (alarmEditField == ALARM_FIELD_HOUR) {
                a.hour = (a.hour >= 23) ? 0 : (a.hour + 1);
            } else if (alarmEditField == ALARM_FIELD_MINUTE) {
                a.minute = (a.minute >= 55) ? 0 : (a.minute + 5);
            } else if (alarmEditField == ALARM_FIELD_DAYS) {
                if (a.days == ALARM_DAYS_ALL) a.days = ALARM_DAYS_WEEKDAYS;
                else if (a.days == ALARM_DAYS_WEEKDAYS) a.days = ALARM_DAYS_WEEKEND;
                else a.days = ALARM_DAYS_ALL;
            }
            if (BUZZER_KEY_BEEP) buzzer.beep(25);
            ui.updateAlarmUI(settingsMgr.get().alarms, MAX_DAILY_ALARMS, selectedAlarmIndex, alarmEditMode, alarmEditField);
        } else {
            Serial.println(F("[BTN] Siguiente (Next)"));
            if (BUZZER_KEY_BEEP) buzzer.beep(25);
            uint8_t nextPage = settingsMgr.get().getNextEnabledPage(currentPage);
            switchPage(nextPage, false);
        }
    }

    // Botón Anterior (Prev)
    if (evPrev == BTN_EVENT_SHORT_CLICK) {
        if (currentPage == 5 && alarmEditMode) {
            DailyAlarm& a = settingsMgr.get().alarms[selectedAlarmIndex];
            if (alarmEditField == ALARM_FIELD_ENABLE) {
                a.enabled = !a.enabled;
            } else if (alarmEditField == ALARM_FIELD_HOUR) {
                a.hour = (a.hour == 0) ? 23 : (a.hour - 1);
            } else if (alarmEditField == ALARM_FIELD_MINUTE) {
                a.minute = (a.minute == 0) ? 55 : (a.minute - 5);
            } else if (alarmEditField == ALARM_FIELD_DAYS) {
                if (a.days == ALARM_DAYS_ALL) a.days = ALARM_DAYS_WEEKEND;
                else if (a.days == ALARM_DAYS_WEEKEND) a.days = ALARM_DAYS_WEEKDAYS;
                else a.days = ALARM_DAYS_ALL;
            }
            if (BUZZER_KEY_BEEP) buzzer.beep(25);
            ui.updateAlarmUI(settingsMgr.get().alarms, MAX_DAILY_ALARMS, selectedAlarmIndex, alarmEditMode, alarmEditField);
        } else {
            Serial.println(F("[BTN] Anterior (Prev)"));
            if (BUZZER_KEY_BEEP) buzzer.beep(25);
            uint8_t prevPage = settingsMgr.get().getPrevEnabledPage(currentPage);
            switchPage(prevPage, false);
        }
    }

    // Botón Acción
    if (evAction != BTN_EVENT_NONE) {
        Serial.printf("[BTN] Accion: %s en Pagina %d\n",
                      evAction == BTN_EVENT_SHORT_CLICK ? "Click Corto" : "Pulsacion Larga",
                      currentPage);

        if (currentPage == 1 || currentPage == 2 || currentPage == 3) {
            // En páginas 1, 2 y 3: Pulsación corta apaga la pantalla (Sleep)
            if (evAction == BTN_EVENT_SHORT_CLICK) {
                if (BUZZER_KEY_BEEP) buzzer.beep(30);
                manualSleepActive = true;
                nightAutoSleepActive = false;
                manualWakeActive = false;
                lastSleepToggleTime = now;
                ui.sleepDisplay();
            }
        } else if (currentPage == 4) {
            // En página 4: Control del temporizador
            if (timerState == TIMER_UI_IDLE) {
                if (evAction == BTN_EVENT_SHORT_CLICK) {
                    timerSelectedSec += TIMER_STEP_MINUTES * 60;
                    if (timerSelectedSec > TIMER_MAX_MINUTES * 60) {
                        timerSelectedSec = TIMER_MIN_MINUTES * 60;
                    }
                    timerTotalSec = timerSelectedSec;
                    timerRemainingSec = timerTotalSec;
                    if (BUZZER_KEY_BEEP) buzzer.beep(30);
                    ui.updateTimer(timerRemainingSec, timerTotalSec, timerState);
                } else if (evAction == BTN_EVENT_LONG_PRESS) {
                    timerState = TIMER_UI_RUNNING;
                    lastTimerTick = now;
                    if (BUZZER_KEY_BEEP) buzzer.beep(120);
                    ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
                }
            } else if (timerState == TIMER_UI_RUNNING) {
                if (evAction == BTN_EVENT_SHORT_CLICK) {
                    timerState = TIMER_UI_IDLE;
                    timerRemainingSec = timerSelectedSec;
                    timerTotalSec = timerRemainingSec;
                    if (BUZZER_KEY_BEEP) buzzer.beep(60);
                    ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
                }
            }
        } else if (currentPage == 5) {
            // En página 5: Despertador / Alarmas Diarias
            if (!alarmEditMode) {
                if (evAction == BTN_EVENT_SHORT_CLICK) {
                    selectedAlarmIndex = (selectedAlarmIndex + 1) % MAX_DAILY_ALARMS;
                    if (BUZZER_KEY_BEEP) buzzer.beep(25);
                    ui.updateAlarmUI(settingsMgr.get().alarms, MAX_DAILY_ALARMS, selectedAlarmIndex, alarmEditMode, alarmEditField);
                } else if (evAction == BTN_EVENT_LONG_PRESS) {
                    alarmEditMode = true;
                    alarmEditField = ALARM_FIELD_ENABLE;
                    if (BUZZER_KEY_BEEP) buzzer.beep(80);
                    ui.updateAlarmUI(settingsMgr.get().alarms, MAX_DAILY_ALARMS, selectedAlarmIndex, alarmEditMode, alarmEditField);
                }
            } else {
                if (evAction == BTN_EVENT_SHORT_CLICK) {
                    if (alarmEditField == ALARM_FIELD_ENABLE) alarmEditField = ALARM_FIELD_HOUR;
                    else if (alarmEditField == ALARM_FIELD_HOUR) alarmEditField = ALARM_FIELD_MINUTE;
                    else if (alarmEditField == ALARM_FIELD_MINUTE) alarmEditField = ALARM_FIELD_DAYS;
                    else alarmEditField = ALARM_FIELD_ENABLE;
                    if (BUZZER_KEY_BEEP) buzzer.beep(30);
                    ui.updateAlarmUI(settingsMgr.get().alarms, MAX_DAILY_ALARMS, selectedAlarmIndex, alarmEditMode, alarmEditField);
                } else if (evAction == BTN_EVENT_LONG_PRESS) {
                    alarmEditMode = false;
                    alarmEditField = ALARM_FIELD_NONE;
                    settingsMgr.save();
                    if (BUZZER_KEY_BEEP) buzzer.beep(120);
                    ui.updateAlarmUI(settingsMgr.get().alarms, MAX_DAILY_ALARMS, selectedAlarmIndex, alarmEditMode, alarmEditField);
                }
            }
        }
    }

    // ----------------------------------------------------
    // LÓGICA DEL TEMPORIZADOR (Conteo en segundo plano)
    // ----------------------------------------------------
    if (timerState == TIMER_UI_RUNNING) {
        if (now - lastTimerTick >= 1000) {
            lastTimerTick = now;
            if (timerRemainingSec > 0) {
                timerRemainingSec--;
            }

            if (currentPage == 4) {
                ui.updateTimer(timerRemainingSec, timerTotalSec, timerState);
            }

            if (timerRemainingSec == 0) {
                timerState = TIMER_UI_ALARM;
                buzzer.startAlarm(TIMER_ALARM_DURATION_MS);

                manualSleepActive = false;
                nightAutoSleepActive = false;
                manualWakeActive = false;
                if (ui.isSleeping()) {
                    ui.wakeDisplay();
                }

                if (currentPage != 4) {
                    switchPage(4, true);
                } else {
                    ui.updateTimer(0, timerTotalSec, timerState, true);
                }
            }
        }
    } else if (timerState == TIMER_UI_ALARM) {
        if (!buzzer.isAlarming()) {
            timerState = TIMER_UI_IDLE;
            timerRemainingSec = timerSelectedSec;
            timerTotalSec = timerRemainingSec;
            if (currentPage == 4) {
                ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
            }
        }
    }

    // ----------------------------------------------------
    // TAREAS PERIÓDICAS (Reloj, Modo Noche, Wi-Fi, Clima)
    // ----------------------------------------------------
    // 1. Refresco de reloj, alarmas diarias y comprobación de modo noche (cada 1 segundo)
    if (now - lastClockTick >= CLOCK_REFRESH_INTERVAL_MS) {
        lastClockTick = now;

        DateTimeInfo dt;
        if (getCurrentDateTime(dt)) {
            checkDailyAlarms(dt);
            handleNightMode(dt);
            if (!ui.isSleeping() && !dailyAlarmActive) {
                if (currentPage != 4 && currentPage != 5) {
                    ui.updateTime(dt);
                }
                ui.updateDate(dt);
            }
        }
    }

    // 2. Comprobación de estado Wi-Fi (cada 4 segundos)
    if (now - lastWifiStatusCheck >= 4000) {
        lastWifiStatusCheck = now;
        if (!ui.isSleeping()) {
            ui.updateWifiStatus(isConnected, WiFi.RSSI(), WiFi.localIP());
        }

        if (isConnected && !weatherData.valid) {
            lastWeatherAttempt = 0;
        }
    }

    // 3. Refresco de pronóstico del tiempo (cada 15 min o reintento)
    unsigned long weatherInterval = weatherData.valid ? WEATHER_REFRESH_INTERVAL_MS : WEATHER_RETRY_INTERVAL_MS;

    if (isConnected && (now - lastWeatherAttempt >= weatherInterval || !initialWeatherFetched)) {
        lastWeatherAttempt = now;

        Serial.println(F("[Loop] Actualizando datos de clima..."));
        if (weatherService.fetchWeather(settingsMgr.get().locationLat,
                                       settingsMgr.get().locationLon,
                                       weatherData,
                                       HOURLY_FORECAST_STEP)) {
            if (currentPage != 4 && !ui.isSleeping()) {
                ui.updateWeather(weatherData, currentPage);
            }
            initialWeatherFetched = true;

            DateTimeInfo dt;
            char statusMsg[60];
            if (getCurrentDateTime(dt)) {
                snprintf(statusMsg, sizeof(statusMsg), "IP:%s | RSSI:%ddBm | Act:%s",
                         WiFi.localIP().toString().c_str(),
                         WiFi.RSSI(),
                         dt.shortTimeStr);
            } else {
                snprintf(statusMsg, sizeof(statusMsg), "IP:%s | RSSI:%ddBm | Clima OK",
                         WiFi.localIP().toString().c_str(),
                         WiFi.RSSI());
            }
            if (!ui.isSleeping()) {
                ui.updateStatusBar(statusMsg);
            }
        } else {
            if (!ui.isSleeping()) {
                ui.updateStatusBar("Error al actualizar clima. Reintentando...");
            }
        }
    }

    // Permitir ejecución suave de tareas de fondo
    delay(5);
}
