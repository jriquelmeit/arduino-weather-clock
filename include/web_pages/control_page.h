#pragma once

#if defined(ESP8266)
#include <Arduino.h>
#include "config.h"

#if defined(WEB_TITLE)
#define WEB_PANEL_TITLE WEB_TITLE
#elif !defined(WEB_PANEL_TITLE)
#define WEB_PANEL_TITLE "Weather Clock"
#endif

static const char CONTROL_PAGE_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Panel de Control - )rawliteral" WEB_PANEL_TITLE R"rawliteral(</title>
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
    <h1>)rawliteral" WEB_PANEL_TITLE R"rawliteral(</h1>
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
    <div style="display:grid;grid-template-columns:repeat(auto-fit,minmax(80px,1fr));gap:6px;margin-bottom:10px" id="page_buttons_grid">
      <button class="btn btn-outline" id="btn_page1" onclick="setPage(1)">P1: Diario</button>
      <button class="btn btn-outline" id="btn_page2" onclick="setPage(2)">P2: Horas</button>
      <button class="btn btn-outline" id="btn_page3" onclick="setPage(3)">P3: Reloj</button>
      <button class="btn btn-outline" id="btn_page4" onclick="setPage(4)">P4: Timer</button>
      <button class="btn btn-outline" id="btn_page5" onclick="setPage(5)">P5: Alarma</button>
      <button class="btn btn-outline" id="btn_page6" onclick="setPage(6)">P6: Spotify</button>
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

      for (let i = 1; i <= 6; i++) {
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

#endif
