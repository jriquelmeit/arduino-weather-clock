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

Página 3 (Modo Dashboard Central - DISPLAY_PAGE_MODE = 3):
+---------------------------------------------------------------+
| 24 de Octubre 2026 - S. Antonio M. Claret             [||||]  |
+-----------+---------------------------------------+-----------+
|  ACTUAL   |                                       |    HOY    |
|           |                SABADO                 |           |
|    \ /    |  -----------------------------------  |   \ | /   |
|   --O--   |                                       |  --( O )  |
|    / \    |                                       |   (____)  |
|  Despej.  |               1 4 : 3 5               |           |
|           |                                       |    MAX    |
|   TEMP    |       (Reloj Gigante Centrado)        |   16.2 C  |
|  15.5 C   |                                       |           |
|           |  -----------------------------------  |    MIN    |
|  HUMEDAD  |  [==== Barra de Segundos (0..59s) ==] |    9.2 C  |
|  [💧] 49% |                                       |           |
|           |           Mayormente claro            |  LLUVIA   |
| INDICE UV |         Hora sincronizada NTP         |   5.6 mm  |
|    3.5    |                                       |    80%    |
|   Mod.    |                                       | (o UV MAX)|
+-----------+---------------------------------------+-----------+
| IP: 192.168.1.50 | RSSI: -58dBm | Act: 14:05                  |
+---------------------------------------------------------------+

Página 4 (Modo Temporizador - DISPLAY_PAGE_MODE = 4):
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

- **Panel de Control Web en Tiempo Real:** Servidor web integrado accesible desde cualquier dispositivo en la red local (`http://<IP-DEL-RELOJ>/`) para cambiar páginas, controlar el temporizador, activar/desactivar modo reposo (sleep) y cambiar/probar melodías de alarma.
- **Portal de Configuración Wi-Fi (Modo AP / Captive Portal):** Al enchufar o reiniciar manteniendo presionado el **Botón 3 (D6)** (o si no hay Wi-Fi configurado), el ESP8266 genera una red propia `WeatherClock-Setup` con portal cautivo (`192.168.4.1`) que permite escanear redes 2.4 GHz, ingresar contraseñas y configurar ciudad/coordenadas.
- **Memoria Persistente (LittleFS):** La configuración de Wi-Fi, ubicación, página inicial y melodía se almacena de forma no volátil en la memoria flash interna del ESP8266, manteniéndose intacta al apagar o reiniciar el reloj.
- **Navegación Interactiva por Botones:** Navegación cíclica entre 4 páginas (Páginas 1, 2, 3 y 4) mediante botones físicos con filtro antirrebote.
- **Página 3 (Dashboard Central con Reloj Gigante):** Vista con Reloj Gigante en verde lima, barra de segundos fluida (0..59s), Santoral católico del día, condición, temperatura, humedad e Índice UV actual y máximo proyectado.
- **Página de Temporizador (Timer en Página 4):** Contador regresivo ajustable de 5 en 5 minutos (5 a 60 min). Ejecución no bloqueante en segundo plano con alarma sonora mediante buzzer o parlante al llegar a cero.
- **Audio Polifónico y 13 Melodías Retro:** Soporte para buzzer activo o parlante de cono de 8Ω (0.5W a 2W) mediante transistor NPN (2N2222). Generación de frecuencias reales con `tone()` para reproducir 13 melodías completas estilo chiptune (Star Wars, Super Mario Bros, The Legend of Zelda, Tetris, Pac-Man, Nokia Tune, Cantina Band, Doom, Casio Beep, Volver al Futuro, Los Simpson, Sonic the Hedgehog y Alien) con cancelación interactiva por botones o desde la web.
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
| **Pantalla TFT** | **LED** | **3V3** ó Transistor NPN | - / GPIO3 (RX) | Retroiluminación (directo a 3V3 ó con 2N2222/2N3904 en pin RX) |
| **Botón 1 (Prev)** | Terminal A / GND | **D1** | GPIO5 | `INPUT_PULLUP` - Retroceder página |
| **Botón 2 (Next)** | Terminal A / GND | **D2** | GPIO4 | `INPUT_PULLUP` - Avanzar página |
| **Botón 3 (Action)** | Terminal A / GND | **D6** | GPIO12 | `INPUT_PULLUP` - Ajuste timer / Sleep / Silenciar |
| **Audio (Buzzer / Parlante)** | Pin (+) Buzzer ó Base 2N2222 | **D0** | GPIO16 | `OUTPUT` - Frecuencias `tone()` para alarma y melodías |

