#ifndef battery_H
#define battery_H
#include <rimLED.h>

class rimBattery {
    public:
        float vBat = 0.0;
        void readBattery();
        bool updateLED(rimLED led);
       
};

#endif
