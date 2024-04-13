#include <rimLED.h>
#include <rimBattery.h>
#include <bluefruit.h>
#include <nrf_soc.h> 

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

#define MANUFACTURER_ID   0x0059 //Nordic Manufacturer ID
#define BATTPIN A6

uint8_t beaconUuid[16] = {
  0x21, 0x21, 0x21, 0x21, 
  0x21, 0x21, 0x21, 0x21,
  0x21, 0x21, 0x21, 0x21, 
  0x21, 0x21, 0x21, 0x21
};

// Set Beacon
BLEBeacon beacon(beaconUuid, 1, 2, -54);
rimBattery bat;
TinyGPSPlus gps;
rimLED batLED(10);
rimLED gpsLED(11);
rimLED bleLED(12);
bool check = false;


SoftwareSerial serial(A0, A1); //   A0 is RX    A1 is TX

char deviceID[23];

char* getID() {
 static char combinedID[23]; 

 char idA[11];
 char idB[11];

 uint32_to_base20(NRF_FICR->DEVICEID[0], idA);
 uint32_to_base20(NRF_FICR->DEVICEID[1], idB);

 sprintf(combinedID, "%s%s", idA, idB);

 return combinedID;
}

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

    // Restart advertising to broadcast the updated beaconUuid
    Bluefruit.Advertising.stop(); // Stop current advertising
    startAdv(); // Restart advertising with the updated beaconUuid
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
  serial.begin(9600);
  // while ( !Serial ) delay(10);
  
  strcpy(deviceID, getID());

  Serial.println("Bluefruit52 Beacon Example");
  Serial.println(deviceID);
  Serial.println("--------------------------\n");

  Bluefruit.begin();

  Bluefruit.setTxPower(8);

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

  Bluefruit.Advertising.addTxPower();

  Bluefruit.ScanResponse.addName();
  
  Bluefruit.Advertising.setType(BLE_GAP_PHY_CODED);
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(160, 160);    
  Bluefruit.Advertising.setFastTimeout(30);      
  check = Bluefruit.Advertising.start(0);    
  if (check == true){
    bleLED.On();
  }  
  else{
    bleLED.Off();
  }          // 0 = Don't stop advertising after n seconds  
}

void loop() {
  float battMeasure;
  Serial.println("Begin!");

  ///////////// Enter GPS Sequence ////////////////////////
  while (serial.available() > 0) {
      if (gps.encode(serial.read())) {

          if (gps.location.isUpdated()){
              gpsLED.On();
          }
          else{
              gpsLED.Off();
          }

          Serial.print("Device ID: ");
          Serial.println(deviceID);

          //////// Read Battery and update LED based off voltage ////////////
          bat.readBattery();
          battMeasure = bat.vBat;
          batLED.status = bat.updateLED(batLED);

          Serial.print("Battery Calculated: " ); 
          Serial.println(battMeasure);
      
          ///////// trigger bluetooth /////////////////////////////////////////
          updatePacket(gps.location.lat(), gps.location.lng(), gps.altitude.meters(), gps.speed.mph()); 
          /////////////////////////////////////////////////////////////////
    }
  }

  delay(500);
}

