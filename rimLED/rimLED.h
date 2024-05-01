#ifndef RIMLED_H
#define RIMLED_H

class rimLED {
    public:
        rimLED(int pin);
        int pin;
        bool status;
        void Off();
        void On();
};

#endif
