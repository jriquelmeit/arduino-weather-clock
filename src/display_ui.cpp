#include "display_ui.h"

#define COLOR_BG_HEADER       0x0926
#define COLOR_BORDER_CARD     0x2145
#define COLOR_TEXT_MUTED      0x8410

DisplayUI::DisplayUI(uint8_t cs, uint8_t dc, uint8_t rst, int8_t ledPin)
    : _tft(cs, dc, rst),
      _ledPin(ledPin),
      _lastTimerState((TimerDisplayState)-1),
      _lastProgressBarWidth(-1),
      _lastSecond(-1),
      _lastRssi(-999),
      _lastWifiState(false),
      _isSleeping(false),
      _pageMode(1),
      _lastAlarmSelected(-1),
      _lastAlarmEditMode(false),
      _lastAlarmEditField(ALARM_FIELD_NONE),
      _lastAlarmTopIndex(255) {
    _lastTimeStr[0] = '\0';
    _lastDateStr[0] = '\0';
    _lastStatusBarMsg[0] = '\0';
    _lastDayName[0] = '\0';
    _lastTimerStr[0] = '\0';
}

void DisplayUI::begin(uint8_t rotation) {
    if (_ledPin >= 0) {
        pinMode(_ledPin, OUTPUT);
        digitalWrite(_ledPin, HIGH); // Encender backlight
    }
    _tft.begin(40000000); // 40 MHz SPI para máxima fluidez
    _tft.setRotation(rotation);
    _tft.fillScreen(ILI9341_BLACK);
}

void DisplayUI::showBootScreen(const char* ssid) {
    _tft.fillScreen(ILI9341_BLACK);

    // Cabecera inicial
    _tft.fillRect(0, 0, 320, 36, COLOR_BG_HEADER);
    _tft.drawFastHLine(0, 36, 320, ILI9341_CYAN);

    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_WHITE);
    _tft.setCursor(14, 10);
    _tft.print("Reloj y Clima ESP8266");

    // Recuadro central de conexión
    _tft.drawRoundRect(15, 55, 290, 155, 8, ILI9341_DARKGREY);

    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_YELLOW);
    _tft.setCursor(30, 72);
    _tft.print("Iniciando Sistema");

    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_LIGHTGREY);
    _tft.setCursor(30, 110);
    _tft.print("Conectando a Wi-Fi:");

    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_CYAN);
    _tft.setCursor(30, 126);
    _tft.print(ssid);

    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_GREENYELLOW);
    _tft.setCursor(30, 160);
    _tft.print("Obteniendo hora NTP y clima...");
}

void DisplayUI::showConfigPortalScreen(const char* apSsid, const char* apIp) {
    _tft.fillScreen(ILI9341_BLACK);

    // Cabecera destacada (Azul marino con línea naranja)
    _tft.fillRect(0, 0, 320, 36, ILI9341_NAVY);
    _tft.drawFastHLine(0, 36, 320, ILI9341_ORANGE);

    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_YELLOW);
    _tft.setCursor(20, 10);
    _tft.print("MODO CONFIGURACION");

    // Marco central
    _tft.drawRoundRect(10, 48, 300, 178, 8, ILI9341_ORANGE);

    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_WHITE);
    _tft.setCursor(22, 60);
    _tft.print("1. Conectate a la red Wi-Fi:");

    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_CYAN);
    _tft.setCursor(26, 76);
    _tft.print(apSsid);

    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_WHITE);
    _tft.setCursor(22, 104);
    _tft.print("2. Abre tu navegador e ingresa a:");

    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_GREENYELLOW);
    _tft.setCursor(26, 120);
    _tft.print("http://");
    _tft.print(apIp);

    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_LIGHTGREY);
    _tft.setCursor(22, 150);
    _tft.print("Configura Wi-Fi, Ciudad y Melodia.");
    _tft.setCursor(22, 165);
    _tft.print("Al guardar, el reloj se reiniciara.");

    _tft.setTextColor(ILI9341_ORANGE);
    _tft.setCursor(22, 195);
    _tft.print("Esperando conexion del usuario...");
}

void DisplayUI::updateBootProgress(const char* stepMsg, uint8_t dots) {
    _tft.fillRect(30, 180, 260, 20, ILI9341_BLACK);
    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_ORANGE);
    _tft.setCursor(30, 185);
    _tft.print(stepMsg);

    for (uint8_t i = 0; i < (dots % 5); i++) {
        _tft.print(".");
    }
}

