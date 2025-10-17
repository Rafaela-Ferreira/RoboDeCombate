#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "esp_http_server.h"

// ===== CONFIGURAÇÃO DO MÓDULO ESP32-CAM (AI Thinker) =====
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

// ===== CONFIGURAÇÃO DO WI-FI =====
const char* ssid = "ESP32CAM_AP";
const char* password = "12345678";

// ===== VARIÁVEIS GLOBAIS =====
httpd_handle_t stream_httpd = NULL;
camera_config_t config;

// ===== FUNÇÃO DE STREAMING =====
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  char part_buf[64];
  static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=frame";
  static const char* _STREAM_BOUNDARY = "\r\n--frame\r\n";
  static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("⚠️ Frame nulo — reiniciando câmera...");
      esp_camera_deinit();
      delay(100);
      if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("❌ Falha ao reiniciar câmera");
        delay(500);
      }
      continue;
    }

    size_t hlen = snprintf(part_buf, sizeof(part_buf), _STREAM_PART, fb->len);
    res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, part_buf, hlen);
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    if (res != ESP_OK) break;

    esp_camera_fb_return(fb);
  }

  if(fb) esp_camera_fb_return(fb);
  return res;
}

// ===== FUNÇÃO PRINCIPAL =====
void startCameraServer() {
  httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
  http_config.server_port = 80;

  httpd_uri_t stream_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };

  if (httpd_start(&stream_httpd, &http_config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    Serial.println("🌐 Servidor iniciado! Acesse o IP no navegador para ver a câmera.");
  } else {
    Serial.println("❌ Falha ao iniciar servidor HTTP");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!psramFound()) {
    Serial.println("⚠️ PSRAM não detectada — vídeo pode travar!");
  }

  Serial.println("\n=== INICIANDO ESP32-CAM ===");

  // --- Configuração da câmera ---
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000; // estabilidade
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 20;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 25;
    config.fb_count = 1;
  }

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("❌ Erro ao iniciar câmera");
    return;
  }
  Serial.println("✅ Câmera iniciada com sucesso!");

  // --- Inicia Access Point ---
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();

  Serial.println("=== Wi-Fi Access Point iniciado ===");
  Serial.print("📶 SSID: "); Serial.println(ssid);
  Serial.print("🔑 Senha: "); Serial.println(password);
  Serial.print("🌐 IP do ESP32-CAM: "); Serial.println(IP);

  // --- Inicia servidor ---
  startCameraServer();
}

unsigned long ultimoReset = 0;
void loop() {
  if (millis() - ultimoReset > 600000) { // 10 minutos
    Serial.println("♻️ Reiniciando preventivamente...");
    ESP.restart();
  }
}
