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

## 🖥️ Interfaz en Pantalla (Layout 320x240)

Representación gráfica de la distribución de la pantalla en modo horizontal:

```text
Página 1 (Modo Diario - DISPLAY_PAGE_MODE = 1):
+---------------------------------------------------------------+
| Santiago          Sabado, 17 de Octubre               [||||]  |
+---------------------------------------------------------------+
| +-------------------------+ +-------------------------------+ |
| | HORA LOCAL              | | PRONOSTICO HOY                | |
| |                         | |                               | |
| |      14:05              | |    \ | /       18.2 C         | |
| |                         | |   -- O --                     | |
| |   Sabado                | |    / | \   Max 22.5 | Min 12.0| |
| |   NTP Sincronizado      | |   [💧] 80%         3.5 mm     | |
| +-------------------------+ +-------------------------------+ |
|                                                               |
| +-------------------------+ +-------------------------------+ |
| | Domingo                 | | Lunes                         | |
| |-------------------------| |-------------------------------| |
| |    .--.    Max  23.6 C  | |   \ | /    Max  24.1 C        | |
| |   (    )   Min  13.2 C  | |  --( O )   Min  14.4 C        | |
| |  ' ' ' '                | |   (____)                      | |
| |  [💧] 85%       4.2 mm  | |  [💧] 10%       0.0 mm        | |
| +-------------------------+ +-------------------------------+ |
+---------------------------------------------------------------+
| IP: 192.168.1.50 | RSSI: -58dBm | Act: 14:05                  |
+---------------------------------------------------------------+

Página 2 (Modo Horario - DISPLAY_PAGE_MODE = 2):
+---------------------------------------------------------------+
| Santiago          Sabado, 17 de Octubre               [||||]  |
+---------------------------------------------------------------+
| +-------------------------+ +-------------------------------+ |
| | HORA LOCAL              | | PRONOSTICO HOY                | |
| |      14:05              | |    \ | /       18.2 C         | |
| |   Sabado                | |    / | \   Max 22.5 | Min 12.0| |
| |   NTP Sincronizado      | |   [💧] 80%         3.5 mm     | |
| +-------------------------+ +-------------------------------+ |
|                                                               |
| +------------+ +------------+ +------------+ +------------+   |
| |   16:00    | |   18:00    | |   20:00    | |   22:00    |   |
| |------------| |------------| |------------| |------------|   |
| |   \ | /    | |   (____)   | |    .--.    | |    .--.    |   |
| |   19.5 C   | |   17.2 C   | |  ' ' ' '   | |  ' ' ' '   |   |
| |            | |            | |   15.0 C   | |   13.4 C   |   |
| |   [💧] 15% | |   [💧] 40% | |   [💧] 75% | |   [💧] 90% |   |
| |   0.0 mm   | |   0.4 mm   | |   1.8 mm   | |   3.2 mm   |   |
| +------------+ +------------+ +------------+ +------------+   |
+---------------------------------------------------------------+
| IP: 192.168.1.50 | RSSI: -58dBm | Act: 14:05                  |
+---------------------------------------------------------------+

Página 3 (Modo Temporizador - DISPLAY_PAGE_MODE = 3):
+---------------------------------------------------------------+
| Santiago          Sabado, 17 de Octubre               [||||]  |
+---------------------------------------------------------------+
| +-----------------------------------------------------------+ |
| | TEMPORIZADOR                                              | |
| |                                                           | |
| |                       15:00                               | |
| |                                                           | |
| |          [======== PROGRESO RESTANTE ========]            | |
| |                                                           | |
| |    MODO AJUSTE                                            | |
| |    Btn 3: +5 min  |  Mantener: INICIAR                    | |
| +-----------------------------------------------------------+ |
+---------------------------------------------------------------+
| IP: 192.168.1.50 | RSSI: -58dBm | Act: 14:05                  |
+---------------------------------------------------------------+
```

---

## Características

