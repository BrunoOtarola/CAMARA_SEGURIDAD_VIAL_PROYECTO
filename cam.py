import cv2
import numpy as np
import urllib.request
import time

# Dirección IP del modo AP del ESP32-CAM
URL = "http://192.168.4.1/capture"

# FACTOR_ESCALA: Cuántos centímetros reales representa UN píxel en la imagen.
FACTOR_ESCALA = 0.2  

# Parámetros para la detección de puntos clave (Shi-Tomasi)
feature_params = dict(maxCorners=15, qualityLevel=0.3, minDistance=10, blockSize=7)

# Parámetros para el flujo óptico Lucas-Kanade
lk_params = dict(winSize=(15, 15), maxLevel=2,
                 criteria=(cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 0.03))

print("Conectando con el ESP32-CAM... Presiona 'q' para salir.")

old_gray = None
p0 = None
last_time = time.time()

while True:
    try:
        # 1. Descargar frame desde el ESP32 (Añadido un timeout corto para evitar congelamientos)
        img_resp = urllib.request.urlopen(URL, timeout=2)
        img_np = np.asarray(bytearray(img_resp.read()), dtype=np.uint8)
        frame = cv2.imdecode(img_np, cv2.IMREAD_COLOR)
        
        if frame is None:
            continue
            
        current_time = time.time()
        dt = current_time - last_time  # Tiempo transcurrido
        last_time = current_time

        # 2. Convertir a escala de grises
        frame_gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        if old_gray is None:
            old_gray = frame_gray.copy()
            p0 = cv2.goodFeaturesToTrack(old_gray, mask=None, **feature_params)
            continue

        # 3. Calcular el Flujo Óptico
        if p0 is not None and len(p0) > 0:
            p1, st, err = cv2.calcOpticalFlowPyrLK(old_gray, frame_gray, p0, None, **lk_params)

            if p1 is not None:
                good_new = p1[st == 1]
                good_old = p0[st == 1]

                velocidades_kmh = []
                for i, (new, old) in enumerate(zip(good_new, good_old)):
                    a, b = new.ravel()
                    c, d = old.ravel()
                    
                    # Distancia Euclidiana en píxeles
                    distancia_pixeles = np.sqrt((a - c)**2 + (b - d)**2)
                    
                    if dt > 0:
                        distancia_real_cm = distancia_pixeles * FACTOR_ESCALA
                        velocidad_cms = distancia_real_cm / dt  # cm/s
                        
                        # EXPLICACIÓN CONVERSIÓN A KM/H:
                        # (cm/s) * (1 m / 100 cm) * (1 km / 1000 m) * (3600 s / 1 h) => Multiplicar por 0.036
                        velocidad_kmh = velocidad_cms * 0.036
                        velocidades_kmh.append(velocidad_kmh)
                    
                    # Dibujar los puntos en la pantalla
                    frame = cv2.circle(frame, (int(a), int(b)), 5, (0, 0, 255), -1)

                # Si detectamos movimiento
                if len(velocidades_kmh) > 0:
                    vel_promedio_kmh = np.mean(velocidades_kmh)
                    
                    # Ajustamos el umbral de ruido a km/h (ej: 0.1 km/h es casi estático)
                    if vel_promedio_kmh > 0.1:  
                        print(f"Velocidad detectada: {vel_promedio_kmh:.2f} km/h")
                        
                        # Lógica del semáforo (ejemplo: Alerta si pasa de 40 km/h)
                        if vel_promedio_kmh > 40.0: 
                            print("¡ALERTA! Velocidad superior a 40 km/h, activar semáforo.")
                            cv2.putText(frame, "ALERTA SEMAFORO", (20, 50), 
                                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 3)

                p0 = good_new.reshape(-1, 1, 2)
            else:
                p0 = None
        else:
            p0 = cv2.goodFeaturesToTrack(frame_gray, mask=None, **feature_params)

        old_gray = frame_gray.copy()
        
        # Mostrar velocidad en pantalla para testear más fácil
        if 'vel_promedio_kmh' in locals():
            cv2.putText(frame, f"{vel_promedio_kmh:.1f} km/h", (20, 100), 
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

        cv2.imshow("Detector de Velocidad ESP32-CAM", frame)

    except Exception as e:
        print(f"Error en el ciclo: {e}")
        old_gray = None

    # Reducimos drásticamente el delay de renderizado para acelerar el bucle
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()