### Diagrama Esquemático Completo de Conexiones

```text
                                 DIAGRAMA COMPLETO DE CONEXIONES
                                =================================

     BOTONES Y BUZZER                     LOLIN NODEMCU V3 (ESP8266)                   PANTALLA TFT ILI9341 2.8"
  +---------------------+               +----------------------------+                +-------------------------+
  |                     |               |                            |                |                         |
  | [Pulsador 1: Prev]  +---------------+ D1 (GPIO5)             VIN +----------------+ VCC (VIN / 5V)          |
  |   (Terminal a GND)  |               |                            |                |                         |
  |                     |               |                            |                |                         |
  | [Pulsador 2: Next]  +---------------+ D2 (GPIO4)             GND +--------+-------+ GND                     |
  |   (Terminal a GND)  |               |                            |        |       |                         |
  |                     |               |                            |        |       |                         |
  | [Pulsador 3: Act]   +---------------+ D6 (GPIO12)            D8  +--------|-------+ CS                      |
  |   (Terminal a GND)  |               |                            |        |       |                         |
  |                     |               |                            |        |       |                         |
  | [BUZZER Activo]     |               |                        D4  +--------|-------+ RESET / RST             |
  |   Pin (+) / Señal   +---------------+ D0 (GPIO16)                |        |       |                         |
  |   Pin (-) a GND     |               |                        D3  +--------|-------+ DC / RS                 |
  +---------------------+               |                            |        |       |                         |
                                        |                        D7  +--------|-------+ SDI (MOSI)              |
                                        |                            |        |       |                         |
                                        |                        D5  +--------|-------+ SCK (CLK)               |
                                        |                            |        |       |                         |
                                        |                        3V3 +----+---|-------+ LED (Opción Directa)*   |
                                        |                            |    |   |       |                         |
                                        |                   RX (GPIO3)+-+ |   |       | SDO (MISO) [DESCONECTAR]|
                                        +----------------------------+  | |   |       +-------------------------+
                                                                        | |   |
                         CIRCUITO CON TRANSISTOR NPN (OPCIÓN 100% NEGRO)| |   |
                        +-----------------------------------------------+ |   |
                        |                                               | |   |
                        |   3V3 (ó VIN) -----------------------------+  | |   |
                        |                                            |  | |   |
                        |                                        (Colector)   |
                        |                                           +-+ | |   |
                        |                                          /  | | |   |
                        |   Pin RX (GPIO3) ----[ Resistencia 1k ]-+- 2N2222   |
                        |                                  (Base)  \  | | |   |
                        |                                           +-+ | |   |
                        |                                        (Emisor) |   |
                        |                                            +------> (Al pin LED de la TFT)*
                        +-----------------------------------------------+

  * NOTA DE RETROILUMINACIÓN (LED):
    - Opción A (Sin transistor): Conectar pin LED de la pantalla directo a 3V3 del NodeMCU.
    - Opción B (Con transistor 2N2222 / 2N3904): Conectar Colector a 3V3 (ó VIN), Base a pin RX (GPIO3) con resistencia de 1k, y Emisor al pin LED de la pantalla (seguidor de emisor).
  * NOTA DE PINES NO USADOS:
    - SDO (MISO) de la pantalla debe quedar DESCONECTADO (al aire) para no interferir con el pulsador D6.
    - Los pines táctiles (T_CLK, T_CS, T_DIN, T_DO, T_IRQ) y lector SD quedan desconectados.
```

### Control de Retroiluminación por Transistor (Apagado 100% Negro)

