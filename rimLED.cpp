#include <Arduino.h> 
#include "rimLED.h"

rimLED::rimLED(int pin){
    this->pin = pin;
    pinMode(this->pin, OUTPUT);
}

void rimLED::Off(){
    digitalWrite(this->pin, LOW);
}

void rimLED::On(){
    digitalWrite(this->pin, HIGH);
}

