#include <rimLED.h>
#include <rimBattery.h>
#include <bluefruit.h>
#include <nrf_soc.h> 
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

#define MANUFACTURER_ID   0x0059 //Nordic Manufacturer ID

uint8_t newMessageData[29] = {
        0x00,                   // 0    AD FLags
        0x0D,                   // 1    AD App
        0x00,                   // 2    AD Counter
        0x10,                   // 3    Message Type & Version
        //
        0x20,                   // 4    Status, Flags
        0x00,                   // 5    Speed North/South
        0x00,                   // 6    Speed East/West
        0x00,                   // 7    Vertical Speed
        0x00, 0x00, 0x00, 0x00, // 8    Latitude
        0x00, 0x00, 0x00, 0x00, // 12   Longitude
        0x00, 0x00,             // 16   Altitude
        0x00, 0x00,             // 18   Geodetic Altitude
        0x00, 0x00,             // 20   Height Above Takeoff
        0x39,                   // 22   Horizontal/Vertical Location Accuracy
        0x34,                   // 23   Timestamp/Speed Accuracy
        0x00, 0x00,             // 24   Timestamp
        0x00, 0x00,             // 26   Reserved
        //
        0x00                    // 28   CRC
    }; 

rimBattery bat;
TinyGPSPlus gps;

rimLED bleLED(12);

SoftwareSerial serial(A0, A1); //   A0 is RX    A1 is TX

char deviceID[23];

uint8_t stamp = 0;

uint8_t counter = 0;

float speed;
float course;
float latitude;
float longitude;
float altitude;

uint8_t hour;
uint8_t minute;
uint8_t second;
uint8_t centisecond;

