/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define DHTPIN 5
#define DHTTYPE DHT11

const char* ssid       = "WERESO_5E";
const char* password   = "#welovereso";
const int dthPin       = 5;
const char* dthType    = "DHT11";
const float latitude  = 50.629874;
const float longitude   = 3.055467;

IPAddress server(51, 254, 117, 247);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
DHT dht(DHTPIN, DHTTYPE);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void publish_data() {
  DynamicJsonDocument doc(1024);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  doc["humidity"] = h;
  doc["temperature"] = t;
  doc["heat_index"] = dht.computeHeatIndex(t, h, false);

  JsonObject position = doc.createNestedObject("pos");
  position["lon"] = longitude;
  position["lat"] = latitude;
  
  char JSONmessageBuffer[100];
  serializeJson(doc, JSONmessageBuffer);
  mqttClient.publish("sensors/device01/from_device", JSONmessageBuffer);
}

void reconnectMqtt () {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (mqttClient.connect("", "device01", "device01")) {
      Serial.println("connected");
      mqttClient.subscribe("sensors/device01/from_clients");
    }
    else {
      Serial.print("failed, rc=");
      Serial.println(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mqttClient.setServer(server, 1883);
  mqttClient.setCallback(callback);

  dht.begin();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMqtt();
  }

  mqttClient.loop();
  publish_data();
  delay(2000);
}
