#pragma once
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "config.h"
#include "sensor.h"
#include "led.h"

static AsyncWebServer _server(SERVER_PORT);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>IoT UNLP - Dashboard</title>
  <style>
    :root {
      --bg-blue:     #e3f2fd;
      --panel-light: #f1f8e9;
      --accent-green:#4caf50;
      --card-green:  #c8e6c9;
      --text-dark:   #1b5e20;
      --border-green:#81c784;
    }
    * { box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Tahoma, sans-serif;
      background-color: var(--bg-blue);
      margin: 0;
      display: flex;
      flex-direction: column;
      align-items: center;
      min-height: 100vh;
      padding: 20px;
      gap: 20px;
    }
    .main-container {
      display: flex;
      flex-direction: row;
      width: 100%;
      max-width: 850px;
      min-height: 450px;
      background: white;
      border-radius: 25px;
      box-shadow: 0 15px 35px rgba(0,0,0,0.15);
      overflow: hidden;
      border: 1px solid rgba(255,255,255,0.5);
    }
    .panel {
      flex: 1;
      padding: 40px;
      display: flex;
      flex-direction: column;
      gap: 20px;
      justify-content: center;
      align-items: center;
    }
    .left {
      background-color: var(--panel-light);
      border-right: 2px solid var(--card-green);
    }
    .right { background-color: #ffffff; }
    .btn {
      width: 100%;
      background-color: var(--accent-green);
      color: white;
      padding: 22px;
      font-size: 1.3rem;
      font-weight: bold;
      border: none;
      border-radius: 15px;
      cursor: pointer;
      transition: 0.3s;
      box-shadow: 0 4px 10px rgba(76,175,80,0.3);
      text-transform: uppercase;
    }
    .btn:hover  { background-color: #388e3c; transform: translateY(-2px); }
    .btn:active { transform: translateY(0); }
    .status {
      width: 100%;
      background: white;
      border: 1.5px solid var(--border-green);
      padding: 14px;
      border-radius: 12px;
      color: var(--text-dark);
      text-align: center;
      font-size: 0.95rem;
    }
    .card {
      width: 100%;
      background: var(--card-green);
      padding: 25px;
      border-radius: 20px;
      text-align: center;
      color: var(--text-dark);
      border: 1px solid var(--border-green);
    }
    .val {
      font-size: 3.2rem;
      font-weight: 800;
      display: block;
      margin: 5px 0;
    }
    h2 {
      margin: 0;
      font-size: 1rem;
      opacity: 0.7;
      text-transform: uppercase;
      letter-spacing: 1px;
    }

    /* ── Historial ── */
    .history-container {
      width: 100%;
      max-width: 850px;
      background: white;
      border-radius: 25px;
      box-shadow: 0 15px 35px rgba(0,0,0,0.15);
      padding: 30px 40px;
    }
    .history-container h3 {
      margin: 0 0 16px 0;
      color: var(--text-dark);
      font-size: 1rem;
      text-transform: uppercase;
      letter-spacing: 1px;
      opacity: 0.7;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      font-size: 0.9rem;
    }
    th {
      background: var(--card-green);
      color: var(--text-dark);
      padding: 10px 14px;
      text-align: left;
      font-weight: 600;
    }
    th:first-child { border-radius: 10px 0 0 10px; }
    th:last-child  { border-radius: 0 10px 10px 0; }
    td {
      padding: 9px 14px;
      border-bottom: 1px solid #f0f0f0;
      color: #333;
    }
    tr:last-child td { border-bottom: none; }
    tr:hover td { background: #f9fbe7; }

    @media (max-width: 768px) {
      .main-container { flex-direction: column; min-height: auto; }
      .left { border-right: none; border-bottom: 2px solid var(--card-green); padding: 30px 20px; }
      .right { padding: 30px 20px; }
      .history-container { padding: 20px; }
    }
  </style>
</head>
<body>

  <div class="main-container">
    <div class="panel left">
      <button class="btn" onclick="fetch('/toggle')">TOGGLE LED</button>
      <div class="status">Sensor Temp: <strong id="ts">--</strong></div>
      <div class="status">Sensor Hum:  <strong id="hs">--</strong></div>
      <div class="status">WiFi: <strong id="wn">--</strong></div>
    </div>
    <div class="panel right">
      <div class="card">
        <h2>Temperatura</h2>
        <span class="val" id="tv">--</span>
      </div>
      <div class="card">
        <h2>Humedad</h2>
        <span class="val" id="hv">--</span>
      </div>
    </div>
  </div>

  <div class="history-container">
    <h3>Historial de mediciones</h3>
    <table>
      <thead>
        <tr>
          <th>#</th>
          <th>Tiempo</th>
          <th>Temperatura</th>
          <th>Humedad</th>
        </tr>
      </thead>
      <tbody id="history-body">
        <tr><td colspan="4" style="text-align:center;opacity:0.5">Cargando...</td></tr>
      </tbody>
    </table>
  </div>

  <script>
    function msToTime(ms) {
      let s = Math.floor(ms / 1000);
      let m = Math.floor(s / 60);
      let h = Math.floor(m / 60);
      s = s % 60;
      m = m % 60;
      if (h > 0) return h + 'h ' + m + 'm ' + s + 's';
      if (m > 0) return m + 'm ' + s + 's';
      return s + 's';
    }

    function update() {
      fetch('/data')
        .then(r => r.json())
        .then(d => {
          document.getElementById('tv').innerText = d.t + '°C';
          document.getElementById('hv').innerText = d.h + '%';
          document.getElementById('wn').innerText = d.w;
          document.getElementById('ts').innerText = d.to ? 'CONECTADO' : 'ERROR';
          document.getElementById('hs').innerText = d.ho ? 'CONECTADO' : 'ERROR';
        })
        .catch(() => {
          ['tv','hv','wn','ts','hs'].forEach(id =>
            document.getElementById(id).innerText = '...'
          );
        });

      fetch('/api/metrics')
        .then(r => r.json())
        .then(records => {
          const tbody = document.getElementById('history-body');
          if (records.length === 0) {
            tbody.innerHTML = '<tr><td colspan="4" style="text-align:center;opacity:0.5">Sin datos aún</td></tr>';
            return;
          }
          // Mostrar del más nuevo al más viejo
          tbody.innerHTML = records.slice().reverse().map((r, i) =>
            `<tr>
              <td>${records.length - i}</td>
              <td>${msToTime(r.ms)}</td>
              <td>${r.t}°C</td>
              <td>${r.h}%</td>
            </tr>`
          ).join('');
        });
    }

    setInterval(update, 2500);
    update();
  </script>
</body>
</html>
)rawliteral";

static String build_json() {
  SensorData d = sensor_get();
  String json = "{";
  json += "\"t\":"   + String(d.temperature, 1) + ",";
  json += "\"h\":"   + String(d.humidity, 1)    + ",";
  json += "\"w\":\"" + String(WiFi.SSID())      + "\",";
  json += "\"to\":"  + String(d.tempOk ? "true" : "false") + ",";
  json += "\"ho\":"  + String(d.humOk  ? "true" : "false");
  json += "}";
  return json;
}

void web_init() {
  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/html", index_html);
  });

  _server.on("/data", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "application/json", build_json());
  });

  _server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *req) {
    led_toggle();
    req->send(200, "text/plain", "OK");
  });

  // Endpoint REST con historial completo
  _server.on("/api/metrics", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "application/json", sensor_history_json());
  });

  _server.onNotFound([](AsyncWebServerRequest *req) {
    req->send(404, "text/plain", "No encontrado");
  });

  _server.begin();
  Serial.println("[Web] Servidor iniciado en puerto " + String(SERVER_PORT));
}