#include "display_ui.h"
#include <ctype.h>

#define COLOR_BG_HEADER       0x0926
#define COLOR_BORDER_CARD     0x2145
#define COLOR_TEXT_MUTED      0x8410

static void toUpperString(const char* src, char* dst, size_t maxLen) {
    size_t i = 0;
    while (src && src[i] && i < maxLen - 1) {
        dst[i] = (char)toupper((unsigned char)src[i]);
        i++;
    }
    dst[i] = '\0';
}

DisplayUI::DisplayUI(uint8_t cs, uint8_t dc, uint8_t rst)
    : _tft(cs, dc, rst), _lastRssi(-999), _lastWifiState(false) {
    _lastTimeStr[0] = '\0';
    _lastDateStr[0] = '\0';
}

void DisplayUI::begin(uint8_t rotation) {
    _tft.begin();
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

void DisplayUI::drawBaseLayout(const char* locationName) {
    _tft.fillScreen(ILI9341_BLACK);

    // 1. Barra de Cabecera
    _tft.fillRect(0, 0, 320, 26, COLOR_BG_HEADER);
    _tft.drawFastHLine(0, 26, 320, ILI9341_CYAN);

    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_CYAN, COLOR_BG_HEADER);
    _tft.setCursor(8, 6);
    _tft.print(locationName);

    // 2. Tarjeta Reloj Digital (Izquierda superior)
    _tft.drawRoundRect(4, 30, 184, 102, 6, ILI9341_DARKGREY);
    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
    _tft.setCursor(12, 36);
    _tft.print("HORA LOCAL");

    // 3. Tarjeta Clima Actual (Derecha superior)
    _tft.drawRoundRect(192, 30, 124, 102, 6, 0x2A6D);
    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    _tft.setCursor(198, 36);
    _tft.print("CLIMA ACTUAL");

    // 4. Tarjeta Pronóstico Día 1 (Izquierda inferior)
    _tft.drawRoundRect(4, 136, 154, 86, 6, COLOR_BORDER_CARD);

    // 5. Tarjeta Pronóstico Día 2 (Derecha inferior)
    _tft.drawRoundRect(162, 136, 154, 86, 6, COLOR_BORDER_CARD);

    // 6. Línea divisoria barra inferior
    _tft.drawFastHLine(0, 224, 320, ILI9341_DARKGREY);

    _lastTimeStr[0] = '\0';
    _lastDateStr[0] = '\0';
}

void DisplayUI::updateTime(const DateTimeInfo& timeInfo) {
    // Actualización flicker-free del reloj
    if (strncmp(_lastTimeStr, timeInfo.timeStr, sizeof(_lastTimeStr)) != 0) {
        _tft.setTextSize(3);
        _tft.setTextColor(ILI9341_GREENYELLOW, ILI9341_BLACK);
        _tft.setCursor(10, 52);
        _tft.print(timeInfo.timeStr);
        strncpy(_lastTimeStr, timeInfo.timeStr, sizeof(_lastTimeStr));
    }

    // Subtítulo: Día de la semana
    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    _tft.setCursor(12, 86);
    _tft.fillRect(12, 86, 168, 16, ILI9341_BLACK);
    _tft.print(timeInfo.dayName);

    // Indicador de sincronización
    _tft.setTextSize(1);
    _tft.setCursor(12, 112);
    _tft.setTextColor(COLOR_TEXT_MUTED, ILI9341_BLACK);
    _tft.fillRect(12, 112, 168, 10, ILI9341_BLACK);
    _tft.print("NTP Sincronizado");
}

void DisplayUI::updateDate(const DateTimeInfo& timeInfo) {
    if (strncmp(_lastDateStr, timeInfo.dateStr, sizeof(_lastDateStr)) != 0) {
        _tft.fillRect(110, 8, 175, 12, COLOR_BG_HEADER);
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_WHITE, COLOR_BG_HEADER);
        _tft.setCursor(110, 9);
        _tft.print(timeInfo.dateStr);
        strncpy(_lastDateStr, timeInfo.dateStr, sizeof(_lastDateStr));
    }
}