void DisplayUI::drawBaseLayout(const char* locationName, uint8_t pageMode, bool fullRedraw) {
    _pageMode = pageMode;

    if (fullRedraw) {
        _tft.fillScreen(ILI9341_BLACK);

        if (pageMode == 3) {
            // Página 3: Barra de Cabecera dedicada a la fecha y santoral (Y: 0..24)
            _tft.fillRect(0, 0, 320, 24, COLOR_BG_HEADER);
            _tft.drawFastHLine(0, 24, 320, ILI9341_CYAN);
        } else {
            // Páginas 1, 2 y 4: Barra de Cabecera estándar con ubicación (Y: 0..26)
            _tft.fillRect(0, 0, 320, 26, COLOR_BG_HEADER);
            _tft.drawFastHLine(0, 26, 320, ILI9341_CYAN);

            _tft.setTextSize(2);
            _tft.setTextColor(ILI9341_CYAN, COLOR_BG_HEADER);
            _tft.setCursor(8, 6);
            _tft.print(locationName);
        }

        // Línea divisoria barra inferior
        _tft.drawFastHLine(0, 224, 320, ILI9341_DARKGREY);

        _lastDateStr[0] = '\0';
        _lastRssi = -999;
        _lastWifiState = false;

        // Redibujar el footer inmediatamente tras limpiar la pantalla
        if (_lastStatusBarMsg[0] != '\0') {
            _tft.fillRect(0, 226, 320, 14, ILI9341_BLACK);
            _tft.setTextSize(1);
            _tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
            _tft.setCursor(6, 228);
            _tft.print(_lastStatusBarMsg);
        }
    } else {
        // Redibujado rápido: solo borrar la zona central de contenido (Y: 27 a 223)
        _tft.fillRect(0, 27, 320, 197, ILI9341_BLACK);
    }

    if (pageMode == 5) {
        // Página 5: Tarjeta grande para Despertador / Alarmas Diarias
        _tft.drawRoundRect(4, 30, 312, 190, 8, ILI9341_DARKCYAN);
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(14, 36);
        _tft.print("DESPERTADOR / ALARMAS DIARIAS");
        _lastAlarmSelected = -1;
        _lastAlarmTopIndex = 255;
    } else if (pageMode == 4) {
        // Página 4: Tarjeta grande central para Temporizador
        _tft.drawRoundRect(4, 30, 312, 190, 8, ILI9341_DARKCYAN);
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(14, 38);
        _tft.print("TEMPORIZADOR");
    } else if (pageMode == 3) {
        // Página 3: Layout Dashboard Central (3 Columnas)
        // 1. Tarjeta Izquierda: Estado Actual (X: 3..67, Y: 26..222)
        _tft.drawRoundRect(3, 26, 64, 196, 5, COLOR_BORDER_CARD);
        _tft.drawFastHLine(6, 42, 58, COLOR_BORDER_CARD);
        _tft.drawFastHLine(6, 90, 58, COLOR_BORDER_CARD);
        _tft.drawFastHLine(6, 130, 58, COLOR_BORDER_CARD);
        _tft.drawFastHLine(6, 170, 58, COLOR_BORDER_CARD);

        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(16, 31);
        _tft.print("ACTUAL");

        _tft.setTextColor(COLOR_TEXT_MUTED, ILI9341_BLACK);
        _tft.setCursor(8, 94);
        _tft.print("TEMP");

        _tft.setCursor(8, 134);
        _tft.print("HUMEDAD");

        _tft.setCursor(6, 174);
        _tft.print("INDICE UV");

        // 2. Tarjeta Central: Reloj Principal (X: 70..250, Y: 26..222, Ancho: 180)
        _tft.drawRoundRect(70, 26, 180, 196, 5, COLOR_BORDER_CARD);
        _tft.drawFastHLine(76, 68, 168, COLOR_BORDER_CARD);  // Divisor superior
        _tft.drawFastHLine(76, 160, 168, COLOR_BORDER_CARD); // Divisor inferior

        // Marco barra de segundos (X: 86, Y: 168, Ancho: 148, Alto: 6)
        _tft.drawRect(86, 168, 148, 6, COLOR_BORDER_CARD);

        // 3. Tarjeta Derecha: Pronóstico Hoy (X: 253..317, Y: 26..222)
        _tft.drawRoundRect(253, 26, 64, 196, 5, COLOR_BORDER_CARD);
        _tft.drawFastHLine(256, 42, 58, COLOR_BORDER_CARD);
        _tft.drawFastHLine(256, 82, 58, COLOR_BORDER_CARD);
        _tft.drawFastHLine(256, 144, 58, COLOR_BORDER_CARD);

        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(276, 31);
        _tft.print("HOY");

        _tft.setTextColor(COLOR_TEXT_MUTED, ILI9341_BLACK);
        _tft.setCursor(257, 86);
        _tft.print("MAX");

        _tft.setCursor(257, 116);
        _tft.print("MIN");
    } else {
        // Páginas 1 y 2: Tarjeta Reloj Digital (Izquierda superior)
        _tft.drawRoundRect(4, 30, 184, 102, 6, ILI9341_DARKGREY);
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
        _tft.setCursor(12, 36);
        _tft.print("HORA LOCAL");

        // Marco barra de segundos
        _tft.drawRect(12, 106, 168, 6, COLOR_BORDER_CARD);

        // Indicador estático de sincronización (se dibuja una sola vez)
        _tft.setCursor(12, 118);
        _tft.setTextColor(COLOR_TEXT_MUTED, ILI9341_BLACK);
        _tft.print("NTP Sincronizado");

        // 3. Tarjeta Clima / Pronóstico Hoy (Derecha superior)
        _tft.drawRoundRect(192, 30, 124, 102, 6, 0x2A6D);
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(198, 36);
        _tft.print("PRONOSTICO HOY");

        // 4. Tarjetas inferiores según la página seleccionada
        if (pageMode == 2) {
            // Página 2: 4 tarjetas para pronóstico por horas
            for (int i = 0; i < 4; i++) {
                int cardX = 4 + (i * 79); // 4, 83, 162, 241
                _tft.drawRoundRect(cardX, 136, 74, 86, 6, COLOR_BORDER_CARD);
            }
        } else {
            // Página 1 (por defecto): 2 tarjetas para pronóstico diario
            _tft.drawRoundRect(4, 136, 154, 86, 6, COLOR_BORDER_CARD);
            _tft.drawRoundRect(162, 136, 154, 86, 6, COLOR_BORDER_CARD);
        }
    }

    _lastTimeStr[0] = '\0';
    _lastDayName[0] = '\0';
    _lastTimerStr[0] = '\0';
    _lastTimerState = (TimerDisplayState)-1;
    _lastProgressBarWidth = -1;
    _lastSecond = -1;
}

