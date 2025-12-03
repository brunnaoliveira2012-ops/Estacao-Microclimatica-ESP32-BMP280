// Código: ESP32-S3 + BMP280 -> ThingSpeak
// Bibliotecas necessárias: Adafruit BMP280, Adafruit Unified Sensor
// Configure: WIFI_SSID, WIFI_PASS, THINGSPEAK_WRITE_APIKEY, I2C SDA/SCL pins

#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

#define SEALEVELPRESSURE_HPA (1013.25)

const char* WIFI_SSID = "Seu wifi";
const char* WIFI_PASSWORD = "Sua senha";

// ThingSpeak
unsigned long myChannelNumber = 3185927;
const char* myWriteAPIKey = "RN5TG4ICNHXO1MRS";
const char* server = "http://api.thingspeak.com/update";

Adafruit_BMP280 bmp;

float lastPressure = 0;
unsigned long lastUpdate = 0;
const long updateInterval = 20000; // 20 segundos

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Conectando ao WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  if (!bmp.begin(0x76)) {
    Serial.println("Erro ao detectar o BMP280!");
    while (1);
  }
}

void loop() {
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    float temp = bmp.readTemperature();
    float press = bmp.readPressure() / 100.0F;
    float altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);

    // Tendência de chuva
    int chuva = 0;
    if (press < 1000) chuva = 1;  // 1 = possibilidade de chuva

    // Índice de conforto térmico simplificado (feels like)
    float conforto = temp - 0.55 * (1 - (press / 1013.25)) * (temp - 14.5);

    Serial.println("Enviando dados...");

    String url = String(server) +
                 "?api_key=" + myWriteAPIKey +
                 "&field1=" + String(temp) +
                 "&field2=" + String(press) +
                 "&field3=" + String(chuva) +
                 "&field4=" + String(altitude) +
                 "&field5=" + String(conforto);

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    http.end();

    Serial.print("Resposta HTTP: ");
    Serial.println(httpCode);

    Serial.print("Temp: "); Serial.println(temp);
    Serial.print("Pressão: "); Serial.println(press);
    Serial.print("Chuva? "); Serial.println(chuva);
    Serial.print("Altitude: "); Serial.println(altitude);
    Serial.print("Conforto: "); Serial.println(conforto);

    Serial.println("----------------------------");
  }
}
