#pragma once
#include <DHT.h>
#include "config.h"

// ─── Estado del sensor ────────────────────────────────────────────────────────
struct SensorData {
  float temperature = 0.0f;
  float humidity    = 0.0f;
  bool  tempOk      = false;
  bool  humOk       = false;
};

// ─── Instancia y estado interno ───────────────────────────────────────────────
static DHT        _dht(DHT_PIN, DHTTYPE);
static SensorData _sensorData;

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
