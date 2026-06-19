#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// ================= CONFIGURACIÓN DEL PUNTO DE ACCESO (AP) =================
// Tu notebook se conectará directamente a esta red Wi-Fi
const char* ssid_AP = "ESP32_Camara_Directa"; 
const char* password_AP = "123456789";        // Mínimo 8 caracteres

WebServer server(80);

// ================= PINOUT MODELO AI THINKER =================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Función que toma la foto y la envía cuando Python la solicita
void handle_capture() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Fallo al capturar imagen");
    return;
  }
  
  // Enviamos los bytes puros del JPG
  server.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
  server.setContentLength(fb->len);
  server.send(200, "image/jpeg", "");
  
  WiFiClient client = server.client();
  client.write(fb->buf, fb->len);
  
  // Liberar la memoria del buffer de la cámara
  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  
  // Configuración de la estructura de la cámara
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Ajustes de resolución óptimos para no saturar el AP
  config.frame_size = FRAMESIZE_VGA;  // 640x480
  config.jpeg_quality = 12;           // Calidad buena (rango 0-63)
  config.fb_count = 1;

  // Inicializar la cámara física
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error al iniciar cámara: 0x%x\n", err);
    return;
  }

  // ================= INICIAR MODO PUNTO DE ACCESO =================
  Serial.println("Configurando Punto de Acceso...");
  WiFi.softAP(ssid_AP, password_AP);

  Serial.println("");
  Serial.println("Punto de Acceso listo.");
  Serial.print("Red Wi-Fi: ");
  Serial.println(ssid_AP);
  Serial.print("Dirección IP para Python: ");
  Serial.println(WiFi.softAPIP()); // Casi siempre mostrará 192.168.4.1

  // Enlazar la ruta /capture con nuestra función
  server.on("/capture", handle_capture);
  
  // Iniciar el servidor web nativo
  server.begin();
  Serial.println("Servidor web en ejecución.");
}

void loop() {
  // Mantener el servidor escuchando peticiones de Python
  server.handleClient();
}