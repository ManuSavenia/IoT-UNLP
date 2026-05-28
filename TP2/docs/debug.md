# Guía de diagnóstico: DHT22 → ESP32 → MQTT

Esta guía permite validar el flujo paso a paso, empezando desde el hardware y llegando hasta Node-RED.

---

## Orden de diagnóstico recomendado

```
A. Verificar cableado del DHT22
B. Compilar y flashear el ESP32
C. Abrir monitor serial
D. Ver lecturas del sensor por Serial
E. Levantar Docker Compose
F. Verificar que Mosquitto esté corriendo
G. Suscribirse al tópico MQTT
H. Confirmar que llegan mensajes desde el ESP32
I. Probar publicación manual si no llegan mensajes
J. Ver mensajes en Node-RED con nodo debug
```

---

## A. Verificar cableado del DHT22

| Pin DHT22 | Conectar a      |
|-----------|-----------------|
| VCC       | 3.3V del ESP32  |
| GND       | GND del ESP32   |
| DATA      | GPIO 4 del ESP32 |

> Si el módulo DHT22 no trae resistencia pull-up integrada, conectar una resistencia de **10kΩ** entre DATA y VCC.

---

## B. Compilar y flashear el ESP32

```bash
# Compilar y subir el firmware
pio run --target upload

# Solo compilar (sin flashear)
pio run
```

Antes de compilar, asegurarse de tener `include/config.h` con los valores correctos:

```bash
cp include/config.example.h include/config.h
# Editar config.h con tu SSID, password y la IP de tu PC
```

---

## C. Abrir monitor serial

```bash
pio device monitor
```

O desde VS Code con PlatformIO: botón del enchufe en la barra inferior.

Velocidad: **115200 baud** (ya configurado en `platformio.ini`).

---

## D. Qué debería ver en el monitor serial si todo funciona

Al arrancar:

```
=== ESP32 DHT22 MQTT - Iniciando ===
[Config] SSID:          MiWiFi
[Config] Broker MQTT:   192.168.1.100
[Config] Puerto MQTT:   1883
[Config] Topico MQTT:   sensor/ambiente
[Config] Pin DHT22:     GPIO 4
[Config] Intervalo:     5000 ms
=====================================

[WiFi] Conectando a SSID: MiWiFi
......
[WiFi] Conectado. IP asignada: 192.168.1.105
[MQTT] Intentando conectar a 192.168.1.100:1883 (ClientID: ESP32-DHT22) ...
[MQTT] Conectado al broker.
```

Cada 5 segundos:

```
[DHT22] Temperatura: 24.5 C  Humedad: 60.2%
[MQTT] Publicando en 'sensor/ambiente': {"temperature":24.5,"humidity":60.2,"source":"esp32-dht22","uptime_ms":12345}
[MQTT] Publicacion exitosa.
```

---

## E. Levantar Docker Compose

```bash
docker compose up -d
```

Para ver los logs en tiempo real:

```bash
docker compose logs -f
```

---

## F. Verificar que los contenedores estén corriendo

```bash
docker compose ps
```

Deben aparecer con estado `running` (o `Up`):
- `mosquitto`
- `nodered`
- `influxdb`
- `grafana`

Si alguno está en `Exit` o `Restarting`:

```bash
docker compose logs mosquitto
docker compose logs nodered
```

---

## G. Suscribirse al tópico MQTT

### Opción 1: Usando el contenedor Mosquitto (recomendada, no requiere nada instalado)

```bash
docker compose exec mosquitto mosquitto_sub -h localhost -p 1883 -t sensor/ambiente -v
```

### Opción 2: Desde la PC (si tenés `mosquitto-clients` instalado)

```bash
mosquitto_sub -h 127.0.0.1 -p 1883 -t sensor/ambiente -v
```

> **Importante:** desde el ESP32, el broker **NO es `localhost`**. Debe ser la IP local de tu PC en la red WiFi. Para encontrarla:
> - Linux/Mac: `ip addr | grep 192.168` o `hostname -I`
> - Windows: `ipconfig` → buscar "Dirección IPv4"

---

## H. Qué debería ver con mosquitto_sub si todo funciona

```
sensor/ambiente {"temperature":24.5,"humidity":60.2,"source":"esp32-dht22","uptime_ms":12345}
sensor/ambiente {"temperature":24.6,"humidity":60.1,"source":"esp32-dht22","uptime_ms":17346}
```

Un mensaje nuevo cada 5 segundos.

---

## I. Probar publicación manual (sin ESP32)

Útil para verificar que Mosquitto y Node-RED funcionan aunque el ESP32 todavía no esté enviando.

### Terminal 1 — escuchar:

```bash
docker compose exec mosquitto mosquitto_sub -h localhost -p 1883 -t sensor/ambiente -v
```

