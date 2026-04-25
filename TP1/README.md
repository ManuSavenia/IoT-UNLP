# Trabajo Práctico: Servidor Web asincrónico con ESP32

## Descripción

Este proyecto implementa un servidor web asincrónico sobre un ESP32 que permite:

* Visualizar temperatura y humedad en tiempo real
* Controlar el LED integrado del dispositivo desde una interfaz web
* Configurar la conexión WiFi dinámicamente mediante portal cautivo
* Consultar un historial de mediciones

La solución integra lectura de sensores, conectividad WiFi, servidor web y una interfaz de usuario accesible desde navegador, cumpliendo con los requisitos del trabajo práctico .

---

## Requisitos

* ESP32
* Sensor DHT22
* PlatformIO (extensión en Visual Studio Code)

---

## Ejecución del proyecto (PlatformIO en VS Code)

Este proyecto está pensado para ejecutarse utilizando la extensión **PlatformIO** dentro de **Visual Studio Code**.

### 🔹 Paso 1 – Abrir el proyecto

1. Abrir Visual Studio Code
2. Seleccionar **"Open Folder"**
3. Elegir la carpeta del proyecto

PlatformIO detectará automáticamente la configuración (`platformio.ini`).

---

### 🔹 Paso 2 – Subir el filesystem (SPIFFS)

Este paso es **obligatorio**, ya que la interfaz web se encuentra almacenada en SPIFFS.

1. Abrir la barra de PlatformIO (lado izquierdo)
2. Ir a:

   ```
   Project Tasks → esp32dev → Platform → Build Filesystem Image
   ```
3. Luego ejecutar:

   ```
   Upload Filesystem Image
   ```

Esto carga los archivos HTML/CSS/JS en la memoria del ESP32.

---

### 🔹 Paso 3 – Compilar y cargar el firmware

1. Ejecutar:

   ```
   Build
   ```
2. Luego:

   ```
   Upload
   ```

---

### 🔹 Paso 4 – Monitor serie (opcional)

Para visualizar logs del sistema:

```
Monitor
```

Aquí se puede observar:

* Conexión WiFi
* Dirección IP asignada
* Lecturas del sensor

---

## Alternativa por consola

En caso de no usar la interfaz gráfica:

```bash
pio run --target buildfs
pio run --target uploadfs
pio run
pio run --target upload
pio device monitor
```

---

## Configuración WiFi (WiFiManager)

El sistema implementa conexión automática mediante **WiFiManager**:

* Si existen credenciales guardadas → se conecta automáticamente
* Si no existen o fallan → se inicia un **portal cautivo**

### Primer uso

1. El ESP32 crea una red WiFi:

   ```
   SSID: ESP32-IoT-Setup
   Password: 12345678
   ```

   (definido en config )

2. Conectarse desde un celular o PC

3. Se abrirá automáticamente una página de configuración

4. Seleccionar red WiFi e ingresar contraseña

5. El dispositivo se conectará y almacenará las credenciales

---

## Acceso a la interfaz web

Una vez conectado el ESP32 a la red:

1. Consultar la IP en el monitor serie
2. Ingresar en el navegador:

```
http://<IP_DEL_ESP>
```

---

## Funcionalidades implementadas

### Control de LED

* Botón en la interfaz web
* Endpoint: `/toggle`
* Permite encender/apagar el LED integrado

---

### Lectura de sensores

* Sensor: DHT22
* Variables:

  * Temperatura
  * Humedad
* Lectura periódica no bloqueante (~2.5 s)
* **Importante:** el sensor DHT22 requiere una **resistencia pull-up externa de aproximadamente 10kΩ entre el pin de datos (DATA) y VCC**. Esto se debe a que la línea de datos funciona en lógica activa en bajo: el sensor fuerza el pin a GND para representar un ‘0’, pero cuando no transmite debe permanecer en nivel alto. Sin la resistencia pull-up, la línea quedaría flotante y la comunicación sería inestable o directamente fallaría.

---

### Visualización en tiempo real

* Datos actualizados automáticamente desde el frontend
* Estado del sensor
* Red WiFi activa

---

### Historial de mediciones

* Buffer circular de hasta 20 registros
* Endpoint REST:

  ```
  /api/metrics
  ```
* Visualización en tabla en la interfaz web

---

## Arquitectura del sistema

* `main.cpp`: inicialización general
* `sensor.h`: lectura y almacenamiento de datos
* `led.h`: control del LED
* `web.h`: servidor web y endpoints
* `wifi_manager_setup.h`: configuración WiFi
* `SPIFFS`: almacenamiento de la interfaz web

---

## Conexionado

* LED → pin definido como `LED_PIN`
* DHT22 → pin `DHT_PIN`

---

## Evidencias (capturas y videos)

Se incluyen en la carpeta `docs/` los recursos solicitados por la consigna:

### Capturas

* `docs/images/dashboard.png`
  → Interfaz web mostrando temperatura, humedad y control de LED

* `docs/images/conexionado.png`
  → Esquema/conexión física del ESP32 y sensor

---

### Videos

* `docs/videos/toggle-led.mp4`
  → Demostración del control del LED desde la web

* `docs/videos/wifi-manager.mp4`
  → Configuración de red mediante portal cautivo desde el celular

---