char* getID() {
 static char combinedID[23]; 

 char idA[11];
 char idB[11];

 uint32_to_base20(NRF_FICR->DEVICEID[0], idA);
 uint32_to_base20(NRF_FICR->DEVICEID[1], idB);

 sprintf(combinedID, "%s%s", idA, idB);

 return combinedID;
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

void calculateSpeeds(float speed_mps, float course_degrees, float& speed_ns, float& speed_ew) {
    
    float course_radians = course_degrees * (PI / 180.0);
  
    speed_ns = speed_mps * sin(course_radians);
    speed_ew = speed_mps * cos(course_radians);
}

int8_t convertSpeed(float speed, bool& multiplier_flag) { 

    int8_t res;

    if ( (abs(speed)/0.25) <= 127 ) {
        res = speed / 0.25;
        multiplier_flag = 0;
    }
    if ( (abs(speed)/0.25) > 127 && abs(speed) < 127) {
        if (speed < 0) {
            res = (speed + (127*0.25)) / 0.75;
        }
        else {
            res = (speed - (127*0.25)) / 0.75;
        }
        multiplier_flag = 1;
    }
    if (speed >= 127) {
        res = 127;
    }

    return res;
}

int32_t convertLatLon(float input) {

    int32_t res = static_cast<int32_t>(round(input * pow(10, 7)));

    return res;
}

int8_t convertVerticalSpeed(float vertical_speed) {

    int8_t res = static_cast<int8_t>(round(vertical_speed / 0.5));

    return res;
}

uint16_t convertAltitude(float altitude) {

    uint16_t res = static_cast<int8_t>(round(altitude / 0.5) + 1000);

    return  res;
}

void updatePacketLocationMessage(float speed_mps, float course_degrees, float latitude, float longitude, float altitude, int hours, int minutes, int seconds, int centiseconds) {

    char buffer[5];

    // --------------------------------------------------------------------------
    // Speed North/South & Speed East/West

    bool speed_ns_multiplier = false;
    bool speed_ew_multiplier = false;

    float speed_ns;
    float speed_ew;

    int8_t speed_ns_out;
    int8_t speed_ew_out;

    calculateSpeeds(speed_mps, course_degrees, speed_ns, speed_ew);

    speed_ns_out = convertSpeed(speed_ns, speed_ns_multiplier);
    speed_ew_out = convertSpeed(speed_ew, speed_ew_multiplier);

    sprintf(buffer, "%08X", speed_ns_out); 
    newMessageData[5] = buffer[0];

    sprintf(buffer, "%08X", speed_ew_out); 
    newMessageData[6] = buffer[0];

    // --------------------------------------------------------------------------
    // Status & Flags

    if (speed_ns_multiplier) {
        buffer[0] = 0x02;           // 0000 0010
    }
    else {
        buffer[0] = 0x00;           // 0000 0000
    }

    if (speed_ew_multiplier) {
        buffer[0] += 1;
    }

    newMessageData[4] = buffer[0];

    // --------------------------------------------------------------------------
    // Vertical Speed

    // int8_t speed_vertical_out = convertVerticalSpeed(speed_vertical);
    // sprintf(buffer, "%08X", speed_vertical_out); 
    // newMessageData[7] = buffer[0];

    // --------------------------------------------------------------------------
    // Latitude                 Longitude

    int32_t latitude_out = convertLatLon(latitude);
    sprintf(buffer, "%08X", latitude_out); 
    for (int i = 0; i < 4; i++) {
        newMessageData[8+i] = buffer[i];
    }

    int32_t longitude_out = convertLatLon(longitude);
    sprintf(buffer, "%08X", longitude_out); 
    for (int i = 0; i < 4; i++) {
        newMessageData[12+i] = buffer[i];
    }

    // --------------------------------------------------------------------------
    // Altitude

    uint16_t altitude_out = convertAltitude(altitude);
    
    sprintf(buffer, "%08X", altitude_out); 
    for (int i = 0; i < 2; i++) {
        newMessageData[16+i] = buffer[i];
        newMessageData[18+i] = buffer[i];
    }

    // --------------------------------------------------------------------------

    //TODO Height above takeoff

    // --------------------------------------------------------------------------
    // Timestamp

    uint16_t time_out = (6000 * minutes) + (100 * seconds) + centiseconds;
    sprintf(buffer, "%08X", longitude_out); 
    for (int i = 0; i < 2; i++) {
        newMessageData[24+i] = buffer[i];
    }
 
    // --------------------------------------------------------------------------

    Serial.println("Updated Bluetooth Message: ");
    for (int i = 0; i < 29; i++) {
        Serial.print(newMessageData[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void setup() {
    serial.begin(9600);

    strcpy(deviceID, getID());

    Serial.println("Bluefruit52 Beacon Example");
    Serial.println(deviceID);
    Serial.println("--------------------------\n");

    int err = Bluefruit.begin();
    if (!err) {
        Serial.println("! Failed to initialize Bluetooth !");
    }

    Bluefruit.setTxPower(8);   
    Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED); 

    err = Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, newMessageData, sizeof(newMessageData));
    if (!err) {
        Serial.println("! Failed to add data in updatePacketLocationMessage() !");
    }

    err = Bluefruit.Advertising.start();
    if (!err) {
        Serial.println("! Failed to start Bluetooth Transmission !");
        bleLED.Off();
    }
    else {
        bleLED.On();
        Serial.println("BLE LED turned ON");
    }

    Serial.println("Advertising started");
    Serial.printf("Broadcasting beacon with MANUFACTURER_ID = 0x%04X\n", MANUFACTURER_ID);
}

void loop() {

    // if (serial.available() > 0) {

    //     Serial.print("!");

        while (serial.available() > 0) {

            Serial.print(".");
            
            if (gps.encode(serial.read())) {

                Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

                delay(500);

                speed = gps.speed.mps();  
                course = gps.course.deg();
                latitude = gps.location.lat();
                longitude = gps.location.lng();
                altitude = gps.altitude.meters();
                
                hour = gps.time.hour();
                minute = gps.time.minute();
                second = gps.time.second();
                centisecond = gps.time.centisecond();
                
            }
        }

        Serial.printf("\n%d:- %.3f, %.3f, %.3f, %.3f, %.3f, %d:%d:%d.%d\n", millis(), speed, course, latitude, longitude, altitude, hour, minute, second, centisecond);
        
        delay(1500);

        updatePacketLocationMessage(speed, course, latitude, longitude, altitude, hour, minute, second, centisecond);

        int err = Bluefruit.Advertising.stop();
        if (!err) {
            Serial.println("Failed stop()");
        }
        Serial.print("BLE stopped...");

        Bluefruit.Advertising.clearData();

        Serial.print("Packet cleared...");

        err = Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, newMessageData, sizeof(newMessageData));
        if (!err) {
            Serial.println("! Failed to add data in updatePacketLocationMessage() !");
        }

        Serial.print("Data added...");

        err = Bluefruit.Advertising.start();
        if (!err) {
            Serial.println("Failed start()");
            bleLED.Off();
        }
        else {
            bleLED.On();
            Serial.print("BLE LED ON");
        }

        Serial.println("..BLE Started.");
        delay(20);
    // }
}
