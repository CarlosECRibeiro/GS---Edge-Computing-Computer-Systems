#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>


// Configurações de WiFi
const char *SSID = "Wokwi-GUEST";
const char *PASSWORD = ""; 

// Configurações de MQTT
const char *BROKER_MQTT = "broker.hivemq.com";
const int BROKER_PORT = 1883;
const char *ID_MQTT = "FHM_mqtt";
const char *TOPIC_PUBLISH = "GS/FILA";
const int trigPin = 5;
const int echoPin = 18;

// Configurações de Hardware
#define RED_LED 21
#define GREEN_LED 2
#define PUBLISH_DELAY 2000
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
#define SERVO_PIN 27

// Variáveis globais
WiFiClient espClient;
PubSubClient MQTT(espClient);
unsigned long publishUpdate = 0;
const int TAMANHO = 200;
long duration;
float distanceCm;
float distanceInch;

// Protótipos de funções
void updateSensorValues();
void initWiFi();
void initMQTT();
void reconnectMQTT();
void reconnectWiFi();
void checkWiFIAndMQTT();


void initWiFi() {
  Serial.print("Conectando com a rede: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso: ");
  Serial.println(SSID);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Tentando conectar com o Broker MQTT: ");
    Serial.println(BROKER_MQTT);

    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao broker MQTT!");
      //entrar no mqtt
      
    } else {
      Serial.println("Falha na conexão com MQTT. Tentando novamente em 2 segundos.");
      delay(2000);
    }
  }
}

void checkWiFIAndMQTT() {
  if (WiFi.status() != WL_CONNECTED) reconnectWiFi();
  if (!MQTT.connected()) reconnectMQTT();
}

void reconnectWiFi(void)
{
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Wifi conectado com sucesso");
  Serial.print(SSID);
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  initWiFi();
  initMQTT();
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED/2;

  if(distanceCm < 50){
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
  }
  else{
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  }

  checkWiFIAndMQTT();
  MQTT.loop();

  if ((millis() - publishUpdate) >= PUBLISH_DELAY) {
    publishUpdate = millis();

      if(distanceCm < 50){
        StaticJsonDocument<TAMANHO> StatusFila;
        StatusFila["Status"] = "Ocupada";

        char BufferStatusFila[TAMANHO];
        serializeJson(StatusFila, BufferStatusFila);
        MQTT.publish(TOPIC_PUBLISH, BufferStatusFila);
        Serial.println(BufferStatusFila);

      }
  }

}

