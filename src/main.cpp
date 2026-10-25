#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "config.h"
#include "time_service.h"
#include "weather_service.h"
#include "display_ui.h"
#include "input_controller.h"
#include "buzzer_controller.h"

// Instancias principales
static DisplayUI ui(TFT_CS, TFT_DC, TFT_RST, TFT_LED_PIN);
static WeatherService weatherService;
static WeatherData weatherData;
static InputController inputCtrl(PIN_BTN_PREV, PIN_BTN_NEXT, PIN_BTN_ACTION, TIMER_LONG_PRESS_MS);
static BuzzerController buzzer(PIN_BUZZER);

static uint8_t currentPage = DISPLAY_PAGE_MODE;

// Estado del Temporizador (Página 3)
static TimerDisplayState timerState = TIMER_UI_IDLE;
static uint16_t timerSelectedMinutes = TIMER_DEFAULT_MINUTES;
static uint16_t timerTotalSec = TIMER_DEFAULT_MINUTES * 60;
static uint16_t timerRemainingSec = TIMER_DEFAULT_MINUTES * 60;
static unsigned long lastTimerTick = 0;

// Modo noche y reposo
static bool manualWakeActive = false;
static unsigned long manualWakeTime = 0;

// Temporizadores periódicos
static unsigned long lastClockTick = 0;
static unsigned long lastWeatherAttempt = 0;
static unsigned long lastWifiStatusCheck = 0;
static bool initialWeatherFetched = false;

void switchPage(uint8_t newPage, bool fullRedraw = false) {
    uint8_t prevPage = currentPage;
    currentPage = newPage;
    bool forceFull = fullRedraw || (newPage == 3) || (prevPage == 3);
    ui.drawBaseLayout(LOCATION_NAME, currentPage, forceFull);

    DateTimeInfo dt;
    if (getCurrentDateTime(dt)) {
        ui.updateDate(dt);
        if (currentPage != 4) {
            ui.updateTime(dt);
        }
    }

    if (currentPage == 4) {
        ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
    } else {
        if (weatherData.valid) {
            ui.updateWeather(weatherData, currentPage);
        }
    }

    if (forceFull) {
        ui.updateWifiStatus(WiFi.status() == WL_CONNECTED, WiFi.RSSI(), WiFi.localIP());
    }
}

void handleNightMode(const DateTimeInfo& dt) {
    if (!NIGHT_MODE_ENABLED) return;

    // Si el timer está corriendo o la alarma suena, no apagar
    if (timerState == TIMER_UI_RUNNING || timerState == TIMER_UI_ALARM) return;

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
                    ui.sleepDisplay();
                }
            } else {
                ui.sleepDisplay();
            }
        }
    } else {
        manualWakeActive = false;
        if (ui.isSleeping()) {
            ui.wakeDisplay();
            switchPage(currentPage, true);
        }
    }
}

void connectWiFi() {
    Serial.println(F("\n========================================"));
    Serial.printf("Conectando a red Wi-Fi: %s\n", WIFI_SSID);
    Serial.println(F("========================================"));

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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
    } else {
        Serial.println(F("\n[Wi-Fi] Tiempo de espera agotado. Continuando en segundo plano..."));
        ui.updateBootProgress("Sin conexion directa", 0);
    }
}

void setup() {
    Serial.begin(115200);
    delay(100);

    Serial.println(F("\n=============================================="));
    Serial.println(F("   ESP8266 Weather Clock - TFT ILI9341 SPI   "));
    Serial.println(F("=============================================="));

    // 1. Inicializar pantalla TFT (debe ser primero para que SPI.begin no altere la configuración de pines)
    ui.begin(TFT_ROTATION);
    ui.showBootScreen(WIFI_SSID);

    // 2. Inicializar botones y buzzer DESPUÉS del bus SPI de la pantalla
    inputCtrl.begin();
    buzzer.begin();

    // 3. Conectar a red Wi-Fi
    connectWiFi();

    // 4. Inicializar servicio de tiempo (NTP)
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

    // 5. Dibujar plantilla inicial (redibujado completo)
    switchPage(currentPage, true);

    // 6. Carga inicial del pronóstico meteorológico si hay Wi-Fi
    if (WiFi.status() == WL_CONNECTED) {
        ui.updateStatusBar("Descargando pronostico Open-Meteo...");
        if (weatherService.fetchWeather(LOCATION_LAT, LOCATION_LON, weatherData, HOURLY_FORECAST_STEP)) {
            if (currentPage != 3) {
                ui.updateWeather(weatherData, currentPage);
            }
            initialWeatherFetched = true;
            ui.updateStatusBar("Pronostico cargado OK");
        }
    }
}

