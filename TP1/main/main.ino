#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <DHT.h>

// WiFi
const char* ssid = "YOUR SSID";
const char* password = "YOUR PWD";

// Pines
const int ledPin = 2;
const int dhtPin = 18;

// DHT22
#define DHTTYPE DHT22
DHT dht(dhtPin, DHTTYPE);

// Variables
bool ledState = LOW;
float t = 0.0;
float h = 0.0;
bool tempOk = false;
bool humOk = false;

AsyncWebServer server(80);

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
      margin: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
      padding: 20px;
      box-sizing: border-box;
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
    .right {
      background-color: #ffffff;
      padding: 40px;
    }
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
      box-shadow: 0 4px 10px rgba(76, 175, 80, 0.3);
      text-transform: uppercase;
    }
    .btn:hover { background-color: #388e3c; transform: translateY(-2px); }
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
      box-sizing: border-box;
    }
    .card {
      width: 100%;
      background: var(--card-green);
      padding: 25px;
      border-radius: 20px;
      text-align: center;
      color: var(--text-dark);
      border: 1px solid var(--border-green);
      box-sizing: border-box;
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
    @media (max-width: 768px) {
      .main-container {
        flex-direction: column;
        max-width: 100%;
        min-height: auto;
      }
      .left {
        border-right: none;
        border-bottom: 2px solid var(--card-green);
        padding: 30px 20px;
      }
      .right {
        padding: 30px 20px;
      }
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
      fetch('/data')
        .then(r => r.json())
        .then(d => {
          document.getElementById('tv').innerText = d.t + "°C";
          document.getElementById('hv').innerText = d.h + "%";
          document.getElementById('wn').innerText = d.w;
          document.getElementById('ts').innerText = d.to ? "CONECTADO" : "ERROR";
          document.getElementById('hs').innerText = d.ho ? "CONECTADO" : "ERROR";
        });
    }
    setInterval(update, 2000);
    update();
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado. IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{";
    json += "\"t\":" + String(t, 1) + ",";
    json += "\"h\":" + String(h, 1) + ",";
    json += "\"w\":\"" + String(WiFi.SSID()) + "\",";
    json += "\"to\":" + String(tempOk ? "true" : "false") + ",";
    json += "\"ho\":" + String(humOk ? "true" : "false");
    json += "}";
    request->send(200, "application/json", json);
  });

  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.write("button!!");
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  static unsigned long lastRead = 0;
  unsigned long now = millis();

  if (now - lastRead >= 2500) {
    lastRead = now;

    float newH = dht.readHumidity();
    float newT = dht.readTemperature();

    if (!isnan(newT) && !isnan(newH)) {
      t = newT;
      h = newH;
      tempOk = true;
      humOk = true;

      Serial.print("Temperatura: ");
      Serial.print(t);
      Serial.print(" °C | Humedad: ");
      Serial.print(h);
      Serial.println(" %");
    } else {
      tempOk = false;
      humOk = false;
      Serial.println("Error al leer el DHT22");
    }
  }
}