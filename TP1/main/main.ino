#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// Credenciales
const char* ssid = "Fibertel WiFi412 2.4GHz";
const char* password = "0142041694";

const int ledPin = 2; 
bool ledState = LOW;

// Datos de prueba para cumplir requisitos mínimos [cite: 25, 26, 28]
float t = 25.5;
float h = 60.0;
bool tempOk = false;
bool humOk = false;

AsyncWebServer server(80);

// HTML optimizado: Estética Natural, fondo celeste y responsive 
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>IoT UNLP - Dashboard</title>
  <style>
    :root {
      --bg-blue: #e3f2fd;
      --panel-light: #f1f8e9;
      --accent-green: #4caf50;
      --card-green: #c8e6c9;
      --text-dark: #1b5e20;
      --border-green: #81c784;
    }
    body { 
      font-family: 'Segoe UI', Tahoma, sans-serif; 
      background-color: var(--bg-blue); 
      margin: 0; display: flex; justify-content: center; align-items: center; 
      min-height: 100vh; padding: 20px; box-sizing: border-box;
    }
    .main-container { 
      display: flex; flex-direction: row; width: 100%; max-width: 850px; 
      min-height: 450px; background: white; border-radius: 25px; 
      box-shadow: 0 15px 35px rgba(0,0,0,0.15); overflow: hidden;
      border: 1px solid rgba(255,255,255,0.5);
    }
    .panel { flex: 1; padding: 40px; display: flex; flex-direction: column; gap: 20px; justify-content: center; align-items: center; }
    .left { background-color: var(--panel-light); border-right: 2px solid var(--card-green); }
    .right { background-color: #ffffff; padding: 40px; }
    .btn { 
      width: 100%; background-color: var(--accent-green); color: white; padding: 22px; 
      font-size: 1.3rem; font-weight: bold; border: none; border-radius: 15px; cursor: pointer; 
      transition: 0.3s; box-shadow: 0 4px 10px rgba(76, 175, 80, 0.3); text-transform: uppercase;
    }
    .btn:hover { background-color: #388e3c; transform: translateY(-2px); }
    .btn:active { transform: translateY(0); }
    .status { 
      width: 100%; background: white; border: 1.5px solid var(--border-green); 
      padding: 14px; border-radius: 12px; color: var(--text-dark); text-align: center; font-size: 0.95rem; box-sizing: border-box;
    }
    .card { 
      width: 100%; background: var(--card-green); padding: 25px; border-radius: 20px; 
      text-align: center; color: var(--text-dark); border: 1px solid var(--border-green); box-sizing: border-box;
    }
    .val { font-size: 3.2rem; font-weight: 800; display: block; margin: 5px 0; }
    h2 { margin: 0; font-size: 1rem; opacity: 0.7; text-transform: uppercase; letter-spacing: 1px; }

    @media (max-width: 768px) {
      .main-container { flex-direction: column; max-width: 100%; min-height: auto; }
      .left { border-right: none; border-bottom: 2px solid var(--card-green); padding: 30px 20px; }
      .right { padding: 30px 20px; }
    }
  </style>
</head>
<body>
  <div class="main-container">
    <div class="panel left">
      <button class="btn" onclick="fetch('/toggle')">TOGGLE LED</button>
      <div class="status">Sensor Temp: <strong id="ts">--</strong></div>
      <div class="status">Sensor Hum: <strong id="hs">--</strong></div>
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
  <script>
    function update() {
      fetch('/data').then(r => r.json()).then(d => {
        document.getElementById('tv').innerText = d.t + "°C";
        document.getElementById('hv').innerText = d.h + "%";
        document.getElementById('wn').innerText = d.w;
        document.getElementById('ts').innerText = d.to ? "CONECTADO" : "DESCONECTADO";
        document.getElementById('hs').innerText = d.ho ? "CONECTADO" : "DESCONECTADO";
      });
    }
    setInterval(update, 2000);
    update();
  </script>
</body>
</html>)rawliteral";

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConectado: " + WiFi.localIP().toString());

  // Servidor principal 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });

  // Endpoint para actualización de valores 
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"t\":" + String(t, 1) + ",";
    json += "\"h\":" + String(h, 1) + ",";
    json += "\"w\":\"" + String(WiFi.SSID()) + "\",";
    json += "\"to\":" + String(tempOk ? "true" : "false") + ",";
    json += "\"ho\":" + String(humOk ? "true" : "false");
    json += "}";
    request->send(200, "application/json", json);
  });

  // Control del LED integrado 
  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  // El loop se mantiene limpio al usar servidor asincrónico 
}