Por defecto, si conectas el pin `LED` de la pantalla directo a `3V3`, el modo Sleep apaga los píxeles pero la luz de fondo permanece encendida (pantalla blanca). Para lograr un **apagado 100% en negro total**, se intercala un transistor NPN (ej. **2N2222** o **2N3904**) en modo **seguidor de emisor**, ya que el pin `LED` del módulo ILI9341 requiere voltaje positivo con respecto al GND común:

```text
       NodeMCU ESP8266                                 Pantalla TFT
      +----------------+                              +------------+
      |  3V3 (ó VIN)* -+-------------------------+    |            |
      |                |                         |    |            |
      |                |                    (Colector)|            |
      |                |                        +--+  |            |
      |                |                       /      |            |
      |   RX (GPIO3) --+---[ Resistencia 1k ]-+- 2N2222       |
      |                |           (Base)      \      |            |
      |                |                        +--+  |            |
      |                |                     (Emisor) |            |
      |                |                         +--> | LED        |
      |                |                              |            |
      |          GND --+----------------------------> | GND        |
      +----------------+                              +------------+

       * Para mayor brillo en la pantalla se puede conectar el Colector a VIN (5V).
         El pin LED del módulo ILI9341 integra una pequeña resistencia interna.

       Pinout Transistor 2N2222 / 2N3904 (TO-92):
           +---------------+
           |   ( ) ( ) ( ) |  (Parte plana con serigrafía de frente)
           +---------------+
             1     2     3
             E     B     C
         (Emisor) (Base) (Colector)
```

### Conexión del Parlante de 8Ω (Amplificador con Transistor NPN 2N2222)

Para reproducir melodías polifónicas estilo chiptune (Marcha Imperial de Star Wars o Super Mario Bros) con frecuencias reales, se conecta un parlante dinámico de **8Ω (0.5W a 2W)** utilizando un transistor NPN para proteger el pin del microcontrolador:

```text
                    VIN (5V)* del NodeMCU
                       |
                  [ Resistencia 47 ohm ]  (Limita corriente y fija volumen)
                       |
                 (+) [ PARLANTE 8 ohm 0.5W/2W ] (-)
                               |
                               |  (Pata 3: Colector)
                          +----+
                          |  /
    D0 (GPIO16) --[ 1k ]-( Pata 2: Base )   Transistor NPN 2N2222 / 2N3904
     del NodeMCU          |  \
                          +----+
                               |  (Pata 1: Emisor)
                              GND (Común)

    * Para volumen suave conectar a 3V3. Para mayor volumen conectar a VIN (5V).
```

> [!WARNING]
> **Nunca conectar un parlante de 8Ω directamente a un pin GPIO del ESP8266.** La demanda de corriente (~412 mA) superaría con creces el límite máximo de 12 mA del GPIO, dañando permanentemente el microcontrolador. Utiliza siempre la etapa con transistor 2N2222 y resistencia limitadora.

> [!NOTE]
> Los pines táctiles (`T_CLK`, `T_CS`, `T_DIN`, `T_DO`, `T_IRQ`) y el lector de tarjeta micro-SD (`SD_CS`, etc.) no son necesarios para este proyecto y pueden permanecer desconectados.

### Funciones de los Botones

| Botón | Pantalla Activa | Pulsación Corta (Click) | Pulsación Larga (> 1 seg) |
| :--- | :--- | :--- | :--- |
| **Botón 1 (Prev / D1)** | Cualquiera | Retroceder página ($4 \rightarrow 3 \rightarrow 2 \rightarrow 1 \rightarrow 4$) | - |
| **Botón 2 (Next / D2)** | Cualquiera | Avanzar página ($1 \rightarrow 2 \rightarrow 3 \rightarrow 4 \rightarrow 1$) | - |
| **Botón 3 (Action / D6)** | Al enchufar / encender | - | **Activar Modo Portal AP (Configuración Wi-Fi)** |
| **Botón 3 (Action / D6)** | Página 1 (Diario) | **Apagar / Encender pantalla** (Sleep Toggle) | - |
| **Botón 3 (Action / D6)** | Página 2 (Horario) | **Apagar / Encender pantalla** (Sleep Toggle) | - |
| **Botón 3 (Action / D6)** | Página 3 (Dashboard Central) | **Apagar / Encender pantalla** (Sleep Toggle) | - |
| **Botón 3 (Action / D6)** | Página 4 (Timer - Ajuste) | Sumar tiempo (**+5 min**) | **Iniciar conteo regresivo** |
| **Botón 3 (Action / D6)** | Página 4 (Timer - Conteo) | **Detener / Cancelar** conteo | - |
| **Cualquier Botón** | Alarma sonando | **Silenciar alarma del buzzer** | Silenciar alarma del buzzer |
| **Cualquier Botón** | Pantalla en reposo (Sleep) | **Encender pantalla / Despertar** | Encender pantalla / Despertar |

