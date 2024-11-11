#include <Arduino.h>
#include <WiFiS3.h>
#include "ArduinoJson.h"
#include "PubSubClient.h"
#include "Adafruit_Sensor.h"
#include "DHT.h"
#include "Adafruit_MLX90614.h"
#include <SPI.h>
#include "config.h"

void initialize_wifi();
void publish_data();
boolean reconnect();
void callback(char* topic, byte* payload, unsigned int length);

WiFiClient net;
PubSubClient mqttClient(net);

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
Adafruit_MLX90614 obj_temp = Adafruit_MLX90614(); 

static char data[256];
long lastReconnectAttempt = 0;
long lastData = 0;

void initialize_wifi(){
  delay(10);

  /*if (!WiFi.config(local_ip, home_router,subnet))
  {
    Serial.println("Failed to configure STA");
  }*/

  WiFi.disconnect();
  //WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting WiFi to: ");
  Serial.println(SSID_WIFI);

  while(WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(SSID_WIFI, PASSWORD_WIFI);
    Serial.print("Attempting WiFi connection .....");

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("\nWiFi RSSI: ");
      Serial.println(WiFi.RSSI());
      delay(100);
    }

    else
    {
      Serial.print("Failed to connect to WiFi");
      Serial.println(", Try again in 5 seconds");
      delay(5000);
    }

  }
}


void publish_data()
{
  JsonDocument doc;
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h))
  {
    Serial.println("Failed to read sensor");
    return;
  }

  float temperature_obj = obj_temp.readObjectTempC(); 

  doc["temperature"] = t;
  doc["humidity"] = h;
  doc["object_temperature"] = temperature_obj;
  serializeJsonPretty(doc, data);

  long now = millis();
  if (now - lastData > 15000) //updated sensor to the cloud every 15 seconds
  {
    lastData = now;
    Serial.println(data);
    mqttClient.publish(PUBLISH_TOPIC, data);
  }
}

boolean reconnect()
{
  Serial.println("Attempting to connect MQTT");

  //Try to connect mqtt broker
  if (mqttClient.connect(TOKEN, MQTT_USERNAME, MQTT_PASSWORD))
  {
    Serial.println("Connected to MQTT broker");
    digitalWrite(LED_BUILTIN, HIGH);
    mqttClient.subscribe(SUBSCRIBE_TOPIC, 1);
  }

  else{
    digitalWrite(LED_BUILTIN, LOW);
    delay(5000);
    Serial.println("Disconnected to MQTT broker");
  }

  //it returns mqtt connected
  return mqttClient.connected();
}

void callback(char* topic, byte* payload, unsigned int length)
{
    Serial.print("Message received on topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    String message;

    for (unsigned int i = 0; i < length; i++){
        //Convert *byte to string
        message += (char)payload[i];
    }
 
}

void setup() {

  Serial.begin(9600);
  dht.begin();
  Serial.println("Adafruit MLX90614 test");

  if (!obj_temp.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1);
  };

  Serial.print("Emissivity = "); Serial.println(obj_temp.readEmissivity());
  Serial.println("================================================");

  pinMode(LED_BUILTIN, OUTPUT);
  initialize_wifi();
  mqttClient.setServer(SECRET_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
  lastReconnectAttempt = 0;

}

void loop() {

  if (WiFi.status() != WL_CONNECTED)
  {
    initialize_wifi();
  }

  if (WiFi.status() == WL_CONNECTED && !mqttClient.connected())
  {
    long now = millis();

    if (now - lastReconnectAttempt > 2000)
    {
      lastReconnectAttempt = now;

      if (reconnect())
      {
        lastReconnectAttempt = 0;

      }

    }

  } else
  {
    mqttClient.loop();
  }

  publish_data();
 
}

