#include <Arduino.h> 
#include "RIMLED.h"

rimLED::rimLED(int pin){
    this->pin = pin;
    pinMode(this->pin, OUTPUT);
    // this->rPin = rPin;
    // this->gPin = gPin;
    // this->bPin = bPin;
    // pinMode(rPin, OUTPUT);
    // pinMode(gPin, OUTPUT);
    // pinMode(bPin, OUTPUT);
} 

void rimLED::Off(){
    digitalWrite(this->pin, LOW);
    // analogWrite(this->rPin, 0);
    // analogWrite(this->gPin, 0);
    // analogWrite(this->bPin, 0);
}

void rimLED::On(){
    digitalWrite(this->pin, HIGH);
}

// void rimLED::setColor(Color color){
//     switch(color){
//         case red:
//             analogWrite(this->rPin, 255);
//             analogWrite(this->gPin, 0);
//             analogWrite(this->bPin, 0);
//             break;
//         case green:
//             analogWrite(this->rPin, 0);
//             analogWrite(this->gPin, 255);
//             analogWrite(this->bPin, 0);
//             break;
//         case blue:
//             analogWrite(this->rPin, 0);
//             analogWrite(this->gPin, 0);
//             analogWrite(this->bPin, 255);
//             break;
//         case yellow:
//             analogWrite(this->rPin, 255);
//             analogWrite(this->gPin, 195);
//             analogWrite(this->bPin, 0);
//             break;
//     }
// }