---

## 🌐 Servidor Web y API REST

El dispositivo expone un servidor HTTP nativo en el puerto 80 que permite monitorear y controlar el reloj desde cualquier navegador web o sistema domótico (Home Assistant, scripts, etc.):

- **Panel de Control Visual:** `http://<IP-DEL-RELOJ>/`
- **Configurador de Wi-Fi y Ciudad:** `http://<IP-DEL-RELOJ>/setup` (o `http://192.168.4.1/` en Modo Portal AP).

---

### Endpoints de la API REST

Todos los endpoints retornan y aceptan datos estándar en formato JSON o parámetros por URL:

#### 1. Obtener Estado en Tiempo Real
* **Método:** `GET /api/status`
* **Descripción:** Retorna la información meteorológica actual, estado de la pantalla, estado del temporizador y ajustes activos.
* **Respuesta de Ejemplo:**
  ```json
  {
    "time": "14:35",
    "temp": 18.5,
    "humidity": 55,
    "condition": "Despejado",
    "page": 1,
    "sleeping": false,
    "rssi": -58,
    "alarming": false,
    "melody": 1,
    "timer": {
      "remaining": 900,
      "total": 900,
      "state": 0
    },
    "config": {
      "ssid": "Mi_Red_WiFi",
      "location": "Santiago",
      "lat": "-33.4428",
      "lon": "-70.6542",
      "melody": 1,
      "page": 1
    }
  }
  ```
  > `timer.state`: `0` = Inactivo / Listo, `1` = En Conteo, `2` = ¡Alarma Sonando!

---

#### 2. Cambiar Página en Pantalla
* **Método:** `POST /api/page?page={1..4}`
* **Parámetros:**
  * `page=1`: Página 1 (Pronóstico Diario a 2 días)
  * `page=2`: Página 2 (Pronóstico Horario)
  * `page=3`: Página 3 (Dashboard Central con Reloj Gigante y Santoral)
  * `page=4`: Página 4 (Temporizador)
* **Ejemplo (curl):**
  ```bash
  curl -X POST "http://192.168.1.50/api/page?page=3"
  ```

---

#### 3. Control de Pantalla (Modo Reposo / Sleep)
* **Método:** `POST /api/sleep?action=toggle`
* **Descripción:** Alterna entre apagar la pantalla (retroiluminación y píxeles) o encenderla y despertarla.
* **Ejemplo (curl):**
  ```bash
  curl -X POST "http://192.168.1.50/api/sleep?action=toggle"
  ```

---

#### 4. Control del Temporizador
* **Método:** `POST /api/timer`
* **Parámetros soportados:**
  * **Iniciar conteo:** `?action=start&hours={H}&minutes={M}` o `?action=start&seconds={S}`
  * **Ajustar sin iniciar:** `?action=set&hours={H}&minutes={M}`
  * **Detener / Reiniciar:** `?action=stop`
* **Ejemplos (curl):**
  ```bash
  # Iniciar un temporizador de 1 hora y 30 minutos
  curl -X POST "http://192.168.1.50/api/timer?action=start&hours=1&minutes=30"

  # Iniciar un temporizador rápido de 15 minutos
  curl -X POST "http://192.168.1.50/api/timer?action=start&minutes=15"

  # Detener o resetear el conteo activo
  curl -X POST "http://192.168.1.50/api/timer?action=stop"
  ```

