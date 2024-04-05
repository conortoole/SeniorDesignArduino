#include <Arduino.h> 

class rimLED {
    public:
        int pin;
        // int rPin;
        // int gPin;
        // int bPin;
        // enum Color {red, green, yellow, blue};
        
        rimLED(int pin);
        void Off();
        void On();
        //void setColor(Color color);
};