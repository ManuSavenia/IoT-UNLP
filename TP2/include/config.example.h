#pragma once

// Copiar este archivo: cp include/config.example.h include/config.h
// Editar config.h con los valores reales antes de compilar.
// config.h está en .gitignore y nunca se sube al repositorio.

// ── WiFi ──────────────────────────────────────────────────────────────────────
#define WIFI_SSID        "TU_WIFI"
#define WIFI_PASSWORD    "TU_PASSWORD"

// ── MQTT ──────────────────────────────────────────────────────────────────────
// Debe ser la IP local de la PC donde corre Docker (NO "localhost" ni "127.0.0.1").
// Para encontrarla: hostname -I   (Linux/Mac)  |  ipconfig  (Windows)
#define MQTT_SERVER      "192.168.X.X"
#define MQTT_PORT        1883
#define MQTT_TOPIC       "sensor/ambiente"

// ── DHT22 ─────────────────────────────────────────────────────────────────────
// Cableado:
//   VCC  → 3.3V del ESP32
//   GND  → GND del ESP32
//   DATA → GPIO definido en DHT_PIN
// Si el módulo no trae resistencia pull-up integrada,
// conectar una resistencia de 10kΩ entre DATA y VCC.
#define DHT_PIN          4        // cambiar al GPIO real donde está conectado DATA
#define DHT_TYPE         DHT22

// ── Publicación ───────────────────────────────────────────────────────────────
#define PUBLISH_INTERVAL_MS  5000  // milisegundos entre cada lectura y publicación
