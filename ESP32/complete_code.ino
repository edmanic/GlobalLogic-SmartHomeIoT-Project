/* Complete code for GlobalLogic student project
 Ensure that you have all the libraries downloaded and working WiFi network
 Code publishes temperature  and humidity data from DHT22 sensor to the MQTT broker through Wifi network
 Code reads messages from MQTT topic "emergency/fan" and turns the fan on if needed
 Code publishes temperature from another temperature sensor DS180B20 to MQTT broker
 Code checks if correct 4-digit password has been enetered through keypad, if it has, published "login" to MQTT broker, else if there were 4 wrong inputs, 
 lockes the system and doesn't let you any more tries

 The circuit:
 - connect VCC sensor pin to 3.3V on ESP32
 - connect GND sensor pin to ESP32 GND pin
 - connect data pin od sensor to D5 pin on microcontroller
 - connect GND and VCC of sensor DS18B20 to 3.3V and GND of microcontroller
 - connect data pin of DS18B20 to 4,7 kohm resistor and then to D15
 - connect the motor and LED diode according to sketch from the pdf document (circuit with diode and npn transistor)
 - connect the keyboard as it is shown in picture in pdf file

 created by: Anita Garic, Edin Pjevic
 GlobalLogic student project
 
*/

#include <WiFi.h>          // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h>  // Allows us to connect to, and publish to the MQTT broker
#include <DHTesp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Keypad.h>
#define DHTpin 5
#define ONE_WIRE_BUS 15
#define motorPin 23
#define ledPin 22

const char* ssid = "SmartHuawei";              //WiFi variables
const char* password =  "smartstudent";

const char* mqttServer = "192.168.43.181";    //MQTT variables
const int mqttPort = 1883;

int raspReady = 1;                            //Sending control variables
int switcher = 5;
 
WiFiClient espClient;       //Initialise the WiFi and MQTT Client objects
PubSubClient client(espClient);
DHTesp dht;
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

char temperatureChar[10];
char humidityChar[10];
char temperatureCharDS[10]; 

int fanWork = 0;

//Keyboard variables
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {13, 12, 14, 27}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 25, 33, 32}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
char correctKeyboard[4] = {'6', '9', '6', '9'};
char passKeyboard[4] = {0,0,0,0};
int i = 0;
int count = 0;
int flag = 1;
int tryCount = 0;
char resetField[] = "reset";

void callback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic,"rasp/feedback")==0){
    payload[length]= '\0';
    raspReady = atoi((char *)payload);
  }
  if(strcmp(topic,"home/room1/login")==0){        //reseting login sensor
     for (int i = 0; i < length; i++) {
           if( (char)payload[i] != resetField[i] ){
              break;
           }
           flag = 1;
           tryCount = 0;
           client.publish("home/room1/login", "{status: reseted, topic: home/room1/login}");
     }
  }
  
  if(strcmp(topic,"emergency/fan")==0){    
    payload[length]= '\0';
    fanWork = atoi((char *)payload);
    if (fanWork == 1){
              digitalWrite(motorPin, HIGH);
              digitalWrite(ledPin, HIGH);
    } else {
              digitalWrite(motorPin, LOW);
              digitalWrite(ledPin, LOW);
    }
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
 
  WiFi.begin(ssid, password);                   //Connecting to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");      //Connecting to MQTT
 
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
 
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(1000);
 
    }
  }
  dht.setup(DHTpin, DHTesp::DHT22);               //creating an object for temperature and humidity sensor
  sensors.begin(); 
  pinMode(motorPin, OUTPUT);
  pinMode (ledPin, OUTPUT);

  client.subscribe("home/room1/temp");
  client.subscribe("home/room1/hum");
  client.subscribe("rasp/feedback");
  client.subscribe("home/room1/temp_ntc");
  client.subscribe("emergency/fan");
  client.subscribe("home/room1/login"); 
}
 
void loop() {

  while (WiFi.status() != WL_CONNECTED) {       //WiFi connection interrupted
    Serial.println("Disconnected from WiFi!");
    WiFi.begin(ssid, password);                 //trying to reconnect to WiFi
    Serial.println("Reconnecting to WiFi..");
  }

  while (!client.connected()) {                 //MQTT connection interrupted
    Serial.println("Reconnecting to MQTT...");  //trying to reconnect to MQTT
 
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(1000);
    }
  }
  client.loop();

  char key = keypad.getKey();
  if(key){
     if(count<4 && flag != 2){
       
        passKeyboard[count]=key;
   
        for (i=0; i<4; i++){
          Serial.print(passKeyboard[i]);
          Serial.print("  ");
        }
        Serial.println("");
        count++; 
     }

     if (key == 'C' && flag != 2){
        for (i=0; i<4; i++){
            if(passKeyboard[i] != correctKeyboard[i]){
              flag = 0;
              break;
            } else {
              flag = 1;
            }
        }
        tryCount++;
        if(tryCount > 3){                   //after 4 tries keypad is locked, waiting for confirmation from raspberry
          flag = 2;
        }
        
        if(flag == 1){
          Serial.println("Authorized access!");
          tryCount = 0;
          client.publish("home/room1/login", "{status: login, topic: home/room1/login}");
        }else if(flag == 2) {
          Serial.println("Too many tries! You have to wait for confirmation!");
          client.publish("home/room1/login", "{status: lockedout, topic: home/room1/login}");
        } else {
          Serial.println("Unauthorized access!");
        }
        count = 0;
        for (i=0; i<4; i++){
          passKeyboard[i]= 0;
        }
     }
  }

 if(raspReady==1){
  
   switch(switcher){
     case 5:{
          TempAndHumidity measurement = dht.getTempAndHumidity();
          dtostrf(measurement.temperature,5,2,temperatureChar);
          dtostrf(measurement.humidity,5,2,humidityChar); 
          
          sensors.requestTemperatures(); 
          dtostrf(sensors.getTempCByIndex(0),5,2,temperatureCharDS);
          
          switcher++;  
     }
     case 6: {
             char charBuffTemp[50];          
             String message1 =  String("{temp: ") + temperatureChar + String(", topic: home/room1/temp}");
             message1.toCharArray(charBuffTemp, 50);
             client.publish("home/room1/temp", charBuffTemp);
             raspReady=0;
             switcher++;
             break;
     }
     case 7: {
             char charBuffHum[50];
             String message2 =  String("{hum: ") + humidityChar + String(", topic: home/room1/hum}"); 
             message2.toCharArray(charBuffHum, 50);
             client.publish("home/room1/hum", charBuffHum);
             raspReady=0;
             switcher++;
             break;
     }
     case 8: {
             char charBuffTemp[50];
             String message3 =  String("{temp: ") + temperatureCharDS + String(", topic: home/room1/temp_ntc}"); 
             message3.toCharArray(charBuffTemp, 50);
             client.publish("home/room1/temp_ntc", charBuffTemp);
             raspReady=0;
             switcher=5;
             break;
     }
    }

 }
 
}
