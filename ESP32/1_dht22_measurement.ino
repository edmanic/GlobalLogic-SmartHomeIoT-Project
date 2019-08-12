/* DHT22 temperature and humidity measurement using ESP32 microcontroller

 The circuit:
 - connect VCC sensor pin to 3.3V on ESP32
 - connect GND sensor pin to ESP32 GND pin
 - connect data pin od sensor to D5 pin on microcontroller

 created by: Anita Garic, Edin Pjevic
 GlobalLogic student project
 
*/

#include <DHTesp.h>
#define DHTpin 5

DHTesp dht;
void setup() {
  Serial.begin(115200);
  dht.setup(DHTpin, DHTesp :: DHT22);    //sensor is connected to pin 5 of ESP32
}
void loop () {
  TempAndHumidity measurement = dht.getTempAndHumidity();
  Serial.print("Temperature: ");
  Serial.println(measurement.temperature);
  Serial.print("Humidity: ");
  Serial.println(measurement.humidity);
  delay(2000);
}
