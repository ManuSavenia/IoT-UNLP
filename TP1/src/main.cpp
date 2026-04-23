#include <SPIFFS.h>

#include "config.h"
#include "sensor.h"
#include "led.h"
#include "wifi_manager_setup.h"
#include "web.h"

// ─── Setup ────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[Main] Iniciando...");

  led_init();
  sensor_init();

  if (!SPIFFS.begin(true)) {
    Serial.println("[Main] ERROR: no se pudo montar SPIFFS");
  } else {
    Serial.println("[Main] SPIFFS montado OK");
  }

  wifi_setup();   // WiFiManager: credenciales guardadas o portal cautivo
  web_init();     // Servidor web con rutas

  Serial.println("[Main] Sistema listo.");
}

// ─── Loop ─────────────────────────────────────────────────────────────────────
void loop() {
  sensor_update();   // lee DHT22 cada DHT_READ_INTERVAL_MS ms (non-blocking)
}
