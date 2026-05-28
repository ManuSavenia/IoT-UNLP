# Estación de Monitoreo Ambiental IoT

Trabajo Práctico — IoT | UNLP Ingeniería

## Objetivo

Implementar un sistema de monitoreo ambiental que lee temperatura y humedad desde un sensor DHT22 conectado a un ESP32, publica los datos por MQTT y los visualiza en tiempo real con Grafana.

## Arquitectura

```
ESP32 + DHT22  →  MQTT/Mosquitto  →  Node-RED  →  InfluxDB  →  Grafana
```

Todos los servicios del servidor (Mosquitto, Node-RED, InfluxDB, Grafana) corren en Docker en la misma PC de desarrollo.

## Flujo de datos

1. El ESP32 lee temperatura y humedad del DHT22 cada 5 segundos.
2. Publica un JSON en el tópico MQTT `sensor/ambiente`.
3. Node-RED recibe el mensaje, lo parsea y lo guarda en InfluxDB.
4. Grafana consulta InfluxDB y muestra los datos en un dashboard.

Para más detalle, ver [docs/flujo.md](docs/flujo.md).

---

## Requisitos previos

- Docker y Docker Compose instalados.
- VS Code con la extensión PlatformIO instalada.
- ESP32 con el sensor DHT22 conectado.
- Python 3 (requerido por PlatformIO, generalmente ya presente).

---

## Levantar los servicios

Desde la raíz del proyecto:

```bash
docker compose up -d
```

Esto descarga las imágenes necesarias (puede tardar algunos minutos la primera vez) y construye la imagen de Node-RED con el paquete de InfluxDB incluido.

Verificar que los contenedores estén corriendo:

```bash
docker compose ps
```

Deberían aparecer cuatro servicios con estado `Up`: `mosquitto`, `nodered`, `influxdb`, `grafana`.

### Accesos

| Servicio  | URL                   | Usuario | Contraseña |
|-----------|-----------------------|---------|------------|
| Node-RED  | http://localhost:1880 | —       | —          |
| InfluxDB  | http://localhost:8086 | admin   | admin123456 |
| Grafana   | http://localhost:3000 | admin   | admin      |

---

## Configurar y flashear el ESP32

### 1. Copiar el archivo de configuración

```bash
cp include/config.example.h include/config.h
```

Editar `include/config.h` y completar:

```cpp
#define WIFI_SSID     "nombre_de_tu_red"
#define WIFI_PASSWORD "contraseña_de_tu_red"
#define MQTT_SERVER   "192.168.1.100"  // IP de tu PC en la red local
```

> **Importante:** `MQTT_SERVER` debe ser la IP de la PC donde corre Docker en tu red local, **no** `localhost` ni `127.0.0.1`. Esas direcciones apuntan al propio ESP32.
>
> Para saber la IP de tu PC en Linux: `ip addr show | grep "inet "` o `hostname -I`.

### 2. Conexión del DHT22

| Pin DHT22 | Conexión ESP32       |
|-----------|----------------------|
| VCC       | 3.3V                 |
| GND       | GND                  |
| DATA      | GPIO 4               |

Si el módulo DHT22 no trae resistencia pull-up integrada, colocar una resistencia de 10kΩ entre VCC y DATA.

### 3. Compilar y flashear desde VS Code

1. Abrir VS Code en la carpeta del proyecto.
2. Instalar la extensión PlatformIO si no está instalada.
3. Conectar el ESP32 por USB.
4. Usar los botones de la barra inferior de PlatformIO:
   - **Build** (✓) para compilar.
   - **Upload** (→) para flashear.
   - **Monitor** (enchufe) para ver los logs por Serial.

O desde la terminal:

```bash
pio run --target upload
pio device monitor
```

Los logs deben mostrar algo como:

```
[WiFi] Conectando a MiRed... conectado. IP: 192.168.1.55
[MQTT] Conectando a 192.168.1.100:1883 ... conectado.
[DHT22] Temperatura: 23.4°C  Humedad: 58.7%
[MQTT] Publicado en sensor/ambiente: {"temperature":23.4,"humidity":58.7}
```

---

## Probar MQTT manualmente

Suscribirse al tópico para ver los mensajes que publica el ESP32:

```bash
mosquitto_sub -h localhost -p 1883 -t "sensor/ambiente"
```

Publicar un mensaje de prueba manualmente (para probar Node-RED sin el ESP32):

```bash
mosquitto_pub -h localhost -p 1883 -t "sensor/ambiente" -m '{"temperature":25.0,"humidity":60.0}'
```

Si `mosquitto_sub` y `mosquitto_pub` no están instalados localmente, se puede ejecutar desde el contenedor:

```bash
docker exec -it mosquitto mosquitto_pub -h localhost -p 1883 -t "sensor/ambiente" -m '{"temperature":25.0,"humidity":60.0}'
```

---

## Verificar el flujo en Node-RED

1. Abrir http://localhost:1880.
2. El flujo `Monitoreo Ambiental` debería estar cargado automáticamente.
3. Si no aparece: menú (☰) → Import → seleccionar `nodered/flows.json` → Deploy.
4. Hacer clic en **Deploy** (botón rojo arriba a la derecha) para activar el flujo.
5. En la pestaña Debug (ícono de insecto), deberían aparecer los mensajes cada vez que llega un dato de MQTT.
6. El nodo `MQTT in` debe mostrar el estado `connected` bajo su nombre.

### Configurar el token de InfluxDB en Node-RED (paso obligatorio)

El token no viaja en el `flows.json` por razones de seguridad. Hay que setearlo manualmente la primera vez:

