/* ESP32 Wifi connection
   Connecting the microcontroller to your WLAN 

   created by: Anita Garic, Edin Pjevic
   GlobalLogic student project
 
 */
 
#include <WiFi.h>
 
const char* ssid     = "SmartHuawei";       //Wifi credentials
const char* password = "smartstudent";
 
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.print("Connecting to ");
  Serial.print(ssid);
 
  WiFi.begin(ssid, password);     //connecting to WiFi
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void loop() {
  while (WiFi.status() == WL_CONNECTED) {     //checking if the connection is still active
    delay(5000);
    Serial.println("Still connected!");
  }
}
