#include "web_server_service.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "config.h"
#include "buzzer_controller.h"

static const char SETUP_PAGE_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Configuración - Weather Clock</title>
<style>
:root{--bg:#0f172a;--card:#1e293b;--text:#f8fafc;--text-muted:#94a3b8;--accent:#38bdf8;--accent-hover:#0ea5e9;--success:#22c55e;--border:#334155}
*{box-sizing:border-box;margin:0;padding:0;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Helvetica,Arial,sans-serif}
body{background:var(--bg);color:var(--text);display:flex;justify-content:center;padding:16px;min-height:100vh}
.container{width:100%;max-width:480px}
.header{text-align:center;margin-bottom:20px}
.header h1{font-size:1.5rem;color:var(--accent);margin-bottom:4px}
.header p{font-size:0.875rem;color:var(--text-muted)}
.card{background:var(--card);border:1px solid var(--border);border-radius:12px;padding:20px;margin-bottom:16px;box-shadow:0 4px 6px -1px rgba(0,0,0,0.2)}
.card h2{font-size:1.1rem;margin-bottom:14px;color:var(--text);display:flex;align-items:center;gap:8px}
.form-group{margin-bottom:14px}
label{display:block;font-size:0.825rem;font-weight:600;margin-bottom:6px;color:var(--text-muted)}
input,select{width:100%;padding:10px 12px;background:#0f172a;border:1px solid var(--border);border-radius:8px;color:var(--text);font-size:0.95rem}
input:focus,select:focus{outline:none;border-color:var(--accent)}
.btn{display:inline-block;width:100%;padding:12px;border:none;border-radius:8px;font-size:1rem;font-weight:600;cursor:pointer;transition:background 0.2s;text-align:center;text-decoration:none}
.btn-primary{background:var(--accent);color:#0f172a}
.btn-primary:hover{background:var(--accent-hover)}
.btn-secondary{background:#334155;color:var(--text);margin-top:8px;font-size:0.875rem;padding:8px}
.btn-secondary:hover{background:#475569}
.btn-scan{padding:8px 12px;font-size:0.8rem;background:#334155;color:var(--text);border-radius:6px;border:none;cursor:pointer;margin-bottom:8px}
.row{display:flex;gap:10px}
.row .form-group{flex:1}
.badge{background:#334155;color:var(--accent);padding:2px 8px;border-radius:12px;font-size:0.75rem}
</style>
</head>
<body>
<div class="container">
  <div class="header">
    <h1>Weather Clock ESP8266</h1>
    <p>Ajustes de Wi-Fi, Ubicación y Sistema</p>
  </div>
  <form action="/save" method="POST">
    <div class="card">
      <h2>Conexión Wi-Fi (2.4 GHz)</h2>
      <div class="form-group">
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:6px">
          <label style="margin:0">Red Wi-Fi (SSID)</label>
          <button type="button" class="btn-scan" onclick="scanWifi()">Buscar redes</button>
        </div>
        <select id="wifi_select" style="display:none;margin-bottom:8px" onchange="selectNetwork(this.value)">
          <option value="">-- Selecciona una red --</option>
        </select>
        <input type="text" id="ssid" name="ssid" required placeholder="Nombre de red Wi-Fi">
      </div>
      <div class="form-group">
        <label>Contraseña Wi-Fi</label>
        <input type="password" id="password" name="password" placeholder="Contraseña de la red">
      </div>
    </div>

    <div class="card">
      <h2>Ubicación y Clima</h2>
      <div class="form-group">
        <label>Nombre de la Ciudad</label>
        <input type="text" id="location" name="location" required placeholder="Ej: Santiago">
      </div>
      <div class="row">
        <div class="form-group">
          <label>Latitud</label>
          <input type="text" id="lat" name="lat" required placeholder="-33.4428">
        </div>
        <div class="form-group">
          <label>Longitud</label>
          <input type="text" id="lon" name="lon" required placeholder="-70.6542">
        </div>
      </div>
    </div>

    <div class="card">
      <h2>Preferencias</h2>
      <div class="form-group">
        <label>Melodía de Alarma</label>
        <select id="melody" name="melody">
          <option value="1">Star Wars (Marcha Imperial)</option>
          <option value="2">Super Mario Bros (Overworld)</option>
          <option value="3">The Legend of Zelda (Main Theme)</option>
          <option value="4">Tetris (Korobeiniki)</option>
          <option value="5">Pac-Man (Intro Arcade)</option>
          <option value="6">Nokia Tune (Gran Vals)</option>
          <option value="7">Star Wars: Cantina Band</option>
          <option value="8">Doom (E1M1 At Doom's Gate)</option>
          <option value="9">Alarma Digital (Casio Beep)</option>
          <option value="10">Volver al Futuro (Back to the Future)</option>
          <option value="11">Los Simpson (The Simpsons Theme)</option>
          <option value="12">Sonic the Hedgehog (Green Hill Zone)</option>
          <option value="13">Alien (Tema Principal - Jerry Goldsmith)</option>
        </select>
      </div>
      <div class="form-group">
        <label>Página al Iniciar</label>
        <select id="page" name="page">
          <option value="1">Página 1: Pronóstico Diario</option>
          <option value="2">Página 2: Pronóstico por Horas</option>
          <option value="3">Página 3: Dashboard Reloj Gigante</option>
          <option value="4">Página 4: Temporizador</option>
          <option value="5">Página 5: Despertador</option>
        </select>
      </div>
    </div>

    <div class="card">
      <h2>Páginas Habilitadas en Pantalla</h2>
      <div style="display:flex;flex-direction:column;gap:8px;font-size:0.85rem">
        <label style="display:flex;align-items:center;gap:8px;cursor:pointer">
          <input type="checkbox" id="p1" name="p1" value="1" style="width:auto"> Página 1: Clima Diario (2 días)
        </label>
        <label style="display:flex;align-items:center;gap:8px;cursor:pointer">
          <input type="checkbox" id="p2" name="p2" value="1" style="width:auto"> Página 2: Pronóstico por Horas
        </label>
        <label style="display:flex;align-items:center;gap:8px;cursor:pointer">
          <input type="checkbox" id="p3" name="p3" value="1" style="width:auto"> Página 3: Dashboard Reloj Gigante + Santoral
        </label>
        <label style="display:flex;align-items:center;gap:8px;cursor:pointer">
          <input type="checkbox" id="p4" name="p4" value="1" style="width:auto"> Página 4: Temporizador
        </label>
        <label style="display:flex;align-items:center;gap:8px;cursor:pointer">
          <input type="checkbox" id="p5" name="p5" value="1" style="width:auto"> Página 5: Despertador
        </label>
      </div>
    </div>

    <button type="submit" class="btn btn-primary">Guardar y Reiniciar Reloj</button>
    <a href="/" class="btn btn-secondary">Volver al Panel</a>
  </form>
</div>

<script>
function selectNetwork(val) {
  if (val) document.getElementById('ssid').value = val;
}
function scanWifi() {
  const sel = document.getElementById('wifi_select');
  sel.innerHTML = '<option>Buscando redes 2.4 GHz...</option>';
  sel.style.display = 'block';
  fetch('/scan')
    .then(r => r.json())
    .then(nets => {
      sel.innerHTML = '<option value="">-- Redes encontradas --</option>';
      nets.forEach(n => {
        const opt = document.createElement('option');
        opt.value = n.ssid;
        opt.textContent = `${n.ssid} (${n.rssi} dBm)`;
        sel.appendChild(opt);
      });
    })
    .catch(() => {
      sel.innerHTML = '<option>Error al escanear</option>';
    });
}
fetch('/api/status')
  .then(r => r.json())
  .then(d => {
    if (d.config) {
      if (d.config.ssid) document.getElementById('ssid').value = d.config.ssid;
      if (d.config.location) document.getElementById('location').value = d.config.location;
      if (d.config.lat) document.getElementById('lat').value = d.config.lat;
      if (d.config.lon) document.getElementById('lon').value = d.config.lon;
      if (d.config.melody) document.getElementById('melody').value = d.config.melody;
      if (d.config.page) document.getElementById('page').value = d.config.page;
    }
    if (d.enabled_pages) {
      for (let i = 1; i <= 5; i++) {
        const el = document.getElementById('p' + i);
        if (el) el.checked = !!d.enabled_pages[String(i)];
      }
    } else {
      for (let i = 1; i <= 5; i++) {
        const el = document.getElementById('p' + i);
        if (el) el.checked = true;
      }
    }
  }).catch(()=>{});
  }).catch(()=>{});
</script>
</body>
</html>)rawliteral";

static const char CONTROL_PAGE_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Panel de Control - Weather Clock</title>
<style>
:root{--bg:#0f172a;--card:#1e293b;--text:#f8fafc;--text-muted:#94a3b8;--accent:#38bdf8;--accent-hover:#0ea5e9;--success:#22c55e;--danger:#ef4444;--warning:#f59e0b;--border:#334155}
*{box-sizing:border-box;margin:0;padding:0;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Helvetica,Arial,sans-serif}
body{background:var(--bg);color:var(--text);display:flex;justify-content:center;padding:16px;min-height:100vh}
.container{width:100%;max-width:540px}
.header{display:flex;justify-content:space-between;align-items:center;margin-bottom:16px}
.header h1{font-size:1.3rem;color:var(--accent)}
.link-setup{color:var(--text-muted);font-size:0.85rem;text-decoration:none;border:1px solid var(--border);padding:6px 10px;border-radius:6px;transition:0.2s}
.link-setup:hover{border-color:var(--accent);color:var(--text)}
.card{background:var(--card);border:1px solid var(--border);border-radius:12px;padding:16px;margin-bottom:14px;box-shadow:0 4px 6px -1px rgba(0,0,0,0.2)}
.card-title{font-size:1rem;font-weight:600;margin-bottom:12px;display:flex;justify-content:space-between;align-items:center}
.badge{background:#334155;color:var(--accent);padding:2px 8px;border-radius:12px;font-size:0.75rem}
.grid-2{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.grid-4{display:grid;grid-template-columns:repeat(4,1fr);gap:8px}
.grid-5{display:grid;grid-template-columns:repeat(5,1fr);gap:6px}
.stat-box{background:#0f172a;padding:10px;border-radius:8px;border:1px solid var(--border);text-align:center}
.stat-val{font-size:1.2rem;font-weight:700;color:var(--accent);margin-top:2px}
.stat-lbl{font-size:0.75rem;color:var(--text-muted);text-transform:uppercase}
.btn{width:100%;padding:10px;border:none;border-radius:8px;font-size:0.9rem;font-weight:600;cursor:pointer;transition:all 0.15s;text-align:center}
.btn-primary{background:var(--accent);color:#0f172a}
.btn-primary:hover{background:var(--accent-hover)}
.btn-success{background:var(--success);color:#0f172a}
.btn-danger{background:var(--danger);color:#fff}
.btn-warning{background:var(--warning);color:#0f172a}
.btn-outline{background:#0f172a;border:1px solid var(--border);color:var(--text)}
.btn-outline.active{border-color:var(--accent);background:rgba(56,189,248,0.15);color:var(--accent)}
.btn-outline:hover{border-color:var(--accent)}
.timer-display{text-align:center;padding:14px;background:#0f172a;border-radius:8px;border:1px solid var(--border);margin-bottom:10px}
.timer-digits{font-size:2.4rem;font-weight:700;letter-spacing:2px;font-family:monospace;color:var(--text)}
.timer-state{font-size:0.8rem;color:var(--text-muted);margin-top:2px}
.form-select{width:100%;padding:8px 10px;background:#0f172a;border:1px solid var(--border);border-radius:8px;color:var(--text);margin-bottom:10px;font-size:0.9rem}
</style>
</head>
<body>
<div class="container">
  <div class="header">
    <h1>Weather Clock</h1>
    <a href="/setup" class="link-setup">Configuración Wi-Fi</a>
  </div>

  <div class="card">
    <div class="card-title">
      <span>Estado en Vivo</span>
      <span id="ip_badge" class="badge">Conectado</span>
    </div>
    <div class="grid-4">
      <div class="stat-box">
        <div class="stat-lbl">Hora</div>
        <div id="stat_time" class="stat-val">--:--</div>
      </div>
      <div class="stat-box">
        <div class="stat-lbl">Temp</div>
        <div id="stat_temp" class="stat-val">--.-&deg;C</div>
      </div>
      <div class="stat-box">
        <div class="stat-lbl">Humedad</div>
        <div id="stat_hum" class="stat-val">--%</div>
      </div>
      <div class="stat-box">
        <div class="stat-lbl">Señal</div>
        <div id="stat_rssi" class="stat-val">-- dBm</div>
      </div>
    </div>
    <div style="margin-top:10px;font-size:0.85rem;color:var(--text-muted);text-align:center" id="stat_cond">
      Actualizando datos...
    </div>
  </div>

  <div class="card">
    <div class="card-title">
      <span>Control de Pantalla TFT</span>
      <span id="sleep_badge" class="badge">Encendida</span>
    </div>
    <div class="grid-5" style="margin-bottom:10px" id="page_buttons_grid">
      <button class="btn btn-outline" id="btn_page1" onclick="setPage(1)">P1: Diario</button>
      <button class="btn btn-outline" id="btn_page2" onclick="setPage(2)">P2: Horas</button>
      <button class="btn btn-outline" id="btn_page3" onclick="setPage(3)">P3: Reloj</button>
      <button class="btn btn-outline" id="btn_page4" onclick="setPage(4)">P4: Timer</button>
      <button class="btn btn-outline" id="btn_page5" onclick="setPage(5)">P5: Alarma</button>
    </div>
    <button class="btn btn-outline" id="btn_sleep" onclick="toggleSleep()">Apagar Pantalla (Sleep)</button>
  </div>

  <div class="card">
    <div class="card-title">
      <span>Despertador / Alarmas Diarias</span>
      <span id="alarms_count_badge" class="badge">10 Alarmas</span>
    </div>

    <!-- Lista interactiva de alarmas -->
    <div id="alarms_list_container" style="display:flex;flex-direction:column;gap:6px">
      <!-- Se carga dinámicamente -->
    </div>

    <!-- Editor de alarma -->
    <div id="alarm_editor" style="display:none;background:#0f172a;border:1px solid var(--border);border-radius:8px;padding:12px;margin-top:10px">
      <div style="font-size:0.9rem;font-weight:600;color:var(--accent);margin-bottom:8px" id="editor_title">Editar Alarma</div>
      <input type="hidden" id="edit_alarm_id" value="1">
      <div class="row" style="margin-bottom:8px">
        <div style="flex:1">
          <label style="font-size:0.75rem;color:var(--text-muted);display:block;margin-bottom:4px">Hora (00..23)</label>
          <input type="number" id="edit_alarm_hour" min="0" max="23" style="width:100%;padding:8px;background:var(--card);border:1px solid var(--border);border-radius:6px;color:var(--text);font-size:1.1rem;font-weight:700">
        </div>
        <div style="flex:1">
          <label style="font-size:0.75rem;color:var(--text-muted);display:block;margin-bottom:4px">Minuto (00..59)</label>
          <input type="number" id="edit_alarm_min" min="0" max="59" style="width:100%;padding:8px;background:var(--card);border:1px solid var(--border);border-radius:6px;color:var(--text);font-size:1.1rem;font-weight:700">
        </div>
      </div>
      <div style="margin-bottom:8px">
        <label style="font-size:0.75rem;color:var(--text-muted);display:block;margin-bottom:4px">Etiqueta / Nombre</label>
        <input type="text" id="edit_alarm_label" maxlength="16" placeholder="Ej: Trabajo" style="width:100%;padding:8px;background:var(--card);border:1px solid var(--border);border-radius:6px;color:var(--text);font-size:0.9rem">
      </div>
      <div style="margin-bottom:8px">
        <label style="font-size:0.75rem;color:var(--text-muted);display:block;margin-bottom:4px">Melodía para esta Alarma</label>
        <select id="edit_alarm_melody" class="form-select" style="margin-bottom:0">
          <option value="0">Predeterminada del sistema</option>
          <option value="1">Star Wars (Marcha Imperial)</option>
          <option value="2">Super Mario Bros (Overworld)</option>
          <option value="3">The Legend of Zelda (Main Theme)</option>
          <option value="4">Tetris (Korobeiniki)</option>
          <option value="5">Pac-Man (Intro Arcade)</option>
          <option value="6">Nokia Tune (Gran Vals)</option>
          <option value="7">Star Wars: Cantina Band</option>
          <option value="8">Doom (E1M1 At Doom's Gate)</option>
          <option value="9">Alarma Digital (Casio Beep)</option>
          <option value="10">Volver al Futuro (Back to the Future)</option>
          <option value="11">Los Simpson (The Simpsons Theme)</option>
          <option value="12">Sonic the Hedgehog (Green Hill Zone)</option>
          <option value="13">Alien (Tema Principal - Jerry Goldsmith)</option>
        </select>
      </div>
      <div style="margin-bottom:10px">
        <label style="font-size:0.75rem;color:var(--text-muted);display:block;margin-bottom:4px">Días de repetición</label>
        <div style="display:flex;gap:4px;margin-bottom:8px">
          <button type="button" class="btn btn-outline" style="padding:4px;font-size:0.75rem;flex:1" onclick="setEditorDaysPreset('all')">Todos</button>
          <button type="button" class="btn btn-outline" style="padding:4px;font-size:0.75rem;flex:1" onclick="setEditorDaysPreset('weekdays')">Lun-Vie</button>
          <button type="button" class="btn btn-outline" style="padding:4px;font-size:0.75rem;flex:1" onclick="setEditorDaysPreset('weekend')">FinSem</button>
        </div>
        <div style="display:flex;justify-content:space-around;gap:2px;background:var(--card);padding:6px;border-radius:6px;border:1px solid var(--border)">
          <label style="font-size:0.75rem;display:flex;flex-direction:column;align-items:center;cursor:pointer"><input type="checkbox" id="d_1"><span>Lun</span></label>
          <label style="font-size:0.75rem;display:flex;flex-direction:column;align-items:center;cursor:pointer"><input type="checkbox" id="d_2"><span>Mar</span></label>
          <label style="font-size:0.75rem;display:flex;flex-direction:column;align-items:center;cursor:pointer"><input type="checkbox" id="d_3"><span>Mie</span></label>
          <label style="font-size:0.75rem;display:flex;flex-direction:column;align-items:center;cursor:pointer"><input type="checkbox" id="d_4"><span>Jue</span></label>
          <label style="font-size:0.75rem;display:flex;flex-direction:column;align-items:center;cursor:pointer"><input type="checkbox" id="d_5"><span>Vie</span></label>
          <label style="font-size:0.75rem;display:flex;flex-direction:column;align-items:center;cursor:pointer"><input type="checkbox" id="d_6"><span>Sab</span></label>
          <label style="font-size:0.75rem;display:flex;flex-direction:column;align-items:center;cursor:pointer"><input type="checkbox" id="d_0"><span>Dom</span></label>
        </div>
      </div>
      <div class="grid-2">
        <button type="button" class="btn btn-success" onclick="saveAlarmFromEditor()">Guardar Alarma</button>
        <button type="button" class="btn btn-outline" onclick="closeAlarmEditor()">Cancelar</button>
      </div>
    </div>
  </div>

  <div class="card">
    <div class="card-title">
      <span>Temporizador</span>
      <span id="timer_badge" class="badge">Inactivo</span>
    </div>
    <div class="timer-display">
      <div id="timer_val" class="timer-digits">05:00</div>
      <div id="timer_status" class="timer-state">Listo para iniciar</div>
    </div>
    <div style="display:flex;gap:10px;margin-bottom:8px">
      <div style="flex:1">
        <label style="font-size:0.75rem;color:var(--text-muted);display:block;margin-bottom:4px">Horas</label>
        <select id="timer_hrs" class="form-select">
          <option value="0">0 horas</option>
          <option value="1">1 hora</option>
          <option value="2">2 horas</option>
          <option value="3">3 horas</option>
          <option value="4">4 horas</option>
          <option value="5">5 horas</option>
          <option value="6">6 horas</option>
          <option value="8">8 horas</option>
          <option value="12">12 horas</option>
        </select>
      </div>
      <div style="flex:1">
        <label style="font-size:0.75rem;color:var(--text-muted);display:block;margin-bottom:4px">Minutos</label>
        <select id="timer_mins" class="form-select">
          <option value="1">1 minuto</option>
          <option value="2">2 minutos</option>
          <option value="3">3 minutos</option>
          <option value="5" selected>5 minutos</option>
          <option value="10">10 minutos</option>
          <option value="15">15 minutos</option>
          <option value="20">20 minutos</option>
          <option value="25">25 minutos</option>
          <option value="30">30 minutos</option>
          <option value="35">35 minutos</option>
          <option value="40">40 minutos</option>
          <option value="45">45 minutos</option>
          <option value="50">50 minutos</option>
          <option value="55">55 minutos</option>
        </select>
      </div>
    </div>
    <div style="display:flex;gap:6px;margin-bottom:10px;flex-wrap:wrap">
      <button type="button" class="btn btn-outline" style="flex:1;padding:6px;font-size:0.8rem" onclick="setTimerPreset(0,5)">5m</button>
      <button type="button" class="btn btn-outline" style="flex:1;padding:6px;font-size:0.8rem" onclick="setTimerPreset(0,15)">15m</button>
      <button type="button" class="btn btn-outline" style="flex:1;padding:6px;font-size:0.8rem" onclick="setTimerPreset(0,30)">30m</button>
      <button type="button" class="btn btn-outline" style="flex:1;padding:6px;font-size:0.8rem" onclick="setTimerPreset(1,0)">1h</button>
      <button type="button" class="btn btn-outline" style="flex:1;padding:6px;font-size:0.8rem" onclick="setTimerPreset(1,30)">1h 30m</button>
      <button type="button" class="btn btn-outline" style="flex:1;padding:6px;font-size:0.8rem" onclick="setTimerPreset(2,0)">2h</button>
    </div>
    <div class="grid-2">
      <button class="btn btn-success" onclick="startTimer()">Iniciar</button>
      <button class="btn btn-danger" onclick="stopTimer()">Detener / Reset</button>
    </div>
  </div>

  <div class="card">
    <div class="card-title">
      <span>Alarma y Audio</span>
      <span id="alarm_badge" class="badge">Silencio</span>
    </div>
    <select id="melody_sel" class="form-select" onchange="changeMelody(this.value)">
      <option value="1">Star Wars (Marcha Imperial)</option>
      <option value="2">Super Mario Bros (Overworld)</option>
      <option value="3">The Legend of Zelda (Main Theme)</option>
      <option value="4">Tetris (Korobeiniki)</option>
      <option value="5">Pac-Man (Intro Arcade)</option>
      <option value="6">Nokia Tune (Gran Vals)</option>
      <option value="7">Star Wars: Cantina Band</option>
      <option value="8">Doom (E1M1 At Doom's Gate)</option>
      <option value="9">Alarma Digital (Casio Beep)</option>
      <option value="10">Volver al Futuro (Back to the Future)</option>
      <option value="11">Los Simpson (The Simpsons Theme)</option>
      <option value="12">Sonic the Hedgehog (Green Hill Zone)</option>
      <option value="13">Alien (Tema Principal - Jerry Goldsmith)</option>
    </select>
    <div class="grid-2">
      <button class="btn btn-warning" onclick="testAlarm()">Probar Alarma</button>
      <button class="btn btn-outline" onclick="stopAlarm()">Silenciar</button>
    </div>
  </div>
</div>

<script>
let curPage = 1;
let isSleeping = false;

function updateStatus() {
  fetch('/api/status')
    .then(r => r.json())
    .then(d => {
      document.getElementById('stat_time').textContent = d.time || '--:--';
      document.getElementById('stat_temp').textContent = (d.temp !== undefined ? d.temp.toFixed(1) : '--') + ' C';
      document.getElementById('stat_hum').textContent = (d.humidity || '--') + '%';
      document.getElementById('stat_rssi').textContent = (d.rssi || '--') + ' dBm';
      document.getElementById('stat_cond').textContent = d.condition || 'Condiciones normales';

      curPage = d.page;
      isSleeping = d.sleeping;

      for (let i = 1; i <= 5; i++) {
        const btn = document.getElementById('btn_page' + i);
        if (btn) {
          btn.classList.toggle('active', curPage === i);
          if (d.enabled_pages && d.enabled_pages[String(i)] !== undefined) {
            btn.style.display = d.enabled_pages[String(i)] ? 'inline-block' : 'none';
          }
        }
      }

      const slpBtn = document.getElementById('btn_sleep');
      const slpBdg = document.getElementById('sleep_badge');
      if (isSleeping) {
        slpBtn.textContent = 'Encender Pantalla (Despertar)';
        slpBdg.textContent = 'En Reposo';
        slpBdg.style.color = '#f59e0b';
      } else {
        slpBtn.textContent = 'Apagar Pantalla (Sleep)';
        slpBdg.textContent = 'Encendida';
        slpBdg.style.color = '#38bdf8';
      }

      if (d.timer) {
        const h = Math.floor(d.timer.remaining / 3600);
        const m = Math.floor((d.timer.remaining % 3600) / 60);
        const s = d.timer.remaining % 60;
        if (d.timer.remaining >= 3600 || d.timer.total >= 3600) {
          document.getElementById('timer_val').textContent = 
            String(h).padStart(2,'0') + ':' + String(m).padStart(2,'0') + ':' + String(s).padStart(2,'0');
        } else {
          document.getElementById('timer_val').textContent = 
            String(m).padStart(2,'0') + ':' + String(s).padStart(2,'0');
        }

        const tb = document.getElementById('timer_badge');
        const ts = document.getElementById('timer_status');
        if (d.timer.state === 1) {
          tb.textContent = 'En Ejecucion';
          tb.style.color = '#22c55e';
          ts.textContent = 'Contando tiempo en segundo plano';
        } else if (d.timer.state === 2) {
          tb.textContent = '¡ALARMA!';
          tb.style.color = '#ef4444';
          ts.textContent = '¡Tiempo cumplido!';
        } else {
          tb.textContent = 'Inactivo';
          tb.style.color = '#94a3b8';
          ts.textContent = 'Listo para iniciar';
        }
      }

      if (d.melody) {
        document.getElementById('melody_sel').value = d.melody;
      }
      const ab = document.getElementById('alarm_badge');
      if (d.alarming) {
        ab.textContent = 'Sonando';
        ab.style.color = '#ef4444';
      } else {
        ab.textContent = 'Silencio';
        ab.style.color = '#94a3b8';
      }
    }).catch(()=>{});
}

let cachedAlarms = [];
const melodyNames = [
  "Predeterminada",
  "Star Wars (Marcha)",
  "Super Mario Bros",
  "Zelda (Theme)",
  "Tetris",
  "Pac-Man",
  "Nokia Tune",
  "Cantina Band",
  "Doom (E1M1)",
  "Casio Beep",
  "Volver al Futuro",
  "Los Simpson",
  "Sonic",
  "Alien"
];

function loadAlarms() {
  fetch('/api/alarms')
    .then(r => r.json())
    .then(d => {
      if (!d.alarms) return;
      cachedAlarms = d.alarms;
      renderAlarmsList();
    }).catch(()=>{});
}

function renderAlarmsList() {
  const container = document.getElementById('alarms_list_container');
  if (!container) return;
  container.innerHTML = '';

  cachedAlarms.forEach(a => {
    const row = document.createElement('div');
    row.style = 'display:flex;align-items:center;justify-content:space-between;background:#0f172a;padding:8px 10px;border-radius:8px;border:1px solid var(--border)';

    const left = document.createElement('div');
    left.style = 'display:flex;align-items:center;gap:10px';

    const idBadge = document.createElement('span');
    idBadge.style = 'color:var(--text-muted);font-size:0.75rem;font-weight:700;width:24px';
    idBadge.textContent = '[' + a.id + ']';

    const timeEl = document.createElement('span');
    timeEl.style = 'font-size:1.2rem;font-weight:700;letter-spacing:1px;color:' + (a.enabled ? 'var(--text)' : 'var(--text-muted)');
    timeEl.textContent = String(a.hour).padStart(2,'0') + ':' + String(a.minute).padStart(2,'0');

    const info = document.createElement('div');
    info.style = 'display:flex;flex-direction:column;font-size:0.75rem';
    const daysEl = document.createElement('span');
    daysEl.style = 'color:' + (a.enabled ? 'var(--accent)' : 'var(--text-muted)');
    daysEl.textContent = a.days_str || 'Personalizado';
    const labelEl = document.createElement('span');
    labelEl.style = 'color:var(--text-muted)';
    labelEl.textContent = a.label || '';
    const melEl = document.createElement('span');
    melEl.style = 'color:var(--text-muted);font-size:0.7rem';
    melEl.textContent = '🎵 ' + (melodyNames[a.melody || 0] || 'Predeterminada');

    info.appendChild(daysEl);
    if (a.label) info.appendChild(labelEl);
    info.appendChild(melEl);

    left.appendChild(idBadge);
    left.appendChild(timeEl);
    left.appendChild(info);

    const right = document.createElement('div');
    right.style = 'display:flex;align-items:center;gap:6px';

    const btnTgl = document.createElement('button');
    btnTgl.className = 'btn ' + (a.enabled ? 'btn-success' : 'btn-outline');
    btnTgl.style = 'padding:4px 8px;font-size:0.75rem;width:auto';
    btnTgl.textContent = a.enabled ? 'ON' : 'OFF';
    btnTgl.onclick = () => toggleAlarm(a.id);

    const btnEdit = document.createElement('button');
    btnEdit.className = 'btn btn-outline';
    btnEdit.style = 'padding:4px 8px;font-size:0.75rem;width:auto';
    btnEdit.textContent = 'Editar';
    btnEdit.onclick = () => openAlarmEditor(a.id);

    right.appendChild(btnTgl);
    right.appendChild(btnEdit);

    row.appendChild(left);
    row.appendChild(right);
    container.appendChild(row);
  });
}

function toggleAlarm(id) {
  fetch('/api/alarms?action=toggle&id=' + id, { method: 'POST' })
    .then(r => r.json())
    .then(() => loadAlarms());
}

function openAlarmEditor(id) {
  const alarm = cachedAlarms.find(a => a.id === id);
  if (!alarm) return;

  document.getElementById('edit_alarm_id').value = alarm.id;
  document.getElementById('edit_alarm_hour').value = alarm.hour;
  document.getElementById('edit_alarm_min').value = alarm.minute;
  document.getElementById('edit_alarm_label').value = alarm.label || '';
  document.getElementById('edit_alarm_melody').value = alarm.melody || 0;
  document.getElementById('editor_title').textContent = 'Editar Alarma [' + alarm.id + ']';

  for (let i = 0; i < 7; i++) {
    const chk = document.getElementById('d_' + i);
    if (chk) chk.checked = !!(alarm.days & (1 << i));
  }

  document.getElementById('alarm_editor').style.display = 'block';
  document.getElementById('alarm_editor').scrollIntoView({ behavior: 'smooth' });
}

function closeAlarmEditor() {
  document.getElementById('alarm_editor').style.display = 'none';
}

function setEditorDaysPreset(preset) {
  let mask = 0x7F;
  if (preset === 'weekdays') mask = 0x3E; // L M X J V
  if (preset === 'weekend') mask = 0x41;  // S D
  for (let i = 0; i < 7; i++) {
    const chk = document.getElementById('d_' + i);
    if (chk) chk.checked = !!(mask & (1 << i));
  }
}

function saveAlarmFromEditor() {
  const id = document.getElementById('edit_alarm_id').value;
  const h = document.getElementById('edit_alarm_hour').value;
  const m = document.getElementById('edit_alarm_min').value;
  const lbl = encodeURIComponent(document.getElementById('edit_alarm_label').value);
  const mel = document.getElementById('edit_alarm_melody').value;

  let daysMask = 0;
  for (let i = 0; i < 7; i++) {
    const chk = document.getElementById('d_' + i);
    if (chk && chk.checked) daysMask |= (1 << i);
  }

  const existing = cachedAlarms.find(a => a.id == id);
  const en = existing ? (existing.enabled ? 1 : 0) : 1;

  fetch('/api/alarms?action=set&id=' + id + '&enabled=' + en + '&hour=' + h + '&minute=' + m + '&days=' + daysMask + '&melody=' + mel + '&label=' + lbl, { method: 'POST' })
    .then(r => r.json())
    .then(() => {
      closeAlarmEditor();
      loadAlarms();
    });
}

function setPage(p) {
  fetch('/api/page?page=' + p, { method: 'POST' }).then(() => updateStatus());
}
function toggleSleep() {
  fetch('/api/sleep?action=toggle', { method: 'POST' }).then(() => updateStatus());
}
function setTimerPreset(h, m) {
  document.getElementById('timer_hrs').value = h;
  document.getElementById('timer_mins').value = m;
}
function startTimer() {
  const hrs = document.getElementById('timer_hrs').value;
  const mins = document.getElementById('timer_mins').value;
  fetch('/api/timer?action=start&hours=' + hrs + '&minutes=' + mins, { method: 'POST' }).then(() => updateStatus());
}
function stopTimer() {
  fetch('/api/timer?action=stop', { method: 'POST' }).then(() => updateStatus());
}
function changeMelody(val) {
  fetch('/api/alarm?action=set&melody=' + val, { method: 'POST' }).then(() => updateStatus());
}
function testAlarm() {
  fetch('/api/alarm?action=test', { method: 'POST' }).then(() => updateStatus());
}
function stopAlarm() {
  fetch('/api/alarm?action=stop', { method: 'POST' }).then(() => updateStatus());
}

updateStatus();
loadAlarms();
setInterval(updateStatus, 2500);
</script>
</body>
</html>)rawliteral";

static const char SAVED_PAGE_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Guardado - Weather Clock</title>
<style>
body{background:#0f172a;color:#f8fafc;font-family:-apple-system,BlinkMacSystemFont,sans-serif;display:flex;align-items:center;justify-content:center;height:100vh;margin:0;padding:16px;text-align:center}
.card{background:#1e293b;border:1px solid #334155;border-radius:12px;padding:30px;max-width:400px}
h1{color:#22c55e;font-size:1.4rem;margin-bottom:12px}
p{color:#94a3b8;font-size:0.95rem;line-height:1.5;margin-bottom:16px}
.count{font-size:1.8rem;font-weight:700;color:#38bdf8}
</style>
</head>
<body>
<div class="card">
  <h1>Configuracion Guardada</h1>
  <p>Los parametros se almacenaron en la memoria permanente del reloj. El dispositivo se esta reiniciando ahora mismo.</p>
  <div class="count" id="cnt">5</div>
</div>
<script>
let c = 5;
setInterval(() => {
  c--;
  if (c >= 0) document.getElementById('cnt').textContent = c;
  if (c === 0) location.href = '/';
}, 1000);
</script>
</body>
</html>)rawliteral";

WebServerService::WebServerService(SettingsManager& settingsMgr, const WebServerCallbacks& callbacks)
    : _settings(settingsMgr),
      _cb(callbacks),
      _server(80),
      _mode(WEB_MODE_NONE),
      _rebootRequestedTime(0),
      _rebootRequested(false) {
}

void WebServerService::beginPortalAP(const char* apSsid) {
    _mode = WEB_MODE_PORTAL_AP;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSsid);

    IPAddress apIP = WiFi.softAPIP();
    Serial.printf("[Web] SoftAP levantado. SSID: %s, IP: %s\n", apSsid, apIP.toString().c_str());

    // Iniciar servidor DNS en puerto 53 para portal cautivo
    _dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServer.start(53, "*", apIP);

    setupCommonRoutes();
    setupPortalRoutes();
    _server.begin();
    Serial.println(F("[Web] Servidor Web y DNS iniciados en modo Portal AP"));
}

void WebServerService::beginStationServer() {
    _mode = WEB_MODE_STATION;
    setupCommonRoutes();
    setupStationRoutes();
    _server.begin();
    Serial.printf("[Web] Servidor Web iniciado en modo Estacion (IP: %s)\n", WiFi.localIP().toString().c_str());
}

void WebServerService::stop() {
    _server.stop();
    if (_mode == WEB_MODE_PORTAL_AP) {
        _dnsServer.stop();
    }
    _mode = WEB_MODE_NONE;
}

void WebServerService::update() {
    if (_mode == WEB_MODE_PORTAL_AP) {
        _dnsServer.processNextRequest();
    }
    _server.handleClient();

    if (_rebootRequested && (millis() - _rebootRequestedTime >= 1500)) {
        Serial.println(F("[Web] Reiniciando ESP8266 tras guardar cambios..."));
        ESP.restart();
    }
}

bool WebServerService::isCaptivePortalRequest() {
    if (_mode != WEB_MODE_PORTAL_AP) return false;
    String host = _server.hostHeader();
    IPAddress apIP = WiFi.softAPIP();
    if (host.length() == 0 || host.indexOf(apIP.toString()) >= 0) {
        return false;
    }
    return true;
}

void WebServerService::handleCaptivePortalRedirect() {
    _server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
    _server.send(302, "text/plain", "");
}

void WebServerService::setupCommonRoutes() {
    _server.on("/scan", HTTP_GET, std::bind(&WebServerService::handleScanWifi, this));
    _server.on("/save", HTTP_POST, std::bind(&WebServerService::handleSaveConfig, this));
    _server.on("/api/status", HTTP_GET, std::bind(&WebServerService::handleApiStatus, this));
    _server.on("/api/alarms", HTTP_ANY, std::bind(&WebServerService::handleApiAlarms, this));
    _server.on("/api/pages", HTTP_ANY, std::bind(&WebServerService::handleApiPages, this));

    // Rutas de detección de portal cautivo para celulares Android, iOS y Windows
    _server.on("/generate_204", [this]() {
        if (isCaptivePortalRequest()) { handleCaptivePortalRedirect(); }
        else { handleRoot(); }
    });
    _server.on("/hotspot-detect.html", [this]() {
        if (isCaptivePortalRequest()) { handleCaptivePortalRedirect(); }
        else { handleRoot(); }
    });
    _server.on("/connecttest.txt", [this]() {
        if (isCaptivePortalRequest()) { handleCaptivePortalRedirect(); }
        else { handleRoot(); }
    });
    _server.on("/ncsi.txt", [this]() {
        if (isCaptivePortalRequest()) { handleCaptivePortalRedirect(); }
        else { handleRoot(); }
    });
    _server.on("/redirect", [this]() { handleCaptivePortalRedirect(); });
}

void WebServerService::setupPortalRoutes() {
    _server.on("/", HTTP_GET, std::bind(&WebServerService::handleSetupPage, this));
    _server.onNotFound([this]() {
        if (isCaptivePortalRequest()) {
            handleCaptivePortalRedirect();
        } else {
            handleSetupPage();
        }
    });
}

void WebServerService::setupStationRoutes() {
    _server.on("/", HTTP_GET, std::bind(&WebServerService::handleRoot, this));
    _server.on("/setup", HTTP_GET, std::bind(&WebServerService::handleSetupPage, this));

    // Endpoints REST de la API
    _server.on("/api/page", HTTP_POST, std::bind(&WebServerService::handleApiPage, this));
    _server.on("/api/sleep", HTTP_POST, std::bind(&WebServerService::handleApiSleep, this));
    _server.on("/api/timer", HTTP_POST, std::bind(&WebServerService::handleApiTimer, this));
    _server.on("/api/alarm", HTTP_POST, std::bind(&WebServerService::handleApiAlarm, this));
    _server.on("/api/alarms", HTTP_ANY, std::bind(&WebServerService::handleApiAlarms, this));
    _server.on("/api/pages", HTTP_ANY, std::bind(&WebServerService::handleApiPages, this));

    _server.onNotFound([this]() {
        _server.send(404, "text/plain", "404: Ruta no encontrada");
    });
}

void WebServerService::handleRoot() {
    if (_mode == WEB_MODE_PORTAL_AP) {
        handleSetupPage();
    } else {
        _server.send_P(200, "text/html", CONTROL_PAGE_HTML);
    }
}

void WebServerService::handleSetupPage() {
    _server.send_P(200, "text/html", SETUP_PAGE_HTML);
}

void WebServerService::handleScanWifi() {
    int n = WiFi.scanNetworks(false, true); // Escaneo sincrono
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < n && i < 15; ++i) {
        JsonObject obj = arr.add<JsonObject>();
        obj["ssid"] = WiFi.SSID(i);
        obj["rssi"] = WiFi.RSSI(i);
        obj["enc"] = (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? 0 : 1;
    }

    String resp;
    serializeJson(doc, resp);
    _server.send(200, "application/json", resp);
    WiFi.scanDelete();
}

void WebServerService::handleSaveConfig() {
    String ssid = _server.arg("ssid");
    String pass = _server.arg("password");
    String loc = _server.arg("location");
    String lat = _server.arg("lat");
    String lon = _server.arg("lon");
    uint8_t melody = _server.arg("melody").toInt();
    uint8_t page = _server.arg("page").toInt();

    bool p1 = _server.hasArg("p1");
    bool p2 = _server.hasArg("p2");
    bool p3 = _server.hasArg("p3");
    bool p4 = _server.hasArg("p4");
    bool p5 = _server.hasArg("p5");
    if (!p1 && !p2 && !p3 && !p4 && !p5) {
        p1 = true; p2 = true; p3 = true; p4 = true; p5 = true;
    }

    if (melody < 1 || melody > MELODY_MAX_TYPE) melody = 1;
    if (page < 1 || page > TOTAL_PAGES) page = 1;

    Serial.printf("[Web] Recibidos nuevos parametros: SSID=%s, Ciudad=%s, Lat=%s, Lon=%s, Melodia=%d, Pagina=%d\n",
                  ssid.c_str(), loc.c_str(), lat.c_str(), lon.c_str(), melody, page);

    bool ok = false;
    if (_cb.onSaveConfig) {
        ok = _cb.onSaveConfig(ssid.c_str(), pass.c_str(), loc.c_str(), lat.c_str(), lon.c_str(), melody, page, p1, p2, p3, p4, p5);
    } else {
        _settings.setWifi(ssid.c_str(), pass.c_str());
        _settings.setLocation(loc.c_str(), lat.c_str(), lon.c_str());
        _settings.setAlarmMelody(melody);
        _settings.setDefaultPage(page);
        _settings.setPageEnabled(1, p1);
        _settings.setPageEnabled(2, p2);
        _settings.setPageEnabled(3, p3);
        _settings.setPageEnabled(4, p4);
        _settings.setPageEnabled(5, p5);
        ok = _settings.save();
    }

    if (ok) {
        _server.send_P(200, "text/html", SAVED_PAGE_HTML);
        _rebootRequested = true;
        _rebootRequestedTime = millis();
    } else {
        _server.send(500, "text/plain", "Error guardando configuracion en LittleFS");
    }
}

void WebServerService::handleApiStatus() {
    JsonDocument doc;

    char timeBuf[16] = "--:--";
    char condBuf[32] = "";
    float temp = 0.0f;
    int hum = 0;

    if (_cb.getWeatherInfo) {
        _cb.getWeatherInfo(timeBuf, sizeof(timeBuf), temp, hum, condBuf, sizeof(condBuf));
    }

    doc["time"] = timeBuf;
    doc["temp"] = temp;
    doc["humidity"] = hum;
    doc["condition"] = condBuf;
    doc["page"] = _cb.getCurrentPage ? _cb.getCurrentPage() : 1;
    doc["sleeping"] = _cb.isSleeping ? _cb.isSleeping() : false;
    doc["rssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
    doc["alarming"] = _cb.isAlarming ? _cb.isAlarming() : false;
    doc["melody"] = _cb.getCurrentMelody ? _cb.getCurrentMelody() : 1;

    if (_cb.getTimerStatus) {
        uint32_t rem = 0, tot = 0;
        uint8_t st = 0;
        _cb.getTimerStatus(rem, tot, st);
        JsonObject t = doc["timer"].to<JsonObject>();
        t["remaining"] = rem;
        t["total"] = tot;
        t["state"] = st;
    }

    const AppSettings& s = _settings.get();
    JsonObject cfg = doc["config"].to<JsonObject>();
    cfg["ssid"] = s.wifiSsid;
    cfg["location"] = s.locationName;
    cfg["lat"] = s.locationLat;
    cfg["lon"] = s.locationLon;
    cfg["melody"] = s.alarmMelody;
    cfg["page"] = s.defaultPage;

    JsonObject pagesObj = doc["enabled_pages"].to<JsonObject>();
    pagesObj["1"] = s.enabledPage1;
    pagesObj["2"] = s.enabledPage2;
    pagesObj["3"] = s.enabledPage3;
    pagesObj["4"] = s.enabledPage4;
    pagesObj["5"] = s.enabledPage5;

    String json;
    serializeJson(doc, json);
    _server.send(200, "application/json", json);
}

void WebServerService::handleApiPage() {
    if (_server.hasArg("page")) {
        uint8_t page = _server.arg("page").toInt();
        if (page >= 1 && page <= TOTAL_PAGES) {
            bool enabled = _cb.isPageEnabled ? _cb.isPageEnabled(page) : _settings.get().isPageEnabled(page);
            if (!enabled) {
                _server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Pagina deshabilitada\"}");
                return;
            }
            if (_cb.onPageChange) {
                _cb.onPageChange(page);
                _server.send(200, "application/json", "{\"status\":\"ok\"}");
                return;
            }
        }
    }
    _server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Pagina invalida (1..5)\"}");
}

void WebServerService::handleApiSleep() {
    if (_cb.onToggleSleep) {
        _cb.onToggleSleep();
        _server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        _server.send(500, "application/json", "{\"status\":\"error\"}");
    }
}

void WebServerService::handleApiTimer() {
    String action = _server.arg("action");

    uint32_t totalSec = 0;
    if (_server.hasArg("seconds")) {
        totalSec = _server.arg("seconds").toInt();
    } else {
        uint32_t hrs = _server.hasArg("hours") ? _server.arg("hours").toInt() : 0;
        uint32_t mins = _server.hasArg("minutes") ? _server.arg("minutes").toInt() : 0;
        totalSec = (hrs * 3600) + (mins * 60);
    }

    if (action == "start") {
        if (totalSec < 5) totalSec = 5;
        if (totalSec > 86400) totalSec = 86400; // Máximo 24h
        if (_cb.onTimerStart) {
            _cb.onTimerStart(totalSec);
            _server.send(200, "application/json", "{\"status\":\"ok\",\"action\":\"started\",\"seconds\":" + String(totalSec) + "}");
            return;
        }
    } else if (action == "stop") {
        if (_cb.onTimerStop) {
            _cb.onTimerStop();
            _server.send(200, "application/json", "{\"status\":\"ok\",\"action\":\"stopped\"}");
            return;
        }
    } else if (action == "set") {
        if (totalSec < 5) totalSec = 5;
        if (totalSec > 86400) totalSec = 86400;
        if (_cb.onTimerSetSeconds) {
            _cb.onTimerSetSeconds(totalSec);
            _server.send(200, "application/json", "{\"status\":\"ok\",\"action\":\"set\",\"seconds\":" + String(totalSec) + "}");
            return;
        }
    }

    _server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Accion de timer invalida\"}");
}

void WebServerService::handleApiAlarm() {
    String action = _server.arg("action");

    if (action == "test") {
        if (_cb.onTestAlarm) _cb.onTestAlarm();
        _server.send(200, "application/json", "{\"status\":\"ok\",\"action\":\"test\"}");
        return;
    } else if (action == "stop") {
        if (_cb.onStopAlarm) _cb.onStopAlarm();
        _server.send(200, "application/json", "{\"status\":\"ok\",\"action\":\"stop\"}");
        return;
    } else if (action == "set") {
        uint8_t m = _server.arg("melody").toInt();
        if (m >= 1 && m <= 13) {
            if (_cb.onSetMelody) _cb.onSetMelody(m);
            _settings.setAlarmMelody(m);
            _settings.save();
            _server.send(200, "application/json", "{\"status\":\"ok\",\"melody\":" + String(m) + "}");
            return;
        }
    }

    _server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Accion de alarma invalida\"}");
}

void WebServerService::handleApiAlarms() {
    if (_server.method() == HTTP_GET) {
        JsonDocument doc;
        doc["status"] = "ok";
        doc["count"] = MAX_DAILY_ALARMS;

        JsonArray arr = doc["alarms"].to<JsonArray>();
        const DailyAlarm* alarms = _settings.get().alarms;
        for (uint8_t i = 0; i < MAX_DAILY_ALARMS; i++) {
            JsonObject a = arr.add<JsonObject>();
            a["id"] = alarms[i].id;
            a["enabled"] = alarms[i].enabled;
            a["hour"] = alarms[i].hour;
            a["minute"] = alarms[i].minute;
            a["days"] = alarms[i].days;
            a["melody"] = alarms[i].melody;
            char dStr[24];
            formatAlarmDaysString(alarms[i].days, dStr, sizeof(dStr));
            a["days_str"] = dStr;
            a["label"] = alarms[i].label;
        }

        String resp;
        serializeJson(doc, resp);
        _server.send(200, "application/json", resp);
        return;
    }

    if (_server.method() == HTTP_POST) {
        String action = _server.arg("action");

        if (action == "toggle") {
            int id = _server.arg("id").toInt();
            if (id >= 1 && id <= MAX_DAILY_ALARMS) {
                bool newState = false;
                if (_cb.onToggleAlarm) {
                    newState = _cb.onToggleAlarm(id);
                } else {
                    newState = _settings.toggleAlarm(id - 1);
                    _settings.save();
                }
                _server.send(200, "application/json", "{\"status\":\"ok\",\"action\":\"toggled\",\"id\":" + String(id) + ",\"enabled\":" + (newState ? "true" : "false") + "}");
                return;
            }
            _server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"ID invalido\"}");
            return;
        }

        if (action == "set") {
            int id = _server.arg("id").toInt();
            if (id >= 1 && id <= MAX_DAILY_ALARMS) {
                bool en = _server.hasArg("enabled") ? (_server.arg("enabled") == "1" || _server.arg("enabled") == "true") : false;
                int h = _server.arg("hour").toInt();
                int m = _server.arg("minute").toInt();
                int d = _server.hasArg("days") ? _server.arg("days").toInt() : ALARM_DAYS_ALL;
                String lbl = _server.arg("label");

                if (h < 0) { h = 0; }
                if (h > 23) { h = 23; }
                if (m < 0) { m = 0; }
                if (m > 59) { m = 59; }
                if (d < 0 || d > 127) d = ALARM_DAYS_ALL;
                int mel = _server.hasArg("melody") ? _server.arg("melody").toInt() : 0;
                if (mel < 0 || mel > MELODY_MAX_TYPE) mel = 0;

                bool ok = false;
                if (_cb.onSetAlarm) {
                    ok = _cb.onSetAlarm(id, en, (uint8_t)h, (uint8_t)m, (uint8_t)d, (uint8_t)mel, lbl.c_str());
                } else {
                    DailyAlarm a;
                    a.id = id;
                    a.enabled = en;
                    a.hour = (uint8_t)h;
                    a.minute = (uint8_t)m;
                    a.days = (uint8_t)d;
                    a.melody = (uint8_t)mel;
                    strncpy(a.label, lbl.c_str(), ALARM_LABEL_MAX_LEN);
                    a.label[ALARM_LABEL_MAX_LEN] = '\0';
                    ok = _settings.setAlarm(id - 1, a);
                    if (ok) _settings.save();
                }

                if (ok) {
                    _server.send(200, "application/json", "{\"status\":\"ok\",\"action\":\"set\",\"id\":" + String(id) + "}");
                } else {
                    _server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Error guardando alarma\"}");
                }
                return;
            }
            _server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"ID invalido\"}");
            return;
        }

        _server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Accion desconocida\"}");
        return;
    }

    _server.send(405, "application/json", "{\"status\":\"error\",\"message\":\"Metodo no permitido\"}");
}

void WebServerService::handleApiPages() {
    if (_server.method() == HTTP_GET) {
        JsonDocument doc;
        doc["status"] = "ok";
        JsonObject p = doc["pages"].to<JsonObject>();
        const AppSettings& s = _settings.get();
        p["1"] = s.enabledPage1;
        p["2"] = s.enabledPage2;
        p["3"] = s.enabledPage3;
        p["4"] = s.enabledPage4;
        p["5"] = s.enabledPage5;

        String resp;
        serializeJson(doc, resp);
        _server.send(200, "application/json", resp);
        return;
    }

    if (_server.method() == HTTP_POST) {
        for (uint8_t i = 1; i <= TOTAL_PAGES; i++) {
            String argName = "p" + String(i);
            if (_server.hasArg(argName)) {
                bool en = (_server.arg(argName) == "1" || _server.arg(argName) == "true");
                if (_cb.onSetPageEnabled) {
                    _cb.onSetPageEnabled(i, en);
                } else {
                    _settings.setPageEnabled(i, en);
                }
            }
        }
        _settings.save();
        _server.send(200, "application/json", "{\"status\":\"ok\"}");
        return;
    }

    _server.send(405, "application/json", "{\"status\":\"error\",\"message\":\"Metodo no permitido\"}");
}

#endif
