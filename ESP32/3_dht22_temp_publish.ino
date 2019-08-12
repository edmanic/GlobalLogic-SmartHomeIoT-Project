/* MQTT publishing data from temperature sensor DHT22 to MQTT broker (Raspberry Pi)
 Ensure that you have all the libraries downloaded and working WiFi network
 Code publishes only temperature data from DHT22 sensor to the MQTT broker through Wifi network

 The circuit:
 - connect VCC sensor pin to 3.3V on ESP32
 - connect GND sensor pin to ESP32 GND pin
 - connect data pin od sensor to D5 pin on microcontroller

 created by: Anita Garic, Edin Pjevic
 GlobalLogic student project
 
*/
#include <WiFi.h>          // Enables the ESP32 to connect to the local network (via WiFi)
#include <PubSubClient.h>  // Allows us to connect to, and publish to the MQTT broker
#include <DHTesp.h>
#define DHTpin 5

const char* ssid = "SmartHuawei";              //WiFi variables
const char* password =  "smartstudent";

const char* mqttServer = "192.168.43.181";    //MQTT variables
const int mqttPort = 1883;

int raspReady = 1;                            //Sending control variables
int switcher = 5;
 
WiFiClient espClient;              //Initialise the WiFi and MQTT Client objects
PubSubClient client(espClient);
DHTesp dht;

char temperatureChar[10];

void callback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic,"rasp/feedback")==0){           //checking if Raspberry is ready to receive next data
    payload[length]= '\0';
    raspReady = atoi((char *)payload);
  }
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
 
}
 
void setup() {
  Serial.begin(9600);
 
  WiFi.begin(ssid, password);                     //Connecting to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");      //Connecting to MQTT
 
    if (client.connect("ESP32Client")) {
      Serial.println("connected");  
 
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(1000);
 
    }
  }
  dht.setup(DHTpin, DHTesp::DHT22);               //creating an object for temperature and humidity sensor

  client.subscribe("home/room1/temp");
  client.subscribe("rasp/feedback");
 
}
 
void loop() {

  while (WiFi.status() != WL_CONNECTED) {       //WiFi connection interrupted
    Serial.println("Disconnected from WiFi!");
    WiFi.begin(ssid, password);                 //trying to reconnect to WiFi
    Serial.println("Reconnecting to WiFi..");
  }

  while (!client.connected()) {                 //MQTT connection interrupted
    Serial.println("Reconnecting to MQTT...");  //trying to reconnect to MQTT
 
    if (client.connect("ESP32Client")) {
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(1000);
    }
  }
  client.loop();

 if(raspReady==1){
  
   switch(switcher){
     case 5:{
          TempAndHumidity measurement = dht.getTempAndHumidity();
          dtostrf(measurement.temperature,5,2,temperatureChar); 
          switcher++;  
     }
     case 6: {
             char charBuffTemp[50];          
             String message1 =  String("{temp: ") + temperatureChar + String(", topic: home/room1/temp}");
             message1.toCharArray(charBuffTemp, 50);
             client.publish("home/room1/temp", charBuffTemp);
             raspReady=0;
             switcher=5;
             break;
     }
    }

 }
 
}
