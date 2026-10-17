# ESP8266 Weather Clock (Lolin NodeMCU V3 + TFT ILI9341 2.8" SPI)

Reloj digital de escritorio con sincronización horaria por internet (NTP) con cambio automático de horario (Chile Continental) y pronóstico del tiempo actual más 2 días posteriores en español a través de la API de Open-Meteo.

---

## 🛠️ Hardware Requerido y Conexiones

Basado en el pinout probado del módulo SPI TFT 2.8" ILI9341:

| Pin Pantalla TFT | Pin NodeMCU (Serigrafía) | GPIO ESP8266 | Descripción |
| :--- | :--- | :--- | :--- |
| **VCC** | **VIN (5V)** ó **3V3** | - | Alimentación |
| **GND** | **GND** | - | Masa común |
| **CS** | **D8** | GPIO15 | Chip Select TFT |
| **RESET / RST** | **D4** | GPIO2 | Reset del controlador TFT |
| **DC / RS** | **D3** | GPIO0 | Data / Command selector |
| **SDI (MOSI)** | **D7** | GPIO13 | Bus HSPI - Datos hacia la pantalla |
| **SCK (CLK)** | **D5** | GPIO14 | Bus HSPI - Reloj SPI |
| **LED** | **3V3** | - | Retroiluminación (Backlight) |
| **SDO (MISO)** | **D6** *(Opcional)* | GPIO12 | Bus HSPI - Lectura de pantalla |

---

## 🚀 Características

- **Reloj Digital Preciso**: Sincronización automática vía servidores NTP (`pool.ntp.org`).
- **Cambio de Horario 100% Automático**: Regla POSIX `CLT4CLST,M9.1.0/0,M4.1.0/0` para Chile Continental (detecta horario de invierno UTC-4 y verano UTC-3 sin intervención manual).
- **Fechas y Textos en Español**: Días de la semana y meses completamente traducidos en la interfaz gráfica.
- **Clima Actual y Pronóstico a 2 Días**:
  - Temperatura actual, humedad relativa, condición y estado atmosférico.
  - Pronóstico para los dos días posteriores (Temperatura máxima, mínima y condición).
  - Iconos climáticos vectoriales procedurales (sol, nublado, lluvia, nieve, tormenta) sin consumo excesivo de memoria.
  - API Open-Meteo (sin necesidad de API keys ni registros).
- **Refresco Flicker-Free**: Actualización de segundos y datos sin parpadeo de pantalla.
- **Optimizado para CLion**: Archivos `CMakeLists.txt` generados para autocompletado y análisis estático en JetBrains CLion.

---

## 📂 Estructura del Proyecto

```text
arduino-weather-clock/
├── platformio.ini              # Configuración y dependencias de PlatformIO
├── CMakeLists.txt              # Integración nativa con CLion
├── .gitignore                  # Exclusiones de Git y credenciales locales
├── include/
│   ├── config.h.example        # Plantilla de configuración
│   ├── config.h                # Configuración activa (SSID, pass, coordenadas)
│   ├── time_service.h          # Gestión de hora, NTP y calendario en español
│   ├── weather_service.h       # Cliente HTTP y parseo JSON de Open-Meteo
│   └── display_ui.h            # Interfaz gráfica ILI9341, layout e iconos
├── src/
│   ├── main.cpp                # Bucle de eventos y orquestación
│   ├── time_service.cpp        # Funciones de tiempo y regla de horario
│   ├── weather_service.cpp     # Deserialización con ArduinoJson 7
│   └── display_ui.cpp          # Renderizado de tarjetas y reloj
└── README.md
```

---

## 💻 Uso en CLion

1. Abre la carpeta `arduino-weather-clock` en **CLion**.
2. CLion reconocerá el proyecto automáticamente mediante el `CMakeLists.txt` generado.
3. Para compilar, subir firmware o abrir el monitor serie puedes usar las herramientas de PlatformIO integradas en CLion o la terminal:

### Comandos de Terminal

- **Compilar el proyecto**:
  ```bash
  pio run
  ```
- **Subir el firmware a la placa NodeMCU**:
  ```bash
  pio run -t upload
  ```
- **Abrir el monitor serie (115200 baudios)**:
  ```bash
  pio device monitor
  ```