void DisplayUI::updateTime(const DateTimeInfo& timeInfo) {
    if (_isSleeping || _pageMode == 4) return;

    if (_pageMode == 3) {
        // --- Modo Página 3: Reloj Gigante Central ---
        // 1. Día de la semana (Centrado arriba del reloj gigante, Y: 32..64)
        if (strncmp(_lastDayName, timeInfo.dayName, sizeof(_lastDayName)) != 0) {
            _tft.fillRect(72, 32, 176, 32, ILI9341_BLACK);
            _tft.setTextSize(2);
            _tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
            int len = strlen(timeInfo.dayName);
            int x = 70 + (180 - (len * 12)) / 2;
            _tft.setCursor(x > 72 ? x : 72, 42);
            _tft.print(timeInfo.dayName);
            strncpy(_lastDayName, timeInfo.dayName, sizeof(_lastDayName));
        }

        // 2. Reloj Gigante en formato HH:MM (Tamaño 6x horiz, 7x vert, centrado Y: 94)
        if (strncmp(_lastTimeStr, timeInfo.shortTimeStr, sizeof(_lastTimeStr)) != 0) {
            _tft.fillRect(72, 90, 176, 62, ILI9341_BLACK);
            _tft.setTextSize(6, 7);
            _tft.setTextColor(ILI9341_GREENYELLOW, ILI9341_BLACK);
            _tft.setCursor(73, 94);
            _tft.print(timeInfo.shortTimeStr);
            strncpy(_lastTimeStr, timeInfo.shortTimeStr, sizeof(_lastTimeStr));
        }

        // 3. Barra de segundos fluida (X: 88, Y: 170, Ancho: 144)
        if (timeInfo.second != _lastSecond) {
            int barW = 144;
            int fillW = (timeInfo.second * barW) / 59;
            if (timeInfo.second == 0) {
                _tft.fillRect(88, 170, barW, 2, ILI9341_BLACK);
            }
            if (fillW > 0) {
                _tft.fillRect(88, 170, fillW, 2, ILI9341_CYAN);
            }
            _lastSecond = timeInfo.second;
        }
    } else {
        // --- Modo Páginas 1 y 2: Reloj en Tarjeta Superior Izquierda ---
        // 1. Reloj en formato HH:MM (solo se redibuja al cambiar el minuto)
        if (strncmp(_lastTimeStr, timeInfo.shortTimeStr, sizeof(_lastTimeStr)) != 0) {
            _tft.fillRect(10, 48, 172, 32, ILI9341_BLACK);
            _tft.setTextSize(4);
            _tft.setTextColor(ILI9341_GREENYELLOW, ILI9341_BLACK);
            _tft.setCursor(24, 50);
            _tft.print(timeInfo.shortTimeStr);
            strncpy(_lastTimeStr, timeInfo.shortTimeStr, sizeof(_lastTimeStr));
        }

        // 2. Subtítulo: Día de la semana
        if (strncmp(_lastDayName, timeInfo.dayName, sizeof(_lastDayName)) != 0) {
            _tft.fillRect(12, 86, 168, 18, ILI9341_BLACK);
            _tft.setTextSize(2);
            _tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
            _tft.setCursor(12, 88);
            _tft.print(timeInfo.dayName);
            strncpy(_lastDayName, timeInfo.dayName, sizeof(_lastDayName));
        }

        // 3. Barra de segundos fluida (X: 14, Y: 108, Ancho interior: 164)
        if (timeInfo.second != _lastSecond) {
            int barX = 14;
            int barY = 108;
            int barW = 164;
            int fillW = (timeInfo.second * barW) / 59;
            if (timeInfo.second == 0) {
                _tft.fillRect(barX, barY, barW, 2, ILI9341_BLACK);
            }
            if (fillW > 0) {
                _tft.fillRect(barX, barY, fillW, 2, ILI9341_CYAN);
            }
            _lastSecond = timeInfo.second;
        }
    }
}

void DisplayUI::updateDate(const DateTimeInfo& timeInfo) {
    if (_isSleeping) return;

    if (_pageMode == 3) {
        // En Página 3 se muestra la fecha completa con el Santoral en toda la cabecera
        if (strncmp(_lastDateStr, timeInfo.dateStr, sizeof(_lastDateStr)) != 0) {
            _tft.fillRect(6, 4, 290, 16, COLOR_BG_HEADER);
            _tft.setTextSize(1);
            _tft.setTextColor(ILI9341_WHITE, COLOR_BG_HEADER);
            _tft.setCursor(8, 8);
            _tft.print(timeInfo.dateStr);
            strncpy(_lastDateStr, timeInfo.dateStr, sizeof(_lastDateStr));
        }
    } else {
        // En Páginas 1, 2 y 4 se muestra a la derecha de la ciudad
        if (strncmp(_lastDateStr, timeInfo.shortDateStr, sizeof(_lastDateStr)) != 0) {
            _tft.fillRect(110, 8, 175, 12, COLOR_BG_HEADER);
            _tft.setTextSize(1);
            _tft.setTextColor(ILI9341_WHITE, COLOR_BG_HEADER);
            _tft.setCursor(110, 9);
            _tft.print(timeInfo.shortDateStr);
            strncpy(_lastDateStr, timeInfo.shortDateStr, sizeof(_lastDateStr));
        }
    }
}

void DisplayUI::updateWeather(const WeatherData& weather, uint8_t pageMode) {
    if (!weather.valid || _isSleeping || pageMode == 4) return;

    if (pageMode == 3) {
        drawPage3Weather(weather);
        return;
    }

    // --- Panel Pronóstico Hoy (Superior Derecho) para Páginas 1 y 2 ---
    // Limpiar zona interior
    _tft.fillRect(195, 48, 118, 80, ILI9341_BLACK);

    // 1. Icono del tiempo (tamaño regular)
    drawWeatherIcon(214, 66, weather.current.iconType, false);

    // 2. Temperatura actual en Tamaño 2
    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
    _tft.setCursor(240, 58);
    _tft.printf("%.1f C", weather.current.temperature);

    // 3. Extremos de Hoy: Max y Min en Tamaño 1
    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
    _tft.setCursor(198, 86);
    _tft.printf("Max %.1f", weather.today.tempMax);

    _tft.setTextColor(COLOR_TEXT_MUTED, ILI9341_BLACK);
    _tft.print(" | ");

    _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    _tft.printf("Min %.1f", weather.today.tempMin);

    // 4. Lluvia de Hoy: Gota + Probabilidad (%) + Milímetros (mm)
    drawWaterDrop(204, 104, ILI9341_CYAN);

    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    _tft.setCursor(214, 104);
    _tft.printf("%d%%", weather.today.rainProbability);

    _tft.setTextColor(weather.today.precipitation > 0.05f ? ILI9341_CYAN : ILI9341_LIGHTGREY, ILI9341_BLACK);
    _tft.setCursor(258, 104);
    _tft.printf("%.1f mm", weather.today.precipitation);

    // --- Pronósticos según la página ---
    if (pageMode == 2) {
        drawHourlyForecast(weather);
    } else {
        drawDailyForecast(weather);
    }
}

