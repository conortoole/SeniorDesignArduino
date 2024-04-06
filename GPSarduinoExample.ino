#include <bluefruit.h>
#include <nrf_soc.h> 

#define MANUFACTURER_ID   0x0059 //Nordic Manufacturer ID

uint8_t beaconUuid[16] = {
  0x21, 0x21, 0x21, 0x21, 
  0x21, 0x21, 0x21, 0x21,
  0x21, 0x21, 0x21, 0x21, 
  0x21, 0x21, 0x21, 0x21
};

// Set Beacon
BLEBeacon beacon(beaconUuid, 1, 2, -54);

//TODO: getID to return combined buffers

void updatePacket(float A, float B, float C, float D) {
    // Union to reinterpret float as bytes
    union {
        float f;
        uint8_t bytes[4];
    } floatToBytes;

    // Convert each float to bytes and insert into beaconUuid
    floatToBytes.f = A;
    memcpy(beaconUuid, floatToBytes.bytes, 4);

    floatToBytes.f = B;
    memcpy(beaconUuid + 4, floatToBytes.bytes, 4);

    floatToBytes.f = C;
    memcpy(beaconUuid + 8, floatToBytes.bytes, 4);

    floatToBytes.f = D;
    memcpy(beaconUuid + 12, floatToBytes.bytes, 4);

    // Optionally, print the updated beaconUuid for debugging
    Serial.print("Updated beaconUuid: ");
    for (int i = 0; i < 16; i++) {
        Serial.print(beaconUuid[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

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

void setup() {
  Serial.begin(115200);

  // while ( !Serial ) delay(10);

  char idA[11];
  char idB[11];

  uint32_to_base20(NRF_FICR->DEVICEID[0], idA);
  uint32_to_base20(NRF_FICR->DEVICEID[1], idB);

  Serial.println("Bluefruit52 Beacon Example");
  Serial.println("--------------------------\n");

  Bluefruit.begin();

  beacon.setManufacturer(MANUFACTURER_ID);

  startAdv();

  Serial.printf("Broadcasting beacon with MANUFACTURER_ID = 0x%04X\n", MANUFACTURER_ID);
  Serial.println("open your beacon app to test such as: nRF Beacon");
  Serial.println("- on Android you may need to change the MANUFACTURER_ID to 0x0059");
  Serial.println("- on iOS you may need to change the MANUFACTURER_ID to 0x004C");

}

void startAdv(void)
{  
  Bluefruit.Advertising.setBeacon(beacon);

  Bluefruit.ScanResponse.addName();
  
  Bluefruit.Advertising.setType(BLE_GAP_PHY_CODED);
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(160, 160);    
  Bluefruit.Advertising.setFastTimeout(30);      
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void loop() {

  delay(1000);

  updatePacket(1.1, 2.2, 3.3, 4.4);

  Serial.println("End of loop...");

}