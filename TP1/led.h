#pragma once
#include <Arduino.h>
#include "config.h"

// ─── Estado interno ───────────────────────────────────────────────────────────
static bool _ledState = LOW;

// ─── API ──────────────────────────────────────────────────────────────────────
void led_init() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, _ledState);
  Serial.println("[LED] Iniciado en pin " + String(LED_PIN));
}

void led_toggle() {
  _ledState = !_ledState;
  digitalWrite(LED_PIN, _ledState);
  Serial.printf("[LED] Estado: %s\n", _ledState ? "ON" : "OFF");
}

bool led_state() {
  return _ledState;
}