void DisplayUI::drawPage3Weather(const WeatherData& weather) {
    // ==========================================
    // 1. COSTADO IZQUIERDO: ESTADO ACTUAL
    // ==========================================
    // Limpiar icono y condición
    _tft.fillRect(5, 44, 60, 44, ILI9341_BLACK);
    drawWeatherIcon(35, 58, weather.current.iconType, false);

    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    char shortCond[11];
    if (strlen(weather.current.conditionText) > 9) {
        strncpy(shortCond, weather.current.conditionText, 9);
        shortCond[9] = '\0';
    } else {
        strncpy(shortCond, weather.current.conditionText, sizeof(shortCond));
    }
    int condX = 3 + (64 - (strlen(shortCond) * 6)) / 2;
    _tft.setCursor(condX > 5 ? condX : 5, 78);
    _tft.print(shortCond);

    // Temperatura actual (formato "15.5°")
    _tft.fillRect(5, 105, 60, 22, ILI9341_BLACK);
    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
    _tft.setCursor(8, 107);
    _tft.printf("%.1f", weather.current.temperature);
    _tft.drawCircle(58, 107, 2, ILI9341_ORANGE);

    // Humedad actual
    _tft.fillRect(5, 144, 60, 22, ILI9341_BLACK);
    drawWaterDrop(8, 151, ILI9341_CYAN);
    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    _tft.setCursor(18, 147);
    _tft.printf("%d%%", weather.current.humidity);

    // Índice UV actual
    _tft.fillRect(5, 184, 60, 34, ILI9341_BLACK);
    uint16_t uvCol = getUvColor(weather.current.uvIndex);
    _tft.setTextSize(2);
    _tft.setTextColor(uvCol, ILI9341_BLACK);
    _tft.setCursor(8, 185);
    _tft.printf("%.1f", weather.current.uvIndex);

    _tft.setTextSize(1);
    _tft.setTextColor(uvCol, ILI9341_BLACK);
    _tft.setCursor(8, 204);
    _tft.print(getUvCategory(weather.current.uvIndex));

    // ==========================================
    // 2. PANEL CENTRAL: DETALLE INFERIOR
    // ==========================================
    _tft.fillRect(72, 180, 176, 38, ILI9341_BLACK);
    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    int fullCondLen = strlen(weather.current.conditionText);
    int fullCondX = 70 + (180 - (fullCondLen * 6)) / 2;
    _tft.setCursor(fullCondX > 72 ? fullCondX : 72, 184);
    _tft.print(weather.current.conditionText);

    _tft.setTextColor(COLOR_TEXT_MUTED, ILI9341_BLACK);
    const char* ntpMsg = "Hora sincronizada NTP";
    int ntpX = 70 + (180 - (strlen(ntpMsg) * 6)) / 2;
    _tft.setCursor(ntpX, 200);
    _tft.print(ntpMsg);

    // ==========================================
    // 3. COSTADO DERECHO: PRONÓSTICO HOY
    // ==========================================
    // Limpiar icono de hoy
    _tft.fillRect(255, 44, 60, 36, ILI9341_BLACK);
    drawWeatherIcon(285, 58, weather.today.iconType, false);

    // Temp Máxima
    _tft.fillRect(255, 96, 60, 18, ILI9341_BLACK);
    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
    _tft.setCursor(257, 97);
    _tft.printf("%.1f", weather.today.tempMax);
    _tft.drawCircle(308, 97, 2, ILI9341_ORANGE);

    // Temp Mínima
    _tft.fillRect(255, 125, 60, 18, ILI9341_BLACK);
    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    _tft.setCursor(257, 126);
    _tft.printf("%.1f", weather.today.tempMin);
    _tft.drawCircle(308, 126, 2, ILI9341_CYAN);

    // Condicional: Lluvia vs Índice UV Máximo de Hoy
    _tft.fillRect(255, 148, 60, 68, ILI9341_BLACK);
    if (weather.today.precipitation > 0.05f) {
        // Mostrar lluvia en mm y probabilidad
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(257, 150);
        _tft.print("LLUVIA");

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(257, 164);
        _tft.printf("%.1f", weather.today.precipitation);

        _tft.setTextSize(1);
        _tft.setCursor(296, 170);
        _tft.print("mm");

        _tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
        _tft.setCursor(257, 192);
        _tft.printf("%d%%", weather.today.rainProbability);
    } else {
        // Mostrar UV Máximo previsto para hoy
        uint16_t uvMaxCol = getUvColor(weather.today.uvIndexMax);
        _tft.setTextSize(1);
        _tft.setTextColor(COLOR_TEXT_MUTED, ILI9341_BLACK);
        _tft.setCursor(257, 150);
        _tft.print("UV MAX");

        _tft.setTextSize(2);
        _tft.setTextColor(uvMaxCol, ILI9341_BLACK);
        _tft.setCursor(257, 164);
        _tft.printf("%.1f", weather.today.uvIndexMax);

        _tft.setTextSize(1);
        _tft.setTextColor(uvMaxCol, ILI9341_BLACK);
        _tft.setCursor(257, 192);
        _tft.print(getUvCategory(weather.today.uvIndexMax));
    }
}

void DisplayUI::drawDailyForecast(const WeatherData& weather) {
    // --- Tarjetas de Pronóstico a 2 Días ---
    for (int i = 0; i < 2; i++) {
        int cardX = (i == 0) ? 4 : 162;
        int cardW = 154;

        // Limpiar interior de la tarjeta
        _tft.fillRect(cardX + 2, 138, cardW - 4, 82, ILI9341_BLACK);

        // 1. Título del día en Tamaño 2
        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(cardX + 10, 140);
        _tft.print(weather.forecast[i].dayName);

        // Línea divisoria interior
        _tft.drawFastHLine(cardX + 6, 158, cardW - 12, COLOR_BORDER_CARD);

        // 2. Icono meteorológico en el lateral izquierdo
        drawWeatherIcon(cardX + 22, 178, weather.forecast[i].iconType, true);

        // 3. Temperatura Máxima en Tamaño 2
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
        _tft.setCursor(cardX + 44, 168);
        _tft.print("Max");

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
        _tft.setCursor(cardX + 68, 164);
        _tft.printf("%.1f C", weather.forecast[i].tempMax);

        // 4. Temperatura Mínima en Tamaño 2
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(cardX + 44, 186);
        _tft.print("Min");

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(cardX + 68, 182);
        _tft.printf("%.1f C", weather.forecast[i].tempMin);

        // 5. Probabilidad y milímetros de lluvia (Reemplaza el texto de condición)
        drawWaterDrop(cardX + 16, 204, ILI9341_CYAN);

        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(cardX + 25, 204);
        _tft.printf("%d%%", weather.forecast[i].rainProbability);

        _tft.setTextColor(weather.forecast[i].precipitation > 0.05f ? ILI9341_CYAN : ILI9341_LIGHTGREY, ILI9341_BLACK);
        _tft.setCursor(cardX + 88, 204);
        _tft.printf("%.1f mm", weather.forecast[i].precipitation);
    }
}

