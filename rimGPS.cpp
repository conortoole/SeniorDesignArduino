#include "rimGPS.h"
#include <SoftwareSerial.h>
#include <Arduino.h>

rimGPS::rimGPS(){}

void rimGPS::initialize(SoftwareSerial serial, int baudrate){
     serial.begin(baudrate);
     while (serial.available() > 0) {
          this->encode(serial.read());
          Serial.println("Encoding...");
     }

     if (millis() > 5000 && this->charsProcessed() < 10) {
         Serial.println(("No GPS detected: check wiring."));
         while(true);
     }
}