- **Navegación Interactiva por Botones:** Navegación cíclica entre páginas (Páginas 1, 2 y 3) mediante botones físicos con filtro antirrebote.
- **Página de Temporizador (Timer) con Alarma:** Contador regresivo ajustable de 5 en 5 minutos (5 a 60 min). Ejecución no bloqueante en segundo plano con alarma sonora mediante buzzer al llegar a cero.
- **Apagado y Modo Noche (Sleep Mode):** Apagado manual de pantalla mediante el botón de acción en las páginas de reloj y clima, o apagado programado automático por horario (modo nocturno). Reactivación instantánea al pulsar cualquier botón.
- **Sincronización NTP y Reglas POSIX:** Obtención automática de fecha y hora local con cálculo de cambio de horario estacional (horario de verano e invierno) en el microcontrolador sin llamadas web adicionales.
- **Pronóstico Meteorológico Completo:** Temperatura actual, porcentaje de humedad, condición del cielo y pronóstico de los 2 días posteriores (mínimas y máximas).
- **API sin Claves ni Registro:** Emplea Open-Meteo mediante peticiones HTTP filtradas de bajo consumo en RAM (<2 KB en heap).
- **Iconografía Climática Vectorial:** Generación procedural de iconos (sol, parcialmente nublado, cubierto, lluvia, nieve, tormentas) que ahorran memoria flash y ofrecen escalado limpio.
- **Visualización Optimizada:** Actualización de dígitos del reloj por sobreescritura de fondo para eliminar el parpadeo en pantalla completa.
- **Integración con CLion y PlatformIO:** Configuración lista para compilar e indexar con CMake en JetBrains CLion o mediante la CLI de PlatformIO.

---

## Diagrama de Conexiones

### Asignación de Pines Completa (NodeMCU V3 -> Pantalla TFT, Botones y Buzzer)

| Periférico | Pin Módulo / Botón | Pin NodeMCU | GPIO ESP8266 | Función / Configuración |
| :--- | :--- | :--- | :--- | :--- |
| **Pantalla TFT** | **VCC** | **VIN (5V)** ó **3V3** | - | Alimentación eléctrica |
| **Pantalla TFT** | **GND** | **GND** | - | Tierra común |
| **Pantalla TFT** | **CS** | **D8** | GPIO15 | TFT Chip Select |
| **Pantalla TFT** | **RESET / RST** | **D4** | GPIO2 | Reset del controlador TFT |
| **Pantalla TFT** | **DC / RS** | **D3** | GPIO0 | Selección de Datos / Comando |
| **Pantalla TFT** | **SDI (MOSI)** | **D7** | GPIO13 | Bus HSPI - Datos hacia la pantalla |
| **Pantalla TFT** | **SCK (CLK)** | **D5** | GPIO14 | Bus HSPI - Reloj SPI |
| **Pantalla TFT** | **LED** | **3V3** ó Transistor NPN | - / GPIO3 (RX) | Retroiluminación (directo a 3V3 ó con 2N2222 en pin RX) |
| **Botón 1 (Prev)** | Terminal A / GND | **D1** | GPIO5 | `INPUT_PULLUP` - Retroceder página |
| **Botón 2 (Next)** | Terminal A / GND | **D2** | GPIO4 | `INPUT_PULLUP` - Avanzar página |
| **Botón 3 (Action)** | Terminal A / GND | **D6** | GPIO12 | `INPUT_PULLUP` - Ajuste timer / Sleep / Silenciar |
| **Buzzer** | Señal / (+) / GND | **D0** | GPIO16 | `OUTPUT` - Alarma de temporizador |

```text
                                Lolin NodeMCU V3 (ESP8266)
                               +--------------------------+
  [ Pulsador 1: Ant/Prev ] ----| D1 (GPIO5)               |
  [ Pulsador 2: Sig/Next ] ----| D2 (GPIO4)           VIN |---> Alimentación 5V / VCC TFT
  [ Pulsador 3: Acción   ] ----| D6 (GPIO12)          GND |---> GND Común (Botones, Buzzer, TFT)
  [ Buzzer (+) / Señal   ] ----| D0 (GPIO16)          3V3 |---> TFT LED Backlight (si no se usa transistor)
                               | D8 (GPIO15)              |---> TFT CS
                               | D4 (GPIO2)               |---> TFT RST
                               | D3 (GPIO0)               |---> TFT DC
                               | D7 (GPIO13)              |---> TFT MOSI (SDI)
                               | D5 (GPIO14)              |---> TFT CLK (SCK)
                               | RX (GPIO3)               |---> Base Transistor NPN (Opcional - Backlight)
                               +--------------------------+

  * Cada pulsador se conecta directamente entre su pin (D1, D2, D6) y GND Común (usa pull-up interno).
  * El buzzer se conecta entre D0 y GND Común (o VCC/GND + señal a D0 si es módulo).
```