void DisplayUI::drawHourlyForecast(const WeatherData& weather) {
    // --- 4 Tarjetas de Pronóstico por Horas ---
    for (int i = 0; i < 4; i++) {
        int cardX = 4 + (i * 79); // 4, 83, 162, 241
        int cardW = 74;

        // Limpiar interior de la tarjeta
        _tft.fillRect(cardX + 2, 138, cardW - 4, 82, ILI9341_BLACK);

        // 1. Hora en Tamaño 2 (centrada)
        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        int timeLen = strlen(weather.hourly[i].timeStr);
        int timeX = cardX + (cardW - (timeLen * 12)) / 2;
        _tft.setCursor(timeX > cardX + 2 ? timeX : cardX + 4, 140);
        _tft.print(weather.hourly[i].timeStr);

        // Línea divisoria interior
        _tft.drawFastHLine(cardX + 4, 156, cardW - 8, COLOR_BORDER_CARD);

        // 2. Icono meteorológico reducido centrado
        drawWeatherIcon(cardX + 37, 168, weather.hourly[i].iconType, true);

        // 3. Temperatura en Tamaño 1 centrada
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
        char tBuf[10];
        snprintf(tBuf, sizeof(tBuf), "%.1f C", weather.hourly[i].temperature);
        int tLen = strlen(tBuf);
        int tX = cardX + (cardW - (tLen * 6)) / 2;
        _tft.setCursor(tX > cardX + 2 ? tX : cardX + 2, 182);
        _tft.print(tBuf);

        // 4. Gota + porcentaje de lluvia centrado
        char pBuf[8];
        snprintf(pBuf, sizeof(pBuf), "%d%%", weather.hourly[i].rainProbability);
        int pLen = strlen(pBuf);
        int totalW = 5 + 3 + (pLen * 6); // Ancho gota (5) + espacio (3) + texto
        int dropX = cardX + (cardW - totalW) / 2 + 2;
        drawWaterDrop(dropX, 195, ILI9341_CYAN);

        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(dropX + 5, 195);
        _tft.print(pBuf);

        // 5. Milímetros de lluvia centrados
        char mmBuf[12];
        snprintf(mmBuf, sizeof(mmBuf), "%.1f mm", weather.hourly[i].precipitation);
        int mmLen = strlen(mmBuf);
        int mmX = cardX + (cardW - (mmLen * 6)) / 2;
        _tft.setTextColor(weather.hourly[i].precipitation > 0.05f ? ILI9341_CYAN : ILI9341_LIGHTGREY, ILI9341_BLACK);
        _tft.setCursor(mmX > cardX + 2 ? mmX : cardX + 2, 207);
        _tft.print(mmBuf);
    }
}

void DisplayUI::drawWaterDrop(int x, int y, uint16_t color) {
    // Mini-gota de agua vectorial (5x7 píxeles)
    _tft.drawPixel(x, y, color);
    _tft.drawFastHLine(x - 1, y + 1, 3, color);
    _tft.drawFastHLine(x - 2, y + 2, 5, color);
    _tft.fillRect(x - 2, y + 3, 5, 3, color);
    _tft.drawFastHLine(x - 1, y + 6, 3, color);
}

void DisplayUI::drawWifiIndicator(int x, int y, bool connected, int rssi) {
    _tft.fillRect(x, y, 16, 14, COLOR_BG_HEADER);

    if (!connected) {
        // Cruz roja indicando desconexión
        _tft.drawLine(x + 2, y + 2, x + 12, y + 12, ILI9341_RED);
        _tft.drawLine(x + 12, y + 2, x + 2, y + 12, ILI9341_RED);
        return;
    }

    // 4 barras de señal Wi-Fi
    uint16_t barColor = ILI9341_GREEN;
    if (rssi < -80) {
        barColor = ILI9341_RED;
    } else if (rssi < -70) {
        barColor = ILI9341_YELLOW;
    }

    int bars = 1;
    if (rssi >= -60) bars = 4;
    else if (rssi >= -70) bars = 3;
    else if (rssi >= -80) bars = 2;

    for (int b = 0; b < 4; b++) {
        int barH = (b + 1) * 3;
        uint16_t c = (b < bars) ? barColor : ILI9341_DARKGREY;
        _tft.fillRect(x + (b * 4), y + (12 - barH), 2, barH, c);
    }
}

void DisplayUI::updateWifiStatus(bool connected, int rssi, const IPAddress& ip) {
    if (_isSleeping) return;
    if (connected != _lastWifiState || abs(rssi - _lastRssi) > 4) {
        drawWifiIndicator(298, 7, connected, rssi);
        _lastWifiState = connected;
        _lastRssi = rssi;
    }
}

void DisplayUI::updateStatusBar(const char* message) {
    if (message != nullptr && message[0] != '\0') {
        strncpy(_lastStatusBarMsg, message, sizeof(_lastStatusBarMsg) - 1);
        _lastStatusBarMsg[sizeof(_lastStatusBarMsg) - 1] = '\0';
    }
    if (_isSleeping) return;
    _tft.fillRect(0, 226, 320, 14, ILI9341_BLACK);
    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
    _tft.setCursor(6, 228);
    _tft.print(_lastStatusBarMsg);
}

