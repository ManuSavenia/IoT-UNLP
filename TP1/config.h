#pragma once

// ─── Pines ────────────────────────────────────────────────────────────────────
#define LED_PIN  2
#define DHT_PIN  18
#define DHTTYPE  DHT22

// ─── Servidor ─────────────────────────────────────────────────────────────────
#define SERVER_PORT 80

// ─── WiFiManager ──────────────────────────────────────────────────────────────
#define WIFI_AP_NAME     "ESP32-IoT-Setup"
#define WIFI_AP_PASSWORD "12345678"
#define WIFI_TIMEOUT_S   30

// ─── DHT ──────────────────────────────────────────────────────────────────────
#define DHT_READ_INTERVAL_MS 2500

// ─── SPIFFS ───────────────────────────────────────────────────────────────────
#define HTML_PATH "/index.html"