void DisplayUI::updateWeather(const WeatherData& weather) {
    if (!weather.valid) return;

    // --- Panel Clima Actual ---
    // Limpiar zona de temperatura y condición
    _tft.fillRect(195, 50, 118, 78, ILI9341_BLACK);

    // Icono clima actual (tamaño regular)
    drawWeatherIcon(214, 70, weather.current.iconType, false);

    // Temperatura actual
    _tft.setTextSize(2);
    _tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
    _tft.setCursor(242, 62);
    _tft.printf("%.1f C", weather.current.temperature);

    // Humedad
    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
    _tft.setCursor(198, 92);
    _tft.printf("Humedad: %d%%", weather.current.humidity);

    // Condición en español
    _tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    _tft.setCursor(198, 106);
    char condBuffer[20];
    snprintf(condBuffer, sizeof(condBuffer), "%-18.18s", weather.current.conditionText);
    _tft.print(condBuffer);

    // --- Tarjetas de Pronóstico a 2 Días (Simétricas con caracteres de mayor tamaño) ---
    for (int i = 0; i < 2; i++) {
        int cardX = (i == 0) ? 4 : 162;
        int cardW = 154;

        // Limpiar interior de la tarjeta
        _tft.fillRect(cardX + 2, 138, cardW - 4, 82, ILI9341_BLACK);

        // 1. Título del día en Tamaño 2
        char dayTitle[16];
        if (i == 0) {
            strncpy(dayTitle, "MANANA", sizeof(dayTitle));
        } else {
            toUpperString(weather.forecast[1].dayName, dayTitle, sizeof(dayTitle));
        }

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(cardX + 10, 140);
        _tft.print(dayTitle);

        // Línea divisoria interior
        _tft.drawFastHLine(cardX + 6, 158, cardW - 12, COLOR_BORDER_CARD);

        // 2. Icono meteorológico en el lateral izquierdo
        drawWeatherIcon(cardX + 22, 178, weather.forecast[i].iconType, true);

        // 3. Temperatura Máxima en Tamaño 2 (doble tamaño)
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
        _tft.setCursor(cardX + 44, 168);
        _tft.print("Max");

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
        _tft.setCursor(cardX + 68, 164);
        _tft.printf("%.1f C", weather.forecast[i].tempMax);

        // 4. Temperatura Mínima en Tamaño 2 (doble tamaño)
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(cardX + 44, 186);
        _tft.print("Min");

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
        _tft.setCursor(cardX + 68, 182);
        _tft.printf("%.1f C", weather.forecast[i].tempMin);

        // 5. Condición del tiempo en español
        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
        _tft.setCursor(cardX + 10, 204);
        char fBuffer[24];
        snprintf(fBuffer, sizeof(fBuffer), "%-22.22s", weather.forecast[i].conditionText);
        _tft.print(fBuffer);
    }
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
    if (connected != _lastWifiState || abs(rssi - _lastRssi) > 4) {
        drawWifiIndicator(298, 7, connected, rssi);
        _lastWifiState = connected;
        _lastRssi = rssi;
    }
}

void DisplayUI::updateStatusBar(const char* message) {
    _tft.fillRect(0, 226, 320, 14, ILI9341_BLACK);
    _tft.setTextSize(1);
    _tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
    _tft.setCursor(6, 228);
    _tft.print(message);
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

        case ICON_PARTLY_CLOUDY:
            // Sol atrás a la izquierda
            _tft.fillCircle(x - (small ? 4 : 6), y - (small ? 4 : 6), small ? 5 : 7, ILI9341_YELLOW);
            // Nube enfrente
            _tft.fillCircle(x, y + 2, small ? 5 : 7, ILI9341_WHITE);
            _tft.fillCircle(x + (small ? 6 : 9), y + 3, small ? 4 : 6, ILI9341_LIGHTGREY);
            _tft.fillRect(x - (small ? 4 : 6), y + (small ? 3 : 4), small ? 14 : 20, small ? 5 : 7, ILI9341_WHITE);
            break;

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