void DisplayUI::drawWeatherIcon(int x, int y, WeatherIconType type, bool small) {
    int r = small ? 7 : 10;

    switch (type) {
        case ICON_SUNNY:
            // Sol amarillo con rayos
            _tft.fillCircle(x, y, r, ILI9341_YELLOW);
            for (int i = 0; i < 8; i++) {
                float angle = i * (3.14159f / 4.0f);
                int x1 = x + (int)((r + 2) * cos(angle));
                int y1 = y + (int)((r + 2) * sin(angle));
                int x2 = x + (int)((r + 5) * cos(angle));
                int y2 = y + (int)((r + 5) * sin(angle));
                _tft.drawLine(x1, y1, x2, y2, ILI9341_ORANGE);
            }
            break;

        case ICON_CLEAR_NIGHT: {
            // Luna creciente amarilla
            _tft.fillCircle(x, y, r, ILI9341_YELLOW);
            int cutOffset = small ? 3 : 4;
            _tft.fillCircle(x + cutOffset, y - cutOffset, r - (small ? 1 : 1), ILI9341_BLACK);
            // Pequeñas estrellas decorativas
            _tft.drawPixel(x - (small ? 4 : 6), y - (small ? 3 : 5), ILI9341_WHITE);
            _tft.drawPixel(x - (small ? 2 : 4), y + (small ? 4 : 6), 0xFFE0);
            if (!small) {
                _tft.drawPixel(x + 5, y + 6, ILI9341_WHITE);
            }
            break;
        }

        case ICON_PARTLY_CLOUDY:
            // Sol atrás a la izquierda
            _tft.fillCircle(x - (small ? 4 : 6), y - (small ? 4 : 6), small ? 5 : 7, ILI9341_YELLOW);
            // Nube enfrente
            _tft.fillCircle(x, y + 2, small ? 5 : 7, ILI9341_WHITE);
            _tft.fillCircle(x + (small ? 6 : 9), y + 3, small ? 4 : 6, ILI9341_LIGHTGREY);
            _tft.fillRect(x - (small ? 4 : 6), y + (small ? 3 : 4), small ? 14 : 20, small ? 5 : 7, ILI9341_WHITE);
            break;

        case ICON_PARTLY_CLOUDY_NIGHT: {
            // Luna creciente atrás a la izquierda
            int lx = x - (small ? 4 : 6);
            int ly = y - (small ? 4 : 6);
            int lr = small ? 5 : 7;
            _tft.fillCircle(lx, ly, lr, ILI9341_YELLOW);
            _tft.fillCircle(lx + (small ? 2 : 3), ly - (small ? 2 : 3), lr - (small ? 1 : 1), ILI9341_BLACK);

            // Nube enfrente
            _tft.fillCircle(x, y + 2, small ? 5 : 7, ILI9341_WHITE);
            _tft.fillCircle(x + (small ? 6 : 9), y + 3, small ? 4 : 6, ILI9341_LIGHTGREY);
            _tft.fillRect(x - (small ? 4 : 6), y + (small ? 3 : 4), small ? 14 : 20, small ? 5 : 7, ILI9341_WHITE);
            break;
        }

        case ICON_CLOUDY:
            // Nube esponjosa
            _tft.fillCircle(x - (small ? 5 : 7), y + 1, small ? 5 : 7, ILI9341_LIGHTGREY);
            _tft.fillCircle(x + 1, y - (small ? 3 : 4), small ? 6 : 9, ILI9341_WHITE);
            _tft.fillCircle(x + (small ? 6 : 9), y + 2, small ? 5 : 7, ILI9341_LIGHTGREY);
            _tft.fillRect(x - (small ? 5 : 7), y + (small ? 2 : 3), small ? 15 : 21, small ? 5 : 7, ILI9341_WHITE);
            break;

        case ICON_RAIN:
            // Nube gris
            _tft.fillCircle(x - (small ? 5 : 6), y - 2, small ? 4 : 6, ILI9341_DARKGREY);
            _tft.fillCircle(x + 1, y - (small ? 4 : 6), small ? 5 : 7, ILI9341_LIGHTGREY);
            _tft.fillCircle(x + (small ? 5 : 6), y - 1, small ? 4 : 5, ILI9341_DARKGREY);
            _tft.fillRect(x - (small ? 4 : 5), y - 1, small ? 12 : 16, small ? 4 : 5, ILI9341_LIGHTGREY);
            // Gotas de lluvia
            _tft.drawLine(x - 5, y + 5, x - 7, y + (small ? 8 : 11), ILI9341_CYAN);
            _tft.drawLine(x, y + 5, x - 2, y + (small ? 8 : 11), ILI9341_CYAN);
            _tft.drawLine(x + 5, y + 5, x + 3, y + (small ? 8 : 11), ILI9341_CYAN);
            break;

        case ICON_SNOW:
            // Nube y copos blancos
            _tft.fillCircle(x, y - 3, small ? 5 : 7, ILI9341_LIGHTGREY);
            _tft.fillCircle(x - 5, y - 1, small ? 4 : 5, 0xBDF7);
            _tft.fillCircle(x + 5, y - 1, small ? 4 : 5, 0xBDF7);
            // Copos (puntos blancos)
            _tft.drawPixel(x - 4, y + 5, ILI9341_WHITE);
            _tft.drawPixel(x, y + 7, ILI9341_WHITE);
            _tft.drawPixel(x + 4, y + 5, ILI9341_WHITE);
            break;

        case ICON_THUNDERSTORM:
            // Nube oscura
            _tft.fillCircle(x - 4, y - 4, small ? 4 : 6, ILI9341_DARKGREY);
            _tft.fillCircle(x + 2, y - 6, small ? 5 : 7, 0x4208);
            _tft.fillCircle(x + 6, y - 3, small ? 4 : 5, ILI9341_DARKGREY);
            // Rayo amarillo zig-zag
            _tft.drawLine(x - 1, y + 1, x - 4, y + 5, ILI9341_YELLOW);
            _tft.drawLine(x - 4, y + 5, x + 1, y + 5, ILI9341_YELLOW);
            _tft.drawLine(x + 1, y + 5, x - 2, y + 10, ILI9341_YELLOW);
            break;

        default:
            _tft.drawCircle(x, y, r, ILI9341_WHITE);
            _tft.drawPixel(x, y, ILI9341_WHITE);
            break;
    }
}

