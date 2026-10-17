# ESP8266 Weather Clock

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Core_6-orange?logo=platformio)](https://platformio.org/)
[![ESP8266](https://img.shields.io/badge/Board-NodeMCU_V3_(ESP8266)-blue?logo=espressif)](https://www.espressif.com/)
[![Display](https://img.shields.io/badge/Display-ILI9341_SPI_2.8''-red)](https://www.adafruit.com/)
[![Framework](https://img.shields.io/badge/Framework-Arduino-00979D?logo=arduino)](https://www.arduino.cc/)
[![API](https://img.shields.io/badge/Weather_API-Open--Meteo-brightgreen)](https://open-meteo.com/)
[![IDE](https://img.shields.io/badge/IDE-CLion-000000?logo=clion)](https://www.jetbrains.com/clion/)

Reloj digital de escritorio conectado a Internet para la placa **Lolin NodeMCU V3 (ESP8266)** y pantalla gráfica **TFT SPI 2.8" (ILI9341)**. 

El sistema sincroniza la hora local vía servidores NTP aplicando cambio automático de horario de verano/invierno (DST), y consulta periódicamente las condiciones meteorológicas actuales más el pronóstico de los dos días posteriores utilizando la API de **Open-Meteo**, con interfaz completamente en **español** y redibujado *flicker-free* (sin parpadeos).

---

## Características

- **Sincronización NTP y Reglas POSIX:** Obtención automática de fecha y hora local con cálculo de cambio de horario estacional (horario de verano e invierno) en el microcontrolador sin llamadas web adicionales.
- **Pronóstico Meteorológico Completo:** Temperatura actual, porcentaje de humedad, condición del cielo y pronóstico de los 2 días posteriores (mínimas y máximas).
- **API sin Claves ni Registro:** Emplea Open-Meteo mediante peticiones HTTP filtradas de bajo consumo en RAM (<2 KB en heap).
- **Iconografía Climática Vectorial:** Generación procedural de iconos (sol, parcialmente nublado, cubierto, lluvia, nieve, tormentas) que ahorran memoria flash y ofrecen escalado limpio.
- **Visualización Optimizada:** Actualización de dígitos del reloj por sobreescritura de fondo para eliminar el parpadeo en pantalla completa.
- **Integración con CLion y PlatformIO:** Configuración lista para compilar e indexar con CMake en JetBrains CLion o mediante la CLI de PlatformIO.

---

## Diagrama de Conexiones

### Asignación de Pines (Lolin NodeMCU V3 -> Pantalla TFT SPI 2.8")

| Pin TFT (ILI9341) | Pin NodeMCU (Serigrafía) | GPIO ESP8266 | Función |
| :--- | :--- | :--- | :--- |
| **VCC** | **VIN (5V)** ó **3V3** | - | Alimentación eléctrica |
| **GND** | **GND** | - | Tierra común |
| **CS** | **D8** | GPIO15 | TFT Chip Select |
| **RESET / RST** | **D4** | GPIO2 | Reset del controlador TFT |
| **DC / RS** | **D3** | GPIO0 | Selección de Datos / Comando |
| **SDI (MOSI)** | **D7** | GPIO13 | Bus HSPI - Datos hacia la pantalla |
| **SCK (CLK)** | **D5** | GPIO14 | Bus HSPI - Reloj SPI |
| **LED** | **3V3** | - | Retroiluminación (Backlight) |
| **SDO (MISO)** | **D6** *(Opcional)* | GPIO12 | Bus HSPI - Lectura (no requerida) |

```text
       +----------------------------+
       |   Lolin NodeMCU V3         |
       |      (ESP8266)             |
       +----------------------------+
       |   VIN (5V) / 3V3           | -----> VCC   (Módulo TFT 2.8")
       |   GND                      | -----> GND   (Módulo TFT 2.8")
       |   D8 (GPIO15)              | -----> CS    (TFT Chip Select)
       |   D4 (GPIO2)               | -----> RESET (TFT Reset)
       |   D3 (GPIO0)               | -----> DC/RS (Data / Command)
       |   D7 (GPIO13 - HSPI MOSI)  | -----> SDI   (MOSI / TFT Data In)
       |   D5 (GPIO14 - HSPI CLK)   | -----> SCK   (CLK / SPI Clock)
       |   3V3                      | -----> LED   (TFT Backlight)
       |   D6 (GPIO12 - HSPI MISO)  | .....> SDO   (MISO - Opcional)
       +----------------------------+
```

> [!NOTE]
> Los pines táctiles (`T_CLK`, `T_CS`, `T_DIN`, `T_DO`, `T_IRQ`) y el lector de tarjeta micro-SD (`SD_CS`, etc.) no son necesarios para este proyecto y pueden permanecer desconectados.

---

## Configuración y Puesta en Marcha

### 1. Clonar el Repositorio

```bash
git clone https://github.com/jriquelmeit/arduino-weather-clock.git
cd arduino-weather-clock
```

### 2. Configurar Credenciales y Ubicación

Copia la plantilla de configuración:

```bash
cp include/config.h.example include/config.h
```

Edita `include/config.h` con los datos de tu red y coordenadas geográficas:

```cpp
// Red Wi-Fi (solo redes de 2.4 GHz)
#define WIFI_SSID       "Tu_Red_WiFi"
#define WIFI_PASSWORD   "Tu_Contraseña"

// Ubicación para el pronóstico meteorológico
#define LOCATION_NAME   "Santiago"
#define LOCATION_LAT    "-33.4489"
#define LOCATION_LON    "-70.6693"

// Regla POSIX de Zona Horaria (cambio automático de horario)
#define TIMEZONE_RULE   "CLT4CLST,M9.1.0/0,M4.1.0/0"
```

> [!IMPORTANT]
> El archivo `include/config.h` está listado en `.gitignore` para evitar que tus contraseñas se suban a repositorios remotos.

---

## Reglas de Zona Horaria (TZ POSIX)

El proyecto utiliza la biblioteca estándar `time.h` del ESP8266 para computar automáticamente los cambios estacionales de hora según la regla POSIX configurada:

| Región | Regla POSIX | Comportamiento |
| :--- | :--- | :--- |
| **Chile Continental (Santiago)** | `"CLT4CLST,M9.1.0/0,M4.1.0/0"` | Invierno UTC-4 (abril), Verano UTC-3 (septiembre) |
| **España (Península / Madrid)** | `"CET-1CEST,M3.5.0,M10.5.0/3"` | Invierno UTC+1 (octubre), Verano UTC+2 (marzo) |
| **Argentina / Uruguay** | `"<-03>3"` | Fijo en UTC-3 sin horario de verano |
| **México (Centro)** | `"CST6"` | Fijo en UTC-6 |

---

## Compilación y Carga

### Desde Terminal con PlatformIO

1. **Compilar el código:**
   ```bash
   pio run
   ```
2. **Subir el firmware a la placa:**
   ```bash
   pio run -t upload
   ```
3. **Monitorear la salida serie (115200 baudios):**
   ```bash
   pio device monitor
   ```

### Desde JetBrains CLion

1. Abre el directorio del proyecto en CLion (`File` -> `Open...`).
2. El entorno detectará el archivo `CMakeLists.txt` autogenerado por PlatformIO.
3. Utiliza los perfiles de ejecución preconfigurados (`PlatformIO Upload`, `PlatformIO Monitor`) desde la barra superior de CLion.

---

## Estructura del Código

```text
arduino-weather-clock/
├── platformio.ini              # Configuración de compilación, microcontrolador y librerías
├── CMakeLists.txt              # Configuración de CMake para CLion
├── .gitignore                  # Exclusiones de control de versiones y credenciales
├── include/
│   ├── config.h.example        # Plantilla pública de parámetros
│   ├── config.h                # Configuración local de Wi-Fi y ubicación (ignorado por Git)
│   ├── time_service.h          # Declaración de funciones NTP y calendario en español
│   ├── weather_service.h       # Modelo de datos meteorológicos y tipos de iconos
│   └── display_ui.h            # Controladores del display TFT y layout visual
├── src/
│   ├── main.cpp                # Setup, bucle de ejecución y temporizadores no bloqueantes
│   ├── time_service.cpp        # Manejo de SNTP nativo y formateo en español
│   ├── weather_service.cpp     # Consulta HTTP a Open-Meteo y deserialización JSON
│   └── display_ui.cpp          # Renderizado de componentes gráficos y reloj
└── README.md
```

---

## Solución de Problemas

- **Pantalla blanca sin contenido:**
  - Comprueba que los cables de reloj (`SCK`/`D5`) y datos (`SDI`/`MOSI`/`D7`) estén bien conectados al bus HSPI del ESP8266.
  - Verifica que `CS` esté conectado a `D8` y `DC` a `D3`.
- **Fallo al conectar a Wi-Fi:**
  - El microcontrolador ESP8266 es compatible únicamente con redes Wi-Fi de **2.4 GHz**.
- **La placa no bootea o entra en modo flasheo constante:**
  - Los pines `D3 (GPIO0)` y `D8 (GPIO15)` determinan el modo de arranque del ESP8266. Si la pantalla produce caídas de tensión al encenderse, conecta la alimentación `VCC` del display a `VIN (5V)` en lugar de `3V3`.