---

#### 5. Control de Alarma y Melodías
* **Método:** `POST /api/alarm`
* **Parámetros soportados:**
  * **Probar alarma acústica:** `?action=test` (reproduce la melodía por 8 segundos)
  * **Silenciar alarma:** `?action=stop` (detiene el sonido inmediatamente)
  * **Cambiar melodía predeterminada:** `?action=set&melody={1..13}`
    * `1`: Star Wars (La Marcha Imperial)
    * `2`: Super Mario Bros (Overworld Theme)
    * `3`: The Legend of Zelda (Main Theme Overworld)
    * `4`: Tetris (Korobeiniki - Type A)
    * `5`: Pac-Man (Intro Arcade)
    * `6`: Nokia Tune (Gran Vals de Tárrega)
    * `7`: Star Wars: Cantina Band
    * `8`: Doom (E1M1 / At Doom's Gate)
    * `9`: Alarma Digital (Casio Beep)
    * `10`: Volver al Futuro (Back to the Future - Tema Principal)
    * `11`: Los Simpson (The Simpsons Theme - Danny Elfman)
    * `12`: Sonic the Hedgehog (Green Hill Zone)
    * `13`: Alien (Tema Principal - Jerry Goldsmith)
* **Ejemplos (curl):**
  ```bash
  # Seleccionar Super Mario Bros como melodía de alarma
  curl -X POST "http://192.168.1.50/api/alarm?action=set&melody=2"

  # Probar la melodía del buzzer/parlante
  curl -X POST "http://192.168.1.50/api/alarm?action=test"
  ```

---

#### 6. Escaneo de Redes Wi-Fi
* **Método:** `GET /scan`
* **Descripción:** Escanea las redes 2.4 GHz cercanas y retorna un arreglo JSON con el SSID y potencia de señal (RSSI).
* **Respuesta de Ejemplo:**
  ```json
  [
    { "ssid": "Mi_Red_Casa", "rssi": -52, "enc": 1 },
    { "ssid": "Red_Vecino", "rssi": -81, "enc": 1 }
  ]
  ```

---

#### 7. Guardar Configuración Persistente
* **Método:** `POST /save`
* **Descripción:** Recibe los datos del formulario (`ssid`, `password`, `location`, `lat`, `lon`, `melody`, `page`), los escribe en `/config.json` (LittleFS) y reinicia el ESP8266 automáticamente.

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
│   ├── settings_manager.h      # Modelo y persistencia de configuración en LittleFS
│   ├── web_server_service.h    # Servidor web, API REST y portal cautivo (SoftAP)
│   ├── time_service.h          # Declaración de funciones NTP y calendario en español
│   ├── santoral.h              # Calendario santoral católico en español para Página 3
│   ├── weather_service.h       # Modelo de datos meteorológicos y tipos de iconos
│   ├── display_ui.h            # Controladores del display TFT, layout y temporizador
│   ├── input_controller.h      # Controlador no bloqueante de botones (antirrebote y long press)
│   └── buzzer_controller.h     # Controlador no bloqueante del buzzer y patrones de alarma
├── src/
│   ├── main.cpp                # Setup, bucle de ejecución, navegación y temporizador
│   ├── settings_manager.cpp    # Carga y almacenamiento no volátil de ajustes en Flash
│   ├── web_server_service.cpp  # Implementación de endpoints REST y páginas en PROGMEM
│   ├── time_service.cpp        # Manejo de SNTP nativo y formateo en español
│   ├── santoral.cpp            # Tabla de nombres del santoral católico
│   ├── weather_service.cpp     # Consulta HTTP a Open-Meteo y deserialización JSON
│   ├── display_ui.cpp          # Renderizado gráfico de reloj, clima y temporizador
│   ├── input_controller.cpp    # Detección de clics y pulsaciones sostenidas
│   └── buzzer_controller.cpp   # Patrones acústicos para alarmas y confirmaciones
├── test/
│   └── test_app_logic.cpp      # Pruebas unitarias nativas (Unity framework)
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
