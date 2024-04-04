#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <SoftwareSerial.h>
#include <rimGPS.h>
#include <TinyGPSPlus.h>

 rimGPS gps; //remote ID module GPS 

void setup() {
  SoftwareSerial serial(A0, A1);
  
  gps.initialize(serial, 9600); //comunicate to GPS
  Serial.begin(115200); //comunicate to monitor
     
}

void loop() {
  delay(1000);
  Serial.println(gps.location.lat());
  // put your main code here, to run repeatedly:

}
