#ifndef battery_H
#define battery_H
#include <Arduino.h>
#include <RIMLED.h>

class Battery {
    public:
        float vBat = 0.0;
        rimLED batLed(1, 2, 3); //(r, g, b)
        void readBattery();
        void updateLED();
}