void loop() {
    unsigned long now = millis();
    bool isConnected = (WiFi.status() == WL_CONNECTED);

    // Actualizar máquina de estados de buzzer
    buzzer.update();

    // ----------------------------------------------------
    // GESTIÓN DE BOTONES
    // ----------------------------------------------------
    ButtonEvent evPrev = inputCtrl.updatePrev();
    ButtonEvent evNext = inputCtrl.updateNext();
    ButtonEvent evAction = inputCtrl.updateAction();

    bool anyButtonActivity = (evPrev != BTN_EVENT_NONE || evNext != BTN_EVENT_NONE || evAction != BTN_EVENT_NONE);

    // Si la pantalla estaba apagada y se pulsa cualquier botón, despertar
    if (anyButtonActivity && ui.isSleeping()) {
        ui.wakeDisplay();
        switchPage(currentPage, true);
        manualWakeActive = true;
        manualWakeTime = now;
        if (BUZZER_KEY_BEEP) buzzer.beep(30);
        return;
    }

    // Si la alarma del buzzer está sonando, cualquier botón la silencia
    if (anyButtonActivity && buzzer.isAlarming()) {
        buzzer.stop();
        timerState = TIMER_UI_IDLE;
        timerRemainingSec = timerSelectedMinutes * 60;
        timerTotalSec = timerRemainingSec;
        if (currentPage == 4) {
            ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
        }
        return;
    }

    // Botón Siguiente (Avanzar página 1 -> 2 -> 3 -> 4 -> 1)
    if (evNext == BTN_EVENT_SHORT_CLICK) {
        Serial.println(F("[BTN] Siguiente (Next)"));
        if (BUZZER_KEY_BEEP) buzzer.beep(25);
        uint8_t nextPage = (currentPage >= 4) ? 1 : (currentPage + 1);
        switchPage(nextPage, false);
    }

    // Botón Anterior (Retroceder página 1 -> 4 -> 3 -> 2 -> 1)
    if (evPrev == BTN_EVENT_SHORT_CLICK) {
        Serial.println(F("[BTN] Anterior (Prev)"));
        if (BUZZER_KEY_BEEP) buzzer.beep(25);
        uint8_t prevPage = (currentPage <= 1) ? 4 : (currentPage - 1);
        switchPage(prevPage, false);
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
                ui.sleepDisplay();
            }
        } else if (currentPage == 4) {
            // En página 4: Control del temporizador
            if (timerState == TIMER_UI_IDLE) {
                if (evAction == BTN_EVENT_SHORT_CLICK) {
                    // Incrementar de 5 en 5 minutos
                    timerSelectedMinutes += TIMER_STEP_MINUTES;
                    if (timerSelectedMinutes > TIMER_MAX_MINUTES) {
                        timerSelectedMinutes = TIMER_MIN_MINUTES;
                    }
                    timerTotalSec = timerSelectedMinutes * 60;
                    timerRemainingSec = timerTotalSec;
                    if (BUZZER_KEY_BEEP) buzzer.beep(30);
                    ui.updateTimer(timerRemainingSec, timerTotalSec, timerState);
                } else if (evAction == BTN_EVENT_LONG_PRESS) {
                    // Confirmar e iniciar conteo regresivo
                    timerState = TIMER_UI_RUNNING;
                    lastTimerTick = now;
                    if (BUZZER_KEY_BEEP) buzzer.beep(120);
                    ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
                }
            } else if (timerState == TIMER_UI_RUNNING) {
                if (evAction == BTN_EVENT_SHORT_CLICK) {
                    // Detener / cancelar conteo regresivo
                    timerState = TIMER_UI_IDLE;
                    timerRemainingSec = timerSelectedMinutes * 60;
                    timerTotalSec = timerRemainingSec;
                    if (BUZZER_KEY_BEEP) buzzer.beep(60);
                    ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
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

                // Despertar pantalla si estaba en sleep
                if (ui.isSleeping()) {
                    ui.wakeDisplay();
                }

                // Si no estaba en página 4, cambiar a página 4 para mostrar alarma
                if (currentPage != 4) {
                    switchPage(4, true);
                } else {
                    ui.updateTimer(0, timerTotalSec, timerState, true);
                }
            }
        }
    } else if (timerState == TIMER_UI_ALARM) {
        // Si el buzzer dejó de sonar por timeout (30 seg)
        if (!buzzer.isAlarming()) {
            timerState = TIMER_UI_IDLE;
            timerRemainingSec = timerSelectedMinutes * 60;
            timerTotalSec = timerRemainingSec;
            if (currentPage == 4) {
                ui.updateTimer(timerRemainingSec, timerTotalSec, timerState, true);
            }
        }
    }

    // ----------------------------------------------------
    // TAREAS PERIÓDICAS (Reloj, Modo Noche, Wi-Fi, Clima)
    // ----------------------------------------------------
    // 1. Refresco de reloj y comprobación de modo noche (cada 1 segundo)
    if (now - lastClockTick >= CLOCK_REFRESH_INTERVAL_MS) {
        lastClockTick = now;

        DateTimeInfo dt;
        if (getCurrentDateTime(dt)) {
            if (!ui.isSleeping()) {
                if (currentPage != 4) {
                    ui.updateTime(dt);
                }
                ui.updateDate(dt);
            }
            handleNightMode(dt);
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
        if (weatherService.fetchWeather(LOCATION_LAT, LOCATION_LON, weatherData, HOURLY_FORECAST_STEP)) {
            if (currentPage != 3 && !ui.isSleeping()) {
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

    // Permitir ejecución suave de las tareas del ESP8266 (Wi-Fi, TCP, watchdog)
    delay(5);
}
