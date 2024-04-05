#include "rimGPS.h"
#include <SoftwareSerial.h>
#include <Arduino.h>

rimGPS::rimGPS(){}

void rimGPS::initialize(SoftwareSerial serial, int baudrate){
     //serial.begin(baudrate);
     if (serial.available() > 0) {
          this->encode(serial.read());
          return;
     }
     else{
     	Serial.println(("No GPS detected: check wiring."));
     	return;
     }
}
