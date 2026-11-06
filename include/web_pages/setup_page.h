#pragma once

#if defined(ESP8266)
#include <Arduino.h>
#include "config.h"

#if defined(WEB_TITLE)
#define WEB_PANEL_TITLE WEB_TITLE
#elif !defined(WEB_PANEL_TITLE)
#define WEB_PANEL_TITLE "Weather Clock"
#endif

static const char SETUP_PAGE_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Configuración - )rawliteral" WEB_PANEL_TITLE R"rawliteral(</title>
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
    <h1>)rawliteral" WEB_PANEL_TITLE R"rawliteral(</h1>
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
</script>
</body>
</html>)rawliteral";

#endif
