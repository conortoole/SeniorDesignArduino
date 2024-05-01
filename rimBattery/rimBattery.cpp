#include <Arduino.h>
#include "rimBattery.h"
#include <rimLED.h>

#define VBATPIN A6

void rimBattery::readBattery() {
    float measuredvbat = analogRead(VBATPIN);
    measuredvbat *= 2;    // we divided by 2 with the voltage divider
    measuredvbat *= 3.7;  // Multiply by 3.7V, our reference voltage
    measuredvbat /= 1024; // convert to actual voltage
    this->vBat = measuredvbat;
}

bool rimBattery::updateLED(rimLED led) {
    if (this->vBat >= 3.40){
        led.On();
        led.status = true;
        return led.status;
    }
    else{
    	if (led.status == true){
    	    led.On();
    	    led.status = false;
    	    return led.status;
    	}
    	else{
    	    led.Off();
    	    led.status = true;
    	    return led.status; 
    	}
    }
}
