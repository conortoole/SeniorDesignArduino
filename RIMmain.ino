#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <SoftwareSerial.h>
//#include <rimGPS.h>
#include <TinyGPSPlus.h>

 //rimGPS gps; //remote ID module GPS 

//void setup() {
//  SoftwareSerial serial(A0, A1);
  
//  gps.initialize(serial, 9600); //comunicate to GPS
//  Serial.begin(115200); //comunicate to monitor
     
//}

//void loop() {
 // delay(1000);
  //Serial.println(gps.location.lat());
  // put your main code here, to run repeatedly:

//}


TinyGPSPlus gps;
SoftwareSerial serial(A0, A1);

void setup() {
  serial.begin(9600);
  Serial.begin(115200); //comunicate to monitor
}

void loop() {
     while (serial.available() > 0) {
          if(gps.encode(serial.read())){
               Serial.println(gps.location.lat());
               Serial.println(gps.location.lng());
               Serial.print(gps.time.hour());
               Serial.print(":");
               Serial.println(gps.time.minute());
          }
     }

     if (millis() > 5000 && gps.charsProcessed() < 10) {
          Serial.println(("No GPS detected: check wiring."));
          while(true);
     }

}