### Control de Retroiluminación por Transistor (Apagado 100% Negro)

Por defecto, si conectas el pin `LED` de la pantalla directo a `3V3`, el modo Sleep apaga los píxeles pero la luz de fondo permanece encendida (pantalla blanca). Para lograr un **apagado 100% en negro total**, se puede intercalar un transistor NPN (ej. **2N2222**, **2N3904** o **BC547**):

```text
       NodeMCU ESP8266                                 Pantalla TFT
      +----------------+                              +------------+
      |   RX (GPIO3) --+---[ Resistencia 1k ]         |            |
      |                |            |                 |            |
      |                |         (Base)               |            |
      |                |          /                   |            |
      |                |     +---|<  NPN (2N2222)     |            |
      |                |     |    \                   |            |
      |                |     |   (Colector)           |            |
      |                |     |      +---------------> | LED        |
      |                |     |                        |            |
      |          GND --+-----+----------------------> | GND        |
      +----------------+  (Emisor)                    +------------+
```

> [!NOTE]
> Los pines táctiles (`T_CLK`, `T_CS`, `T_DIN`, `T_DO`, `T_IRQ`) y el lector de tarjeta micro-SD (`SD_CS`, etc.) no son necesarios para este proyecto y pueden permanecer desconectados.

### Funciones de los Botones

| Botón | Pantalla Activa | Pulsación Corta (Click) | Pulsación Larga (> 1 seg) |
| :--- | :--- | :--- | :--- |
| **Botón 1 (Prev / D1)** | Cualquiera | Retroceder página ($3 \rightarrow 2 \rightarrow 1 \rightarrow 3$) | - |
| **Botón 2 (Next / D2)** | Cualquiera | Avanzar página ($1 \rightarrow 2 \rightarrow 3 \rightarrow 1$) | - |
| **Botón 3 (Action / D6)** | Página 1 (Diario) | **Apagar / Encender pantalla** (Sleep Toggle) | - |
| **Botón 3 (Action / D6)** | Página 2 (Horario) | **Apagar / Encender pantalla** (Sleep Toggle) | - |
| **Botón 3 (Action / D6)** | Página 3 (Timer - Ajuste) | Sumar tiempo (**+5 min**) | **Iniciar conteo regresivo** |
| **Botón 3 (Action / D6)** | Página 3 (Timer - Conteo) | **Detener / Cancelar** conteo | - |
| **Cualquier Botón** | Alarma sonando | **Silenciar alarma del buzzer** | Silenciar alarma del buzzer |
| **Cualquier Botón** | Pantalla en reposo (Sleep) | **Encender pantalla / Despertar** | Encender pantalla / Despertar |

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

// Modo de Pantalla (Página activa)
// 1: Página 1 - Pronóstico diario a 2 días (Por defecto)
// 2: Página 2 - Pronóstico por horas (4 bloques)
#define DISPLAY_PAGE_MODE      1

// Intervalo entre pronósticos para la Página 2 (1, 2, 3 o n horas)
#define HOURLY_FORECAST_STEP   2
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
│   ├── config.h                # Configuración local de Wi-Fi, pines y temporizador
│   ├── time_service.h          # Declaración de funciones NTP y calendario en español
│   ├── weather_service.h       # Modelo de datos meteorológicos y tipos de iconos
│   ├── display_ui.h            # Controladores del display TFT, layout y temporizador
│   ├── input_controller.h      # Controlador no bloqueante de botones (antirrebote y long press)
│   └── buzzer_controller.h     # Controlador no bloqueante del buzzer y patrones de alarma
├── src/
│   ├── main.cpp                # Setup, bucle de ejecución, navegación y temporizador
│   ├── time_service.cpp        # Manejo de SNTP nativo y formateo en español
│   ├── weather_service.cpp     # Consulta HTTP a Open-Meteo y deserialización JSON
│   ├── display_ui.cpp          # Renderizado gráfico de reloj, clima y temporizador
│   ├── input_controller.cpp    # Detección de clics y pulsaciones sostenidas
│   └── buzzer_controller.cpp   # Patrones acústicos para alarmas y confirmaciones
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