void DisplayUI::updateTimer(uint32_t remainingSec, uint32_t totalSec, TimerDisplayState state, bool forceRedraw) {
    if (_isSleeping) return;

    // 1. Formatear tiempo en HH:MM:SS (si >= 1 hora) o MM:SS (si < 1 hora)
    char timerStr[16];
    uint32_t hours = remainingSec / 3600;
    uint32_t mins = (remainingSec % 3600) / 60;
    uint32_t secs = remainingSec % 60;

    bool hasHours = (remainingSec >= 3600 || totalSec >= 3600);
    if (hasHours) {
        snprintf(timerStr, sizeof(timerStr), "%02u:%02u:%02u", hours, mins, secs);
    } else {
        snprintf(timerStr, sizeof(timerStr), "%02u:%02u", mins, secs);
    }

    // 2. Redibujar dígitos solo si cambiaron o forceRedraw
    if (forceRedraw || strncmp(_lastTimerStr, timerStr, sizeof(_lastTimerStr)) != 0 || state != _lastTimerState) {
        _tft.fillRect(20, 56, 280, 52, ILI9341_BLACK);

        if (hasHours) {
            _tft.setTextSize(5);
            _tft.setCursor(40, 62);
        } else {
            _tft.setTextSize(6);
            _tft.setCursor(70, 60);
        }

        if (state == TIMER_UI_RUNNING) {
            _tft.setTextColor(ILI9341_GREENYELLOW, ILI9341_BLACK);
        } else if (state == TIMER_UI_ALARM) {
            _tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
        } else {
            _tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
        }
        _tft.print(timerStr);
        strncpy(_lastTimerStr, timerStr, sizeof(_lastTimerStr));
    }

    // 3. Barra de progreso (X: 30, Y: 122, W: 260, H: 14)
    if (forceRedraw) {
        _tft.drawRoundRect(28, 120, 264, 18, 4, ILI9341_WHITE);
        _lastProgressBarWidth = -1;
    }

    int currentBarWidth = 0;
    if (totalSec > 0) {
        currentBarWidth = (remainingSec * 260) / totalSec;
        if (currentBarWidth > 260) currentBarWidth = 260;
    }

    if (forceRedraw || currentBarWidth != _lastProgressBarWidth) {
        uint16_t barColor = (state == TIMER_UI_ALARM) ? ILI9341_RED : ILI9341_CYAN;
        if (currentBarWidth > 0) {
            _tft.fillRect(30, 122, currentBarWidth, 14, barColor);
        }
        if (currentBarWidth < 260) {
            _tft.fillRect(30 + currentBarWidth, 122, 260 - currentBarWidth, 14, ILI9341_BLACK);
        }
        _lastProgressBarWidth = currentBarWidth;
    }

    // 4. Textos de estado y ayuda según máquina de estados
    if (forceRedraw || state != _lastTimerState) {
        _tft.fillRect(10, 150, 300, 68, ILI9341_BLACK);

        if (state == TIMER_UI_IDLE) {
            _tft.setTextSize(2);
            _tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
            _tft.setCursor(95, 156);
            _tft.print("MODO AJUSTE");

            _tft.setTextSize(1);
            _tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
            _tft.setCursor(45, 188);
            _tft.print("Btn 3: +5 min  |  Mantener: INICIAR");
        } else if (state == TIMER_UI_RUNNING) {
            _tft.setTextSize(2);
            _tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
            _tft.setCursor(105, 156);
            _tft.print("EN CONTEO");

            _tft.setTextSize(1);
            _tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
            _tft.setCursor(55, 188);
            _tft.print("Presione Btn 3 para DETENER");
        } else if (state == TIMER_UI_ALARM) {
            _tft.setTextSize(2);
            _tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
            _tft.setCursor(45, 156);
            _tft.print("!TIEMPO FINALIZADO!");

            _tft.setTextSize(1);
            _tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
            _tft.setCursor(55, 188);
            _tft.print("Presione Btn 3 para SILENCIAR");
        }
        _lastTimerState = state;
    }
}

