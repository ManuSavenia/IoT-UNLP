#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "config.h"

DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long lastPublish = 0;

void connectWiFi() {
    Serial.printf("[WiFi] Conectando a SSID: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\n[WiFi] Conectado. IP asignada: %s\n", WiFi.localIP().toString().c_str());
}

void connectMQTT() {
    while (!mqttClient.connected()) {
        Serial.printf("[MQTT] Intentando conectar a %s:%d (ClientID: ESP32-DHT22) ...\n",
                      MQTT_SERVER, MQTT_PORT);
        if (mqttClient.connect("ESP32-DHT22")) {
            Serial.println("[MQTT] Conectado al broker.");
        } else {
            Serial.printf("[MQTT] Fallo de conexion (rc=%d). Reintentando en 5s.\n",
                          mqttClient.state());
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== ESP32 DHT22 MQTT - Iniciando ===");
    Serial.printf("[Config] SSID:          %s\n", WIFI_SSID);
    Serial.printf("[Config] Broker MQTT:   %s\n", MQTT_SERVER);
    Serial.printf("[Config] Puerto MQTT:   %d\n", MQTT_PORT);
    Serial.printf("[Config] Topico MQTT:   %s\n", MQTT_TOPIC);
    Serial.printf("[Config] Pin DHT22:     GPIO %d\n", DHT_PIN);
    Serial.printf("[Config] Intervalo:     %d ms\n", PUBLISH_INTERVAL_MS);
    Serial.println("=====================================\n");

    dht.begin();
    connectWiFi();
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    connectMQTT();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] Conexion perdida. Reconectando...");
        connectWiFi();
    }

    if (!mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop();

    unsigned long now = millis();
    if (now - lastPublish >= PUBLISH_INTERVAL_MS) {
        lastPublish = now;

        float temperature = dht.readTemperature();
        float humidity    = dht.readHumidity();

        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("[DHT22] ERROR: Lectura invalida (NaN). Verificar cableado y pin.");
            return;
        }

        Serial.printf("[DHT22] Temperatura: %.1f C  Humedad: %.1f%%\n", temperature, humidity);

        // JSON armado con snprintf, sin ArduinoJson. Buffer 128 bytes cubre todos los campos.
        char payload[128];
        snprintf(payload, sizeof(payload),
                 "{\"temperature\":%.1f,\"humidity\":%.1f,\"source\":\"esp32-dht22\",\"uptime_ms\":%lu}",
                 temperature, humidity, now);

        Serial.printf("[MQTT] Publicando en '%s': %s\n", MQTT_TOPIC, payload);
        bool ok = mqttClient.publish(MQTT_TOPIC, payload);
        if (ok) {
            Serial.println("[MQTT] Publicacion exitosa.");
        } else {
            Serial.println("[MQTT] ERROR: Fallo al publicar. Verificar conexion al broker.");
        }
    }
}
