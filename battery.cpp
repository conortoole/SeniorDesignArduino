#include "battery.h"
#include <SoftwareSerial.h>
#include <Arduino.h>

#define VBATPIN A6

void readBattery {
  Serial.begin(9600);
  
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2 with the voltage divider
  measuredvbat *= 3.7;  // Multiply by 3.7V, our reference voltage
  measuredvbat /= 1024; // convert to actual voltage
  Serial.print("VBat: " ); 
  Serial.println(measuredvbat);
  
}