void DisplayUI::updateAlarmUI(const DailyAlarm* alarms, uint8_t count, uint8_t selectedIndex,
                              bool editMode, AlarmEditField editField, bool forceRedraw) {
    if (!alarms || count == 0) return;
    if (selectedIndex >= count) selectedIndex = 0;

    uint8_t topIndex = 0;
    if (count <= 4) {
        topIndex = 0;
    } else if (selectedIndex >= count - 2) {
        topIndex = count - 4;
    } else if (selectedIndex >= 2) {
        topIndex = selectedIndex - 1;
    }

    _lastAlarmTopIndex = topIndex;
    _lastAlarmEditMode = editMode;
    _lastAlarmEditField = editField;
    _lastAlarmSelected = selectedIndex;

    char daysStr[24];
    char timeBuf[8];
    char idBuf[8];

    for (uint8_t r = 0; r < 4; r++) {
        uint8_t aIdx = topIndex + r;
        if (aIdx >= count) break;

        int rowY = 48 + (r * 30);
        bool isSelected = (aIdx == selectedIndex);

        // Fondo y recuadro de la fila
        if (isSelected) {
            uint16_t borderColor = editMode ? ILI9341_YELLOW : ILI9341_CYAN;
            uint16_t rowBg = editMode ? 0x18C3 : 0x0926;
            _tft.fillRoundRect(8, rowY, 304, 28, 4, rowBg);
            _tft.drawRoundRect(8, rowY, 304, 28, 4, borderColor);
        } else {
            _tft.fillRoundRect(8, rowY, 304, 28, 4, ILI9341_BLACK);
            _tft.drawRoundRect(8, rowY, 304, 28, 4, COLOR_BORDER_CARD);
        }

        const DailyAlarm& alarm = alarms[aIdx];

        // 1. Identificador de alarma: [1] a [10]
        snprintf(idBuf, sizeof(idBuf), "[%u]", alarm.id);
        _tft.setTextSize(1);
        _tft.setTextColor(isSelected ? ILI9341_YELLOW : ILI9341_DARKGREY);
        _tft.setCursor(14, rowY + 9);
        _tft.print(idBuf);

        // 2. Hora: HH:MM
        if (isSelected && editMode) {
            char hBuf[4], mBuf[4];
            snprintf(hBuf, sizeof(hBuf), "%02u", alarm.hour);
            snprintf(mBuf, sizeof(mBuf), "%02u", alarm.minute);

            _tft.setTextSize(2);
            _tft.setTextColor(editField == ALARM_FIELD_HOUR ? ILI9341_YELLOW : ILI9341_WHITE);
            _tft.setCursor(42, rowY + 6);
            _tft.print(hBuf);

            _tft.setTextColor(ILI9341_WHITE);
            _tft.print(":");

            _tft.setTextColor(editField == ALARM_FIELD_MINUTE ? ILI9341_YELLOW : ILI9341_WHITE);
            _tft.print(mBuf);
        } else {
            snprintf(timeBuf, sizeof(timeBuf), "%02u:%02u", alarm.hour, alarm.minute);
            _tft.setTextSize(2);
            uint16_t timeColor = alarm.enabled ? (isSelected ? ILI9341_WHITE : ILI9341_GREEN) : COLOR_TEXT_MUTED;
            _tft.setTextColor(timeColor);
            _tft.setCursor(42, rowY + 6);
            _tft.print(timeBuf);
        }

        // 3. Días
        formatAlarmDaysString(alarm.days, daysStr, sizeof(daysStr));
        _tft.setTextSize(1);
        uint16_t daysColor;
        if (isSelected && editMode && editField == ALARM_FIELD_DAYS) {
            daysColor = ILI9341_YELLOW;
        } else {
            daysColor = alarm.enabled ? ILI9341_CYAN : COLOR_TEXT_MUTED;
        }
        _tft.setTextColor(daysColor);
        _tft.setCursor(114, rowY + 10);
        _tft.print(daysStr);

        // 4. Estado [ ON ] / [ OFF ]
        _tft.setTextSize(1);
        _tft.setCursor(248, rowY + 10);
        if (isSelected && editMode && editField == ALARM_FIELD_ENABLE) {
            _tft.setTextColor(ILI9341_YELLOW);
            _tft.print(alarm.enabled ? ">[ON ]" : ">[OFF]");
        } else if (alarm.enabled) {
            _tft.setTextColor(ILI9341_GREEN);
            _tft.print("[ ON ]");
        } else {
            _tft.setTextColor(ILI9341_RED);
            _tft.print("[ OFF]");
        }
    }

    // Pie de página de instrucciones (Y: 172..218)
    _tft.drawFastHLine(8, 172, 304, COLOR_BORDER_CARD);
    _tft.fillRect(8, 174, 304, 44, ILI9341_BLACK);

    if (!editMode) {
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
        _tft.setCursor(12, 176);
        _tft.print("Btn 1/2: Pagina  |  Btn 3: Elegir alarma");

        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(12, 190);
        _tft.print("Mantener Btn 3: Editar alarma seleccionada");

        _tft.setTextColor(COLOR_TEXT_MUTED, ILI9341_BLACK);
        _tft.setCursor(12, 204);
        char posBuf[32];
        snprintf(posBuf, sizeof(posBuf), "Alarma %u de %u", selectedIndex + 1, count);
        _tft.print(posBuf);
    } else {
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
        _tft.setCursor(12, 176);
        if (editField == ALARM_FIELD_ENABLE) {
            _tft.print("MODO EDICION: [ ACTIVAR / DESACTIVAR ]");
        } else if (editField == ALARM_FIELD_HOUR) {
            _tft.print("MODO EDICION: [ HORA ]");
        } else if (editField == ALARM_FIELD_MINUTE) {
            _tft.print("MODO EDICION: [ MINUTO ]");
        } else if (editField == ALARM_FIELD_DAYS) {
            _tft.print("MODO EDICION: [ DIAS (Presets) ]");
        }

        _tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
        _tft.setCursor(12, 190);
        _tft.print("Btn 1/2: Ajustar  |  Btn 3 corto: Siguiente");

        _tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
        _tft.setCursor(12, 204);
        _tft.print("Mantener Btn 3: Guardar y salir");
    }
}

void DisplayUI::showAlarmAlert(const char* timeStr, const char* label) {
    uint16_t boxColor = ILI9341_MAROON;
    uint16_t textColor = ILI9341_YELLOW;

    _tft.fillRect(16, 40, 288, 150, boxColor);
    _tft.drawRect(14, 38, 292, 154, ILI9341_WHITE);
    _tft.drawRect(15, 39, 290, 152, ILI9341_WHITE);

    _tft.setTextSize(2);
    _tft.setTextColor(textColor, boxColor);
    _tft.setCursor(45, 52);
    _tft.print("! DESPERTADOR !");

    _tft.setTextSize(4);
    _tft.setTextColor(ILI9341_WHITE, boxColor);
    _tft.setCursor(95, 80);
    _tft.print(timeStr ? timeStr : "--:--");

    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_YELLOW, boxColor);
    _tft.setCursor(30, 126);
    _tft.print(label && strlen(label) > 0 ? label : "Alarma Programada");

    _tft.setTextColor(ILI9341_WHITE, boxColor);
    _tft.setCursor(24, 156);
    _tft.print("Pulsa CUALQUIER boton para silenciar");
}

void DisplayUI::sleepDisplay() {
    if (_isSleeping) return;
    if (_ledPin >= 0) {
        digitalWrite(_ledPin, LOW); // Cortar retroiluminación con transistor en pin RX (GPIO3)
    }
    _isSleeping = true;
}

void DisplayUI::wakeDisplay() {
    if (!_isSleeping) return;
    if (_ledPin >= 0) {
        digitalWrite(_ledPin, HIGH); // Encender retroiluminación con transistor en pin RX (GPIO3)
    }
    _isSleeping = false;
}

bool DisplayUI::isSleeping() const {
    return _isSleeping;
}
