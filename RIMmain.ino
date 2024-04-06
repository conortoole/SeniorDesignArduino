#include <RIMLED.h>
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <SoftwareSerial.h>
#include <rimGPS.h>
#include <TinyGPSPlus.h>
//
//rimGPS gps; // remote ID module GPS 
//SoftwareSerial serial(A0, A1);
//rimLED gpsLED(10);
//
//void setup() {
//  // comunicate to GPSrimLED::
//  serial.begin(gps.baudrate);
//  Serial.begin(115200); // comunicate to monitor;
//  Serial.println("Hello World!");    
//  gpsLED.Off();
//}
//
//void loop() {
//  delay(1000);
//  gps.initialize(serial, gps.baudrate); // comunicate to GPS
//  Serial.println(gps.location.lat());
//  if (gps.location.lat() > 0){ // this should probably be updated to something different
//      Serial.println(gps.location.lat());
//      gpsLED.On();
//      // evan put your bluetooth code in here 
//  }
//  else{
//      Serial.println("waiting on GPS signal...");
//  }
//}

TinyGPSPlus gps;
SoftwareSerial serial(A0, A1);
rimLED gpsLED(10);

void setup() {
  serial.begin(9600);
  Serial.begin(115200); //comunicate to monitor
  gpsLED.Off();
}

void loop() {
     while (serial.available() > 0) {
          if(gps.encode(serial.read())){
               if (gps.location.isValid()){
                   gpsLED.On();
               }
               else{
                   gpsLED.Off();
               }
               Serial.println(gps.location.isValid());
               Serial.println(gps.location.lat());
               Serial.println(gps.location.lng());
               Serial.println(gps.altitude.meters());
               Serial.println(gps.speed.mps());
               Serial.print(gps.time.hour());
               Serial.print(":");
               Serial.println(gps.time.minute());
               
          }
     }

     if (millis() > 5000 && gps.charsProcessed() < 10) {
          Serial.println(("Waiting for connection to satellite..."));
          while(true);
     }

}
