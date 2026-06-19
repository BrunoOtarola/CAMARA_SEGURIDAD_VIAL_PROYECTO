# Sistema de Camara de Seguridad Vial con ESP32-CAM

Sistema de deteccion de velocidad vehicular en tiempo real usando una ESP32-CAM como servidor de video y Python con OpenCV para el procesamiento de imagenes. Incluye ademas el firmware de un semaforo de dos canales (vehiculos y bicicletas) controlado por Arduino.

---

## Descripcion general

El sistema opera en modo Punto de Acceso (AP): la ESP32-CAM crea su propia red Wi-Fi a la que se conecta el equipo que ejecuta el script de Python. El script descarga frames continuamente, aplica flujo optico Lucas-Kanade para estimar la velocidad de los objetos en movimiento y emite una alerta cuando se supera el umbral configurado.

---

## Componentes del proyecto

| Archivo | Descripcion |
|---|---|
| `cam.ino` | Firmware para la ESP32-CAM (modelo AI Thinker). Levanta un servidor HTTP en modo AP y expone el endpoint `/capture` que devuelve un frame JPEG. |
| `cam.py` | Script Python que consume el endpoint, calcula la velocidad mediante flujo optico y muestra el resultado en pantalla. |
| `semaforo.ino` | Firmware Arduino para un semaforo de dos canales (vehiculos y bicicletas) con ciclo rojo/amarillo/verde. |
| `AP.txt` | Referencia de instalacion de dependencias y enlace de referencia. |

---

## Requisitos de hardware

- ESP32-CAM (modelo AI Thinker)
- Arduino (cualquier modelo compatible con 6 salidas digitales)
- 6 LEDs: rojo, amarillo y verde para vehiculos; rojo, amarillo y verde para bicicletas
- Resistencias limitadoras de corriente para los LEDs
- Computador con Python 3.x

---

## Requisitos de software

```bash
pip install opencv-python numpy
```

---

## Configuracion

### ESP32-CAM (`cam.ino`)

| Parametro | Valor por defecto | Descripcion |
|---|---|---|
| `ssid_AP` | `ESP32_Camara_Directa` | Nombre de la red Wi-Fi AP |
| `password_AP` | `123456789` | Contrasena de la red (minimo 8 caracteres) |
| `FRAMESIZE_VGA` | 640x480 | Resolucion del frame |
| `jpeg_quality` | 12 | Calidad JPEG (0 = maxima, 63 = minima) |

La IP del servidor siempre sera `192.168.4.1` al operar en modo AP.

### Script Python (`cam.py`)

| Parametro | Valor por defecto | Descripcion |
|---|---|---|
| `URL` | `http://192.168.4.1/capture` | Endpoint de captura de la ESP32-CAM |
| `FACTOR_ESCALA` | `0.2` | Centimetros reales por pixel en la imagen |
| Umbral de alerta | `40.0 km/h` | Velocidad a partir de la cual se genera alerta |
| Umbral de ruido | `0.1 km/h` | Velocidades menores a este valor se ignoran |

**Ajuste de `FACTOR_ESCALA`:** mida un objeto de tamano conocido en la escena y cuente cuantos pixeles ocupa. Divida la medida real en centimetros entre los pixeles para obtener el factor correcto.

### Semaforo (`semaforo.ino`)

| Pin | Funcion |
|---|---|
| 2 | LED Rojo - vehiculos |
| 3 | LED Amarillo - vehiculos |
| 4 | LED Verde - vehiculos |
| 8 | LED Rojo - bicicletas |
| 9 | LED Amarillo - bicicletas |
| 10 | LED Verde - bicicletas |

Tiempos del ciclo:
- Rojo vehiculos / Verde bicicletas: 10 segundos
- Amarillo (ambos canales): 2.5 segundos
- Verde vehiculos / Rojo bicicletas: 15 segundos

---

## Uso

1. Cargar `cam.ino` en la ESP32-CAM con el IDE de Arduino.
2. Conectar el equipo a la red Wi-Fi `ESP32_Camara_Directa`.
3. Ejecutar el script de deteccion:

```bash
python cam.py
```

4. La ventana mostrara el video con los puntos de seguimiento superpuestos y la velocidad estimada en km/h. Presionar `q` para salir.
5. (Opcional) Cargar `semaforo.ino` en el Arduino con los LEDs conectados segun la tabla de pines anterior.

---

## Funcionamiento del algoritmo de velocidad

1. Se descarga un frame JPEG desde `http://192.168.4.1/capture`.
2. Se detectan hasta 15 puntos de interes con el metodo Shi-Tomasi.
3. Se calcula el flujo optico entre el frame anterior y el actual usando Lucas-Kanade.
4. La distancia en pixeles de cada punto se convierte a centimetros mediante `FACTOR_ESCALA` y se divide por el tiempo transcurrido para obtener cm/s.
5. Se convierte a km/h multiplicando por `0.036`.
6. Si la velocidad promedio supera los 40 km/h, se imprime una alerta en consola y se superpone el texto `ALERTA SEMAFORO` en el video.

---

## Notas

- La precision de la medicion depende directamente de la calibracion de `FACTOR_ESCALA` y de la estabilidad de la camara.
- El timeout de 2 segundos en `urllib.request.urlopen` evita que el bucle se congele ante perdidas de conexion.
- El proyecto fue desarrollado como prototipo de bajo costo para validacion del concepto.
