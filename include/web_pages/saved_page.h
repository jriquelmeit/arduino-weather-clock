#pragma once

#if defined(ESP8266)
#include <Arduino.h>
#include "config.h"

#if defined(WEB_TITLE)
#define WEB_PANEL_TITLE WEB_TITLE
#elif !defined(WEB_PANEL_TITLE)
#define WEB_PANEL_TITLE "Weather Clock"
#endif

static const char SAVED_PAGE_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Guardado - )rawliteral" WEB_PANEL_TITLE R"rawliteral(</title>
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

#endif
