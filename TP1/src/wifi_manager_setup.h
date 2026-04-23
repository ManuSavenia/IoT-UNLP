#pragma once
#include <WiFi.h>
#include <WiFiManager.h>
#include "config.h"

// ─── API ──────────────────────────────────────────────────────────────────────
// Intenta conectarse con credenciales guardadas en NVS.
// Si no tiene o falla, levanta un portal cautivo en modo AP.
// Bloquea hasta que se conecta exitosamente.
void wifi_setup() {
  WiFiManager wm;
  wm.setConfigPortalTimeout(WIFI_TIMEOUT_S);

  Serial.println("[WiFi] Intentando conectar con credenciales guardadas...");

  bool connected = wm.autoConnect(WIFI_AP_NAME, WIFI_AP_PASSWORD);

  if (!connected) {
    Serial.println("[WiFi] Timeout en portal cautivo. Reiniciando...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("[WiFi] Conectado!");
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("[WiFi] SSID: ");
  Serial.println(WiFi.SSID());
}
