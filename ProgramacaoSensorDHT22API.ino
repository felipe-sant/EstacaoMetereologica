#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <DHT.h>

const char* ssid = "Familia Machado";
const char* password = "963214785";

#define DHTPIN D3    // Mudado para D3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600); // Mudado para 9600
  WiFi.begin(ssid, password);

  Serial.println("\nConectando ao Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  dht.begin();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    client->setInsecure(); // Aceita qualquer certificado SSL

    HTTPClient https;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      String serverPath = "https://coletor-de-tempo.vercel.app/api/";
      https.begin(*client, serverPath);
      https.addHeader("Content-Type", "application/json");

      String postData = "{\"temperatura\":" + String(t) + ",\"umidade\":" + String(h) + "}";

      int httpResponseCode = https.POST(postData);

      Serial.print("Código de resposta: ");
      Serial.println(httpResponseCode);

      String response = https.getString();
      Serial.println("Resposta do servidor: ");
      Serial.println(response);

      https.end();
    } else {
      Serial.println("Falha ao ler o sensor DHT22!");
    }
  } else {
    Serial.println("Wi-Fi desconectado");
  }

  delay(10000); // Mudado para 10 segundos
}