### Terminal 2 — publicar:

```bash
docker compose exec mosquitto mosquitto_pub -h localhost -p 1883 -t sensor/ambiente \
  -m '{"temperature":25.5,"humidity":55.0,"source":"manual-test"}'
```

Si la terminal 1 muestra el mensaje, Mosquitto está funcionando correctamente.

---

## J. Ver mensajes en Node-RED

1. Abrir Node-RED: [http://localhost:1880](http://localhost:1880)
2. El flujo "Monitoreo Ambiental" tiene dos nodos de debug:
   - **Debug RAW (mensaje MQTT)**: muestra el string JSON exacto que llega del ESP32 o de la prueba manual. Conectado directamente al nodo MQTT in.
   - **Debug (post-InfluxDB format)**: muestra el objeto reformateado para InfluxDB.
3. Abrir el panel de debug (ícono de insecto en la barra derecha).
4. Al llegar un mensaje, debería verse algo como:
   ```
   msg.payload : string
   {"temperature":24.5,"humidity":60.2,"source":"esp32-dht22","uptime_ms":12345}
   ```

> Si los flujos no aparecen, el volumen Docker puede tener datos previos. Solución: `docker compose down -v` para borrar los volúmenes y volver a levantar, o importar manualmente el flujo desde `nodered/flows.json` (menú ☰ → Import).

---

## Problemas comunes

### El ESP32 muestra `NaN` en temperatura/humedad

| Causa posible | Verificación |
|---|---|
| Sensor mal conectado | Revisar que DATA vaya a GPIO 4 |
| Pin incorrecto en `config.h` | `DHT_PIN` debe ser `4` |
| Falta pull-up | Agregar resistencia 10kΩ entre DATA y VCC |
| Tipo de sensor incorrecto | `DHT_TYPE` debe ser `DHT22`, no `DHT11` |
| Alimentación incorrecta | Verificar que VCC reciba 3.3V |

### El ESP32 conecta a WiFi pero no conecta a MQTT

| Causa posible | Verificación |
|---|---|
| `MQTT_SERVER` incorrecto | No usar `localhost`; usar la IP local de la PC |
| Mosquitto no está corriendo | `docker compose ps` y `docker compose logs mosquitto` |
| Puerto 1883 bloqueado | `netstat -tlnp | grep 1883` o verificar firewall |
| PC y ESP32 en redes distintas | Ambos deben estar en la misma red WiFi |
| Firewall bloqueando | Permitir entrada al puerto 1883 en la PC |

En el monitor serial, el código de error de PubSubClient (`rc=`) indica el problema:

| rc | Significado |
|---|---|
| -4 | Timeout de conexión |
| -3 | Conexión rechazada |
| -2 | No se pudo conectar |
| -1 | Desconectado |
| 1 | Versión de protocolo incorrecta |
| 2 | Client ID rechazado |
| 3 | Servidor no disponible |
| 5 | No autorizado |

### El comando mosquitto_sub no muestra nada

| Causa posible | Verificación |
|---|---|
| El ESP32 no está publicando | Verificar monitor serial |
| Tópico incorrecto | El tópico en ESP32 y `mosquitto_sub` deben coincidir |
| Se está escuchando otro broker | Verificar la IP o el contenedor |
| Puerto no expuesto | Verificar que `docker compose ps` muestre `0.0.0.0:1883->1883` |

### Node-RED no recibe mensajes

| Causa posible | Verificación |
|---|---|
| Host incorrecto en el nodo MQTT in | Dentro de Docker debe ser `mosquitto`, no `localhost` |
| Tópico incorrecto | Debe ser `sensor/ambiente` |
| Nodo debug desactivado | Hacer click en el botón verde del nodo debug para activarlo |
| Flujo sin hacer deploy | Hacer click en "Deploy" después de cualquier cambio |
| Volumen con configuración vieja | `docker compose down -v` y volver a levantar |

---

## Datos llegan a Node-RED pero no aparecen en Grafana

### A. Confirmar que Node-RED recibe MQTT

En el panel debug de Node-RED (`http://localhost:1880`), el nodo **"Debug RAW (mensaje MQTT)"** debe mostrar el string JSON del ESP32 con cada publicación.

### B. Ver el payload antes de InfluxDB

El nodo **"Debug pre-InfluxDB (payload formateado)"** muestra exactamente lo que se intenta escribir en InfluxDB. Debe tener esta forma:

```json
[
  {
    "measurement": "ambiente",
    "fields": {
      "temperature": 24.5,
      "humidity": 60.2
    },
    "timestamp": "2024-01-01T00:00:00.000Z"
  }
]
```

Verificar que:
- `temperature` y `humidity` sean **números**, no strings
- `measurement` sea exactamente `ambiente`
- El array no esté vacío

### C. Ver errores de escritura en InfluxDB

El nodo **"ERROR InfluxDB"** captura cualquier error del nodo de salida. Si hay un problema de autenticación, conectividad o formato, aparecerá ahí.

También ver los logs de Node-RED:

```bash
docker compose logs -f nodered
```

Buscar líneas con `Error`, `401`, `403`, `influx`, `write`.

### D. Confirmar conectividad Node-RED → InfluxDB

Desde el contenedor de Node-RED:

```bash
docker compose exec nodered wget -qO- http://influxdb:8086/health
```

Debe responder `{"name":"influxdb","message":"ready for queries and writes","status":"pass",...}`.

Si falla, InfluxDB no está corriendo o no está en la misma red Docker.

### E. Verificar datos en InfluxDB desde la interfaz web

1. Abrir `http://localhost:8086`
2. Usuario: `admin` / Contraseña: `admin123456`
3. Ir a **Data Explorer** (ícono de cohete)
4. Seleccionar bucket **iot**
5. Filtrar por measurement **ambiente**
6. Si hay datos, los gráficos van a aparecer ahí

O usar la consola Flux (Script Editor):

```flux
from(bucket: "iot")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "ambiente")
```

Para ver solo temperatura:

```flux
from(bucket: "iot")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "ambiente")
  |> filter(fn: (r) => r._field == "temperature")
```

Para ver solo humedad:

```flux
from(bucket: "iot")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "ambiente")
  |> filter(fn: (r) => r._field == "humidity")
```

Si la consulta devuelve resultados en InfluxDB pero Grafana no muestra nada, el problema está en Grafana.

### F. Revisar el datasource en Grafana

1. Abrir `http://localhost:3000` (usuario: `admin`, contraseña: `admin`)
2. Ir a **Connections → Data sources**
3. Verificar que exista el datasource **InfluxDB**
4. Hacer click en él y verificar:
   - URL: `http://influxdb:8086`
   - Query language: **Flux**
   - Organization: `iot-org`
   - Token: `iot-token-123456`
   - Default bucket: `iot`
5. Hacer click en **Save & test** — debe decir "datasource is working"

Si el datasource no aparece o el test falla, reiniciar Grafana para que recargue el provisioning:

```bash
docker compose restart grafana
```

### G. Revisar el dashboard

1. Ir a **Dashboards → Monitoreo Ambiental - DHT22**
2. Si no aparece en la lista, buscarlo o ir a `http://localhost:3000/d/iot-ambiente-dht22`
3. Cambiar el rango de tiempo a **Last 15 minutes**
4. Si los paneles muestran "No data", abrir uno y editar la query

### H. Queries Flux correctas para Grafana

**Temperatura (gráfico de serie temporal):**
```flux
from(bucket: "iot")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r["_measurement"] == "ambiente")
  |> filter(fn: (r) => r["_field"] == "temperature")
```

**Humedad (gráfico de serie temporal):**
```flux
from(bucket: "iot")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r["_measurement"] == "ambiente")
  |> filter(fn: (r) => r["_field"] == "humidity")
```

**Temperatura actual (gauge):**
```flux
from(bucket: "iot")
  |> range(start: -10m)
  |> filter(fn: (r) => r["_measurement"] == "ambiente")
  |> filter(fn: (r) => r["_field"] == "temperature")
  |> last()
```

**Humedad actual (gauge):**
```flux
from(bucket: "iot")
  |> range(start: -10m)
  |> filter(fn: (r) => r["_measurement"] == "ambiente")
  |> filter(fn: (r) => r["_field"] == "humidity")
  |> last()
```

### I. Inserción manual de prueba desde Node-RED

Para verificar el pipeline sin el ESP32, publicar un mensaje manual con `mosquitto_pub`:

```bash
docker compose exec mosquitto mosquitto_pub -h localhost -p 1883 -t sensor/ambiente \
  -m '{"temperature":25.0,"humidity":50.0,"source":"manual-test"}'
```

Luego verificar en orden:
1. Que aparezca en el nodo "Debug RAW" de Node-RED
2. Que aparezca en el nodo "Debug pre-InfluxDB" con formato correcto
3. Que NO aparezca nada en "ERROR InfluxDB"
4. Que la query Flux en InfluxDB devuelva ese dato
5. Que el dashboard de Grafana lo muestre

### Pasos para aplicar el flows.json actualizado

Si se importó el flujo manualmente y ahora hay cambios en el repo, reimportarlo:

```bash
# Opción 1: borrar volumen y reiniciar (borra todo el historial de flows)
docker compose down
docker volume rm tp2_nodered-data
docker compose up -d

# Opción 2: importar manualmente el flows.json desde Node-RED
# Ir a http://localhost:1880 → menú (☰) → Import → subir nodered/flows.json
```
