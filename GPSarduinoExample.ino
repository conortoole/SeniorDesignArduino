#include <bluefruit.h>
#include <nrf_soc.h> 
#include <BLEAdvertising.h>
#include <BLEUuid.h>
#include <math.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

#include <rimLED.h>
#include <rimBattery.h>

#define MANUFACTURER_ID   0x0059 //Nordic Manufacturer ID

const char* messageData = "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

TinyGPSPlus gps;

rimBattery bat;

rimLED batLED(10);
rimLED gpsLED(11);
rimLED bleLED(12);

bool takeoff_logged = false;
float initialLat = 0;
float initialLng = 0;
float initialAlt = 0;

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

void updatePacketIDMessage() {

        uint8_t newMessageData[29] = {
        0x00,                   // 0    AD FLags
        0x0D,                   // 1    AD App
        0x00,                   // 2    AD Counter
        0x00,                   // 3    Message Type & Version
        //
        0x12,                   // 4    ID Type, UAS Type
        0x4C, 0x33, 0x48, 0x41, // 5    UAS ID Pefix
        0x0F,                   // 9    UAS ID Length Code
        0x00, 0x00, 0x00, 0x00, // 10   UAS ID 
        0x00, 0x00, 0x00, 0x00, //
        0x00, 0x00, 0x00, 0x00, //
        0x00, 0x00, 0x00,       //
        //
        0x00, 0x00, 0x00,       // 26   Reserved
        0x00                    // 28   CRC
    }; 

    for (int i = 0; i < 15; i++) {
        newMessageData[10+i] = deviceID[i];
    }

    Serial.println("Updated Bluetooth Message: ");
    for (int i = 0; i < 29; i++) {
        Serial.print(newMessageData[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    Bluefruit.Advertising.stop(); 
    Bluefruit.Advertising.clearData();

    int err = Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, newMessageData, sizeof(newMessageData));
    if (!err) {
        Serial.println("! Failed to add data in updatePacketIDMessage() !");
    }

    err = Bluefruit.Advertising.start();
    if (!err) {
        Serial.println("! Failed to start Bluetooth Transmission in updatePacketIDMessage() !");
    }

    Serial.println();
}

void updatePacketLocationMessage(float speed_mps, float course_degrees, float latitude, float longitude, float altitude, int hours, int minutes, int seconds, int centiseconds) {

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
    Serial.printf("\nTime: %d:%d:%d.%d\nProgram ms: ", hours, minutes, seconds, centiseconds);
    Serial.print(millis());
    Serial.println();

    Serial.println("Updated Bluetooth Message: ");
    for (int i = 0; i < 29; i++) {
        Serial.print(newMessageData[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    Bluefruit.Advertising.stop(); 
    Bluefruit.Advertising.clearData();

    int err = Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, newMessageData, sizeof(newMessageData));
    if (!err) {
        Serial.println("! Failed to add data in updatePacketLocationMessage() !");
    }

    err = Bluefruit.Advertising.start();
    if (!err) {
        Serial.println("! Failed to start Bluetooth Transmission in updatePacketLocationMessage() !");
    }

    Serial.println();
}

void setup() {
    serial.begin(9600);
    Serial.begin(9600);

    while ( !Serial ) delay(10);

    strcpy(deviceID, getID());

    Serial.println("Remote ID Module");
    Serial.println(deviceID);
    Serial.println("--------------------------\n");

    int err = Bluefruit.begin();
    if (!err) {
    Serial.println("! Failed to initialize Bluetooth !");
    }

    Bluefruit.setTxPower(8);    

    uint8_t messageDataLen = strlen(messageData); 
    Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, messageData, messageDataLen);

    err = Bluefruit.Advertising.start();
    if (!err) {
    Serial.println("! Failed to start Bluetooth Transmission !");
    }
    Serial.println("Advertising started");
    Serial.printf("Broadcasting with MANUFACTURER_ID = 0x%04X\n", MANUFACTURER_ID);
}

void loop() {

    while (serial.available() > 0) {
        if (gps.encode(serial.read())) {

            updatePacketIDMessage();

            bool updated = gps.location.isUpdated();
            Serial.printf("GPS Update Status: %d\n", updated);

            if (updated) {
                gpsLED.On();
                Serial.println("GPS LED turned OFF");
            }
            else {
                gpsLED.Off();
                Serial.println("GPS LED turned OFF");
            }

            int err = Bluefruit.Advertising.isRunning();
            Serial.printf("Bluetooth Status: %d\n", err);

            err = serial.available();
            Serial.printf("UART Serial Status: %d\n", err);

            err = gps.location.isUpdated();
            Serial.printf("GPS Update Status: %d\n", err);

            int num_satellites = gps.satellites.value();
            Serial.printf("GPS Satellites Connected: %d\n", num_satellites);
            
            float speed = gps.speed.mps();
            float course = gps.course.deg();
            float latitude = gps.location.lat();
            float longitude = gps.location.lng();
            float altitude = gps.altitude.meters();

            int hour = gps.time.hour();
            int minute = gps.time.minute();
            int second = gps.time.second();
            int centisecond = gps.time.centisecond();

            if ( (!takeoff_logged) && num_satellites >= 4) {
                initialLat = latitude;
                initialLng = longitude;
                initialAlt = altitude;
                takeoff_logged = true;
            }

            bat.readBattery();
            float battMeasure = bat.vBat;
            batLED.status = bat.updateLED(batLED);

            Serial.print("Battery Voltage " ); 
            Serial.println(battMeasure);

            updatePacketLocationMessage(speed, course, latitude, longitude, altitude, hour, minute, second, centisecond);
        }
    }
}

//----------------------------------
// Output Packet Received as...

//0: HCI Packet Type            hci_h4.type                           Preamble                             
//1: Event Code                 bthci_evt.code                        Acc Addr                             
//2: Parameter Total Length     bthci_evt.param_length                Acc Addr                                 
//3: Sub Event                  bthci_evt.le_meta_subevent            Acc Addr                              
//4: Num Reports                bthci_evt.le_num_reports              Acc Addr                             
//5: Event Type                 bthci_evt.le_advts_event_type         PDU Header
//6: Peer Address Type          bthci_evt.le_peer_Address_type        PDU Header
//7-12: BD_ADDR                 bthci_evt.bd_addr                     AD Addr
//13: Data Length               bthci_evt.data_length                 AD Flags
//14: Length                    btcommon.eir_ad.entry.length          AD Flags
//15: Type                      btcommon.eir_ad.entry.type            AD Flags
//16: LE Limited Discoverable   "".flags.le_limited_discoverable_mode AD Flags
//17: Length                    btcommon.eir_ad.entry.length          AD Counter
//18: Type                      btcommon.eir_ad.entry.type            Message Type / Protocol Version
//19-20: Company ID             btcommon.eir_ad.entry.company_id      Unique ID 
//21-43: Data                   btcommon.eir_ad.entry.data            Unique ID / Location Message / CRC
//44: RSSI                      bthci_evt.rssi                        CRC

//----------------------------------