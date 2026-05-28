# Flujo de datos del sistema

## Recorrido completo

```
ESP32 (DHT22)
    |
    | Lee temperatura y humedad cada N segundos
    v
ESP32 arma JSON: {"temperature": 24.5, "humidity": 60.2}
    |
    | Publica por MQTT en el tópico "sensor/ambiente"
    v
Mosquitto (broker MQTT, puerto 1883)
    |
    | Distribuye el mensaje a los suscriptores
    v
Node-RED (suscrito a "sensor/ambiente")
    |
    | Nodo JSON: convierte el string a objeto JavaScript
    | Nodo Function: formatea el objeto para InfluxDB
    v
InfluxDB 2.x (puerto 8086)
    |
    | Almacena en bucket "iot", measurement "ambiente"
    | campos: temperature, humidity
    v
Grafana (puerto 3000)
    |
    | Consulta InfluxDB usando Flux
    | Muestra gráficos en función del tiempo
```

---

## Detalle de cada etapa

### 1. ESP32 + DHT22

El ESP32 ejecuta un loop que, cada `PUBLISH_INTERVAL_MS` milisegundos:
1. Lee temperatura y humedad del DHT22.
2. Valida que los valores no sean NaN.
3. Construye el JSON como string.
4. Publica en el tópico MQTT configurado.

Si el WiFi o la conexión MQTT se cae, el código intenta reconectarse automáticamente antes de cada publicación.

### 2. Mosquitto

Broker MQTT liviano configurado para aceptar conexiones anónimas en el puerto 1883. Dentro de la red Docker, su hostname es `mosquitto`. Desde la PC o el ESP32, se accede por la IP de la máquina que corre Docker.

### 3. Node-RED

El flujo tiene cuatro nodos encadenados:

**MQTT in**
- Se conecta al broker Mosquitto usando el hostname `mosquitto` y puerto `1883`.
- Está suscrito al tópico `sensor/ambiente`.
- Cuando llega un mensaje, lo pasa al siguiente nodo como `msg.payload`.

**JSON**
- Convierte el payload de string a objeto JavaScript.
- Después de este nodo, `msg.payload.temperature` y `msg.payload.humidity` son números.

**Function**
- Valida que los valores sean numéricos.
- Construye el array que espera el nodo InfluxDB:

```javascript
msg.payload = [
    {
        measurement: 'ambiente',
        fields: {
            temperature: temp,
            humidity: hum
        },
        timestamp: new Date()
    }
];
```

**InfluxDB out**
- Usa el paquete `node-red-contrib-influxdb` (ya instalado en la imagen Docker).
- Se conecta a `http://influxdb:8086`.
- Escribe en el bucket `iot`, organización `iot-org`, con el token configurado.

### 4. InfluxDB

Los datos se guardan en:
- **Bucket:** `iot`
- **Measurement:** `ambiente`
- **Fields:** `temperature` (float), `humidity` (float)
- **Timestamp:** provisto por Node-RED al momento de recibir el mensaje.

Para verificar datos desde la UI de InfluxDB (http://localhost:8086):
1. Ir a Data Explorer.
2. Seleccionar bucket `iot`.
3. Filtrar por measurement `ambiente`.
4. Elegir el campo que quieran ver.

También se puede usar la consola de Script con Flux:

```flux
from(bucket: "iot")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "ambiente")
```

### 5. Grafana

Grafana se conecta a InfluxDB como datasource usando el lenguaje Flux. El dashboard provisionado automáticamente incluye:
- Gráfico de temperatura en función del tiempo.
- Gráfico de humedad en función del tiempo.
- Gauge de temperatura actual (último valor).
- Gauge de humedad actual (último valor).

Los paneles se actualizan cada 10 segundos. El rango de tiempo por defecto es la última hora.

---

## Notas sobre el flujo en Node-RED

Si el flujo no aparece al entrar a Node-RED, puede deberse a que el volumen Docker ya tenía datos previos y sobreescribió el `flows.json`. En ese caso:

1. Borrar el volumen: `docker compose down -v` y volver a levantarlo.
2. O importar el flujo manualmente: abrir Node-RED → menú (☰) → Import → pegar el contenido de `nodered/flows.json`.

Si los nodos de InfluxDB aparecen como "desconocidos" (recuadros grises), el paquete `node-red-contrib-influxdb` no está instalado. Esto no debería pasar si se usa la imagen Docker del proyecto (que lo instala en el `Dockerfile`). Si se usa Node-RED directamente, instalarlo desde el menú Manage Palette.
