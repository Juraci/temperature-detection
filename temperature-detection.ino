#include <Arduino.h>
#include <string>
#include <WifiWrapper.h>
#include <MqttWrapper.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>

#define USE_SERIAL Serial

WiFiClient espClient;
PubSubClient client(espClient);

char* ssid = "____";
char* password = "____";

char* mqttServer = "____";
int mqttPort = 17152;
char* clientId = "____";
char* tempTopic = "____";
char* humidityTopic = "____";
char* inTopic = "____";
char* brokerUser = "____";
char* brokerPassword = "____";
long now = 0;
long lastMsg = 0;

const int mainLoopDelay = 1000;
const int reportInterval = 30000;

int dhtPin = 4;

void callback(char* topic, byte* payload, unsigned int length);
void reportReadings();

WifiWrapper wifi(ssid, password);
MqttWrapper mqtt(mqttServer, clientId, brokerUser, brokerPassword, mqttPort, client);
DHTesp dht;

void setup() {
  USE_SERIAL.begin(115200);

  dht.setup(dhtPin, DHTesp::DHT22);
  wifi.waitForConnection();
  mqtt.setup();
  mqtt.setTopics(inTopic, tempTopic);
  mqtt.setCallback(callback);
  mqtt.connect();
}

void loop() {
  wifi.waitForConnection();

  mqtt.loop();

  now = millis();
  if (now - lastMsg >= reportInterval) {
    lastMsg = now;
    reportReadings();
  }

  delay(mainLoopDelay);
}

void reportReadings() {
  TempAndHumidity lastValues = dht.getTempAndHumidity();
  Serial.println("Temperature: " + String(lastValues.temperature,0));
  Serial.println("Humidity: " + String(lastValues.humidity,0));

  char temp[8];
  char humidity[8];

  dtostrf(lastValues.temperature, 6, 1, temp);
  dtostrf(lastValues.humidity, 6, 2, humidity);

  mqtt.publish(temp, tempTopic);
  mqtt.publish(humidity, humidityTopic);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
