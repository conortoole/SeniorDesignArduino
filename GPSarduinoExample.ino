#include <bluefruit.h>
#include <nrf_soc.h> 

#define MANUFACTURER_ID   0x0059 //Nordic Manufacturer ID

uint8_t ad[16] = {
  0x21, 0x21, 0x21, 0x21, 
  0x21, 0x21, 0x21, 0x21,
  0x21, 0x21, 0x21, 0x21, 
  0x21, 0x21, 0x21, 0x21
};

void uint32_to_base20(uint32_t value, char* buffer) { 
    char* ptr = buffer;
    do {
        uint32_t digit = value % 20;
        *ptr++ = digit < 10 ? '0' + digit : 'A' + (digit - 10);
        value /= 20;
    } while (value != 0);
    *ptr = '\0';

    char* start = buffer;
    char* end = ptr - 1;
    while (start < end) {
        char temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

// Set Beacon
BLEBeacon beacon(ad, 1, 2, -54);

void setup() {

 Serial.begin(115200);
 while (!Serial) delay(20000);

 Serial.println("Remote ID Module");
 Serial.println("-----------------\n");

 char bufferA[11];
 uint32_to_base20(NRF_FICR->DEVICEID[0], bufferA);
 char bufferB[11];
 uint32_to_base20(NRF_FICR->DEVICEID[1], bufferB);

}

void startAdv(void)
{  
  Bluefruit.Advertising.setBeacon(beacon);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.setType(BLE_GAP_PHY_CODED);
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(160, 160);    
  Bluefruit.Advertising.setFastTimeout(30);      
  Bluefruit.Advertising.start(0);               
}

void updateAdvertisement() {

 //GPS gathering 

 //Set new advertising data with GPS information

 uint8_t newAd[16] = {
    0x21, 0x21, 0x21, 0x21, 
    0x21, 0x21, 0x21, 0x21,
    0x21, 0x21, 0x23, 0x23, 
    0x23, 0x23, 0x21, 0x21
 };

 memcpy(ad, newAd, sizeof(newAd));
 beacon.setUuid(ad);
 Bluefruit.Advertising.setBeacon(beacon);
 Bluefruit.Advertising.stop();
 Bluefruit.Advertising.start(0); 
}

void loop() {

  delay(5);
 
  updateAdvertisement();

  delay(5);
}