
#include <rimLED.h>
#include <rimBattery.h>
#include <Adafruit_TinyUSB.h>

rimLED led(10);
rimBattery bat;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  led.On();
}

void loop() {
  // put your main code here, to run repeatedly:
  bat.readBattery();
  Serial.print("Battery Voltage: ");
  Serial.println(bat.vBat);
  
  Serial.println(led.status);
  led.status = bat.updateLED(led);
  // Serial.println(bat.count);
  delay(1000);
}
