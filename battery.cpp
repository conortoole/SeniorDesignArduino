#include "battery.h"
#include <Arduino.h>

#define VBATPIN A6

void battery::readBattery() {
    Serial.begin(9600);
  
    float measuredvbat = analogRead(VBATPIN);
    measuredvbat *= 2;    // we divided by 2 with the voltage divider
    measuredvbat *= 3.7;  // Multiply by 3.7V, our reference voltage
    measuredvbat /= 1024; // convert to actual voltage
    this->vBat = measuredvbat
}

void updateLED() {
    if (this->vBat > 3.7){
        this->batLed.setColor(rimLED::green);
    }
    else if (this->vBat > 3.4){
        this->batLed.setColor(rimLED::yellow);
    }
    else{
        this->batLed.setColor(rimLED::red);
    }
}
