#pragma once
#include <DHT.h>
#include "config.h"

// ─── Estructuras ──────────────────────────────────────────────────────────────
struct SensorData {
  float temperature = 0.0f;
  float humidity    = 0.0f;
  bool  tempOk      = false;
  bool  humOk       = false;
};

struct SensorRecord {
  float temperature;
  float humidity;
  unsigned long timestamp;  // millis() al momento de la lectura
};

// ─── Estado interno ───────────────────────────────────────────────────────────
static DHT        _dht(DHT_PIN, DHTTYPE);
static SensorData _sensorData;

static SensorRecord _history[HISTORY_SIZE];
static int          _historyCount = 0;   // cuántos registros hay (hasta HISTORY_SIZE)
static int          _historyHead  = 0;   // índice del próximo slot a escribir (buffer circular)

// ─── API ──────────────────────────────────────────────────────────────────────
void sensor_init() {
  _dht.begin();
  Serial.println("[Sensor] DHT22 iniciado en pin " + String(DHT_PIN));
}

void sensor_update() {
  static unsigned long _lastRead = 0;
  unsigned long now = millis();
  if (now - _lastRead < DHT_READ_INTERVAL_MS) return;
  _lastRead = now;

  float newT = _dht.readTemperature();
  float newH = _dht.readHumidity();

  if (!isnan(newT) && !isnan(newH)) {
    _sensorData.temperature = newT;
    _sensorData.humidity    = newH;
    _sensorData.tempOk      = true;
    _sensorData.humOk       = true;

    // Guardar en historial (buffer circular)
    _history[_historyHead] = { newT, newH, now };
    _historyHead = (_historyHead + 1) % HISTORY_SIZE;
    if (_historyCount < HISTORY_SIZE) _historyCount++;

    Serial.printf("[Sensor] Temp: %.1f°C  Hum: %.1f%%\n", newT, newH);
  } else {
    _sensorData.tempOk = false;
    _sensorData.humOk  = false;
    Serial.println("[Sensor] Error al leer DHT22");
  }
}

SensorData sensor_get() {
  return _sensorData;
}

// Devuelve el historial como JSON ordenado del más viejo al más nuevo
String sensor_history_json() {
  String json = "[";
  // El más viejo está en _historyHead cuando el buffer está lleno
  int start = (_historyCount < HISTORY_SIZE) ? 0 : _historyHead;

  for (int i = 0; i < _historyCount; i++) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) json += ",";
    json += "{";
    json += "\"t\":"  + String(_history[idx].temperature, 1) + ",";
    json += "\"h\":"  + String(_history[idx].humidity, 1)    + ",";
    json += "\"ms\":" + String(_history[idx].timestamp);
    json += "}";
  }
  json += "]";
  return json;
}