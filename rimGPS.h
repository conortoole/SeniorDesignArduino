#ifndef rimGPS_H
#define rimGPS_H
#include <TinyGPSPlus.h>
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <SoftwareSerial.h>


class rimGPS: public TinyGPSPlus{
     public:
          rimGPS();
          void initialize(SoftwareSerial serial, int baudrate);
};

#endif
