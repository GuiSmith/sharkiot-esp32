#include <WiFi.h>
#include <HTTPClient.h>
#include "sMQTTBroker.h"

#define IR_SENSOR_PIN 13
#define TRIGGER_PIN 7
#define ECHO_PIN 10

const char* ssid = "motog14";
const char* password = "fernando";

// HTTP
const char* serverName = "http://192.168.19.50:5000/log_sensor";

// MQTT
sMQTTBroker broker;
const unsigned short mqttPort = 1883;
String lastProtocolUsed = "";

// Alternância
bool useHttp = true;

IPAddress local_IP(192, 168, 19, 100); // IP fixo
IPAddress gateway(192, 168, 19, 83); 
IPAddress subnet(255, 255, 255, 0); 

void setup() {
  Serial.begin(115200);

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Falha ao configurar IP estático");
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao Wi-Fi...");
  }

  Serial.println("Conectado ao Wi-Fi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Inicia o broker MQTT
  broker.init(mqttPort);
}

void loop() {
  // Leitura do sensor IR
  int irValue = digitalRead(IR_SENSOR_PIN);
  String irStatus = (irValue == LOW) ? "Objeto detectado" : "Nenhum objeto";

  // Medição de distância
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.0344 / 2;

  Serial.print("IR Status: ");
  Serial.println(irStatus);
  Serial.print("Distância: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (WiFi.status() == WL_CONNECTED) {
    if (useHttp) {
      // Enviar via HTTP
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String postData = "irStatus=" + irStatus + "&distance=" + String(distance) + "&protocol=HTTP";
      int response = http.POST(postData);

      if (response > 0) {
        Serial.println("Dados enviados via HTTP");
        lastProtocolUsed = "HTTP";
      } else {
        Serial.println("Erro ao enviar via HTTP");
      }

      http.end();
    } else {
      // Enviar via MQTT (publicar local no broker)
      String mqttMessage = "IR: " + irStatus + ", Distância: " + String(distance) + " cm";
      broker.publish("sensor/ir", mqttMessage.c_str());
      Serial.println("Dados enviados via MQTT");
      lastProtocolUsed = "MQTT";
    }
  } else {
    Serial.println("WiFi não conectado");
  }

  Serial.println("Último protocolo usado: " + lastProtocolUsed);
  Serial.println("----------------------------------");

  // Alterna protocolo
  useHttp = !useHttp;

  // Atualiza broker MQTT
  broker.update();

  delay(5000);
}
