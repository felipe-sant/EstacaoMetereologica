#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

// Configuração Wi-Fi
const char* ssid = "Familia Machado";
const char* password = "963214785";

// DHT11
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQ-7
#define MQ7_PIN 35

// Sensor de chuva
#define RAIN_PIN 25

// BMP280
Adafruit_BMP280 bmp;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Conectando-se ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());

  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();

  pinMode(RAIN_PIN, INPUT);

  if (!bmp.begin(0x76)) {
    Serial.println("❌ Erro ao iniciar o BMP280!");
  } else {
    Serial.println("✅ BMP280 iniciado.");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    float h = dht.readHumidity();
    float t_bmp = bmp.readTemperature();  // usar essa como temperatura principal
    int mq7Value = analogRead(MQ7_PIN);
    int chuva = digitalRead(RAIN_PIN);

    // Log no Serial Monitor
    Serial.println("======= Dados Lidos =======");
    Serial.print("Temperatura (BMP280): ");
    Serial.print(t_bmp);
    Serial.println(" °C");

    Serial.print("Umidade (DHT11): ");
    Serial.print(h);
    Serial.println(" %");

    Serial.print("CO (MQ-7): ");
    Serial.println(mq7Value);

    Serial.print("Sensor de chuva: ");
    Serial.println(chuva == 0 ? "☔ Chuva detectada" : "🌤️ Seco");
    Serial.println("===========================");

    // Enviar para a API
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, "https://coletor-de-tempo.vercel.app/api");
    http.addHeader("Content-Type", "application/json");

    String postData = "{";
    postData += "\"temperatura\":" + String(t_bmp) + ","; // usa BMP280 como temperatura
    postData += "\"umidade\":" + String(h) + ",";
    postData += "\"co\":" + String(mq7Value) + ",";
    postData += "\"chuva\":" + String(chuva);
    postData += "}";

    int httpResponseCode = http.POST(postData);

    Serial.print("Código de resposta da API: ");
    Serial.println(httpResponseCode);

    String response = http.getString();
    Serial.println("Resposta da API: " + response);

    http.end();
  } else {
    Serial.println("❌ Wi-Fi desconectado!");
  }

  Serial.println("Aguardando 10 segundos...\n");
  delay(10000);
}