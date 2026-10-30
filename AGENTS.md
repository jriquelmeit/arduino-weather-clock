# Instrucciones del repositorio

## Verificación
- Proyecto PlatformIO; `platformio.ini` define `nodemcuv2` (firmware) y `native` (pruebas host). Desde la raíz: `pio run -e nodemcuv2` y `pio test -e native`. No hay linter ni formateador configurados.
- Las pruebas Unity están en `test/test_app_logic.cpp` y prueban lógica auxiliar replicada; no enlazan directamente los módulos de firmware.
- Cargar placa: `pio run -e nodemcuv2 -t upload`. Monitor serie: `pio device monitor` (115200 baudios); requiere la placa conectada.

## Especificaciones técnicas
- Target: NodeMCU v2 / ESP-12E (ESP8266), Arduino, CPU a 160 MHz. Pantalla TFT ILI9341 de 320×240 por SPI, configurada a 80 MHz; dependencias firmware: Adafruit GFX, Adafruit ILI9341 y ArduinoJson.
- Pines definidos en `include/config.h.example`: TFT CS/DC/RST = D8/D3/D4, MOSI/SCK = D7/D5; botones Prev/Next/Action = D1/D2/D6; buzzer = D0. Conserva este mapeo al cambiar hardware o configuración.
- Usa Wi-Fi 2.4 GHz, hora NTP, pronóstico Open-Meteo y servidor web HTTP en puerto 80. Ajustes persistentes (incluidas alarmas) se guardan en LittleFS como `/config.json`.

## Estructura
- `src/main.cpp`: `setup()`, `loop()` e integración entre servicios; `src/*.cpp` contiene implementaciones por servicio.
- `include/*.h`: interfaces, configuración de hardware y tipos compartidos. `include/config.h.example` es la plantilla de configuración.
- `test/`: pruebas Unity ejecutables en el entorno `native`.
- `requerimientos/`: requisitos funcionales del proyecto; `README.md`: documentación de uso, conexiones y API.

## Configuración local
- `include/config.h` está ignorado por Git y contiene credenciales/ajustes locales. Si falta, créalo copiando `include/config.h.example`; no sobrescribas una copia existente ni expongas sus secretos en salida o commits. La compilación de firmware requiere este archivo.