1. Doble click en el nodo **"Guardar en InfluxDB"** (nodo naranja).
2. Click en el **ícono de lápiz** junto al campo "Server".
3. En el campo **Token** escribir: `iot-token-123456`
4. Click en **Update** → **Done** → **Deploy**.

> Si los nodos de InfluxDB aparecen como bloques grises desconocidos, el paquete `node-red-contrib-influxdb` no se instaló correctamente. Ejecutar `docker compose build nodered` y volver a levantar.

---

## Verificar datos en InfluxDB

1. Abrir http://localhost:8086 e iniciar sesión (admin / admin123456).
2. Ir a **Data Explorer** en el menú lateral.
3. Seleccionar bucket `iot` → measurement `ambiente`.
4. Elegir el campo `temperature` o `humidity`.
5. Hacer clic en **Submit** para ver los datos.

También se puede usar el editor de scripts Flux:

```flux
from(bucket: "iot")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "ambiente")
```

---

## Grafana: datasource y dashboard

### Datasource

El datasource de InfluxDB se provisiona automáticamente al levantar los contenedores. Para verificarlo:

1. Abrir http://localhost:3000 (admin / admin).
2. Ir a Connections → Data sources.
3. Debe aparecer `InfluxDB` configurado con lenguaje Flux.
4. Hacer clic en **Test** para verificar la conexión.

Si hay que crearlo manualmente:

- **Type:** InfluxDB
- **Query Language:** Flux
- **URL:** `http://influxdb:8086`
- **Organization:** `iot-org`
- **Token:** `iot-token-123456`
- **Default Bucket:** `iot`

### Dashboard

El dashboard `Monitoreo Ambiental - DHT22` también se provisiona automáticamente. Para encontrarlo:

1. Ir a Dashboards en el menú lateral.
2. Abrir la carpeta `IoT`.
3. Hacer clic en `Monitoreo Ambiental - DHT22`.

El dashboard muestra:
- Gráfico de temperatura en función del tiempo.
- Gráfico de humedad en función del tiempo.
- Gauge de temperatura actual.
- Gauge de humedad actual.

Se actualiza automáticamente cada 10 segundos.

---

## Bajar el proyecto

### Detener los contenedores (conserva todos los datos)

```bash
docker compose down
```

Los volúmenes de InfluxDB, Grafana y Node-RED se conservan. Al volver a levantar con `docker compose up -d` todo continúa desde donde estaba.

### Detener y borrar todos los datos

```bash
docker compose down -v
```

Borra los volúmenes junto con los contenedores. La próxima vez que se levante el stack arranca completamente limpio (sin historial en InfluxDB, sin flows en Node-RED, sin configuración en Grafana). Requiere volver a importar el flujo en Node-RED y configurar el token nuevamente.

---

## Problemas comunes

**El ESP32 no conecta a WiFi**
- Verificar que el SSID y la contraseña en `config.h` sean correctos.
- El ESP32 solo soporta redes 2.4 GHz, no 5 GHz.

**El ESP32 no conecta a MQTT**
- Confirmar que `MQTT_SERVER` sea la IP local de la PC (no `localhost`).
- Verificar que el contenedor `mosquitto` esté corriendo: `docker compose ps`.
- Verificar que el puerto 1883 no esté bloqueado por el firewall de la PC.
  En Linux: `sudo ufw allow 1883` si UFW está activo.

**El DHT22 devuelve NaN**
- Verificar las conexiones físicas del sensor.
- Confirmar que `DHT_PIN` en `config.h` coincide con el pin físico usado.
- Si el módulo no tiene resistencia pull-up integrada, agregar una de 10kΩ.
- Puede tardar hasta 2 segundos en dar la primera lectura válida tras encender.

**Node-RED no guarda datos en InfluxDB**
- Verificar que el flujo esté desplegado (botón Deploy).
- Revisar la pestaña Debug en Node-RED para ver si hay errores.
- Confirmar que el contenedor `influxdb` esté corriendo.
- Verificar que el token en el nodo InfluxDB de Node-RED sea `iot-token-123456`.

**Los nodos de InfluxDB en Node-RED aparecen grises**
- La imagen Docker de Node-RED no se construyó correctamente.
- Ejecutar: `docker compose build nodered` y luego `docker compose up -d`.

**Grafana no conecta a InfluxDB**
- Ir a Connections → Data sources → InfluxDB → Test.
- Asegurarse de que el lenguaje de query sea **Flux** (no InfluxQL).
- La URL debe ser `http://influxdb:8086` (hostname del contenedor, no localhost).

**El dashboard de Grafana no muestra datos**
- Verificar que haya datos en InfluxDB primero (Data Explorer).
- Ampliar el rango de tiempo en Grafana (esquina superior derecha).
- Confirmar que el datasource `InfluxDB` esté configurado correctamente.

---

## Estructura del proyecto

```
.
├── docker-compose.yml
├── platformio.ini
├── README.md
├── src/
│   └── main.cpp                # Código del ESP32
├── include/
│   └── config.example.h        # Plantilla de configuración
├── data/                       # Archivos para SPIFFS (no usado en esta versión)
├── mosquitto/
│   └── config/
│       └── mosquitto.conf      # Configuración del broker MQTT
├── nodered/
│   ├── Dockerfile              # Imagen con node-red-contrib-influxdb
│   └── flows.json              # Flujo MQTT → JSON → InfluxDB
├── grafana/
│   └── provisioning/
│       ├── datasources/
│       │   └── datasource.yml  # Datasource InfluxDB (Flux)
│       └── dashboards/
│           ├── dashboard.yml   # Proveedor de dashboards
│           └── ambient.json    # Dashboard de temperatura y humedad
└── docs/
    └── flujo.md                # Descripción detallada del flujo de datos
```
