#include "web_server_service.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "config.h"
#include "buzzer_controller.h"
#include "web_pages/web_pages.h"

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
