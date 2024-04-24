#include <TinyGPSPlus.h>
#include <bluefruit.h>
#include <SoftwareSerial.h>
#include <rimLED.h>
#include <rimBattery.h>

#define MANUFACTURER_ID   0x0059 //Nordic Manufacturer ID

uint8_t messageData[29] = {
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

// Create instances of GPS and Bluetooth
SoftwareSerial serial(A0, A1); //   A0 is RX    A1 is TX
TinyGPSPlus gps;
BLEUart bleuart;

rimBattery bat;

rimLED batLED(10);
rimLED gpsLED(11);
rimLED bleLED(12);

float speed;
float course;
float latitude;
float longitude;
float altitude;
float initialLat;
float initialLng;
float initialAlt;

uint8_t hour;
uint8_t minute;
uint8_t second;
uint8_t centisecond;

// Create a timer for Bluetooth advertising updates
unsigned long bluetoothTimer = 0;
const unsigned long BLUETOOTH_INTERVAL = 750; // Update every 1 second

// GPS and Bluetooth status variables
bool takeoff_logged = false;

void processGPSData() {

    speed = gps.speed.mps();  
    course = gps.course.deg();
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    altitude = gps.altitude.meters();
    
    hour = gps.time.hour();
    minute = gps.time.minute();
    second = gps.time.second();
    centisecond = gps.time.centisecond(); 

    Serial.printf("\n%d - stored     : %.3f, %.3f, %.3f, %.3f, %.3f, %d:%d:%d.%d\n", millis(), speed, course, latitude, longitude, altitude, hour, minute, second, centisecond);
    Serial.printf("Bluetooth status: %d\n", Bluefruit.Advertising.isRunning());

    if (gps.time.isValid()) {
        gpsLED.On();
    } else {
        gpsLED.Off();
    }

    // Check if takeoff location needs to be logged
    if (!takeoff_logged && gps.satellites.value() >= 4) {
        initialLat = latitude;
        initialLng = longitude;
        initialAlt = altitude;
        takeoff_logged = true;
    }

    // Read battery voltage and update battery LED
    bat.readBattery();
    float battMeasure = bat.vBat;
    batLED.status = bat.updateLED(batLED);

    Serial.printf("%d - Message (stored)     : ", millis());
    for (int i = 0; i < 29; i++) {
        Serial.print(messageData[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
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

    messageData[5] = speed_ns_out;
    messageData[6] = speed_ew_out;

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

    messageData[4] += buffer[0];

    // --------------------------------------------------------------------------
    // Vertical Speed

    float vertical_speed = sqrt((speed_mps * speed_mps) - (speed_ns * speed_ns) - (speed_ew * speed_ew));

    int8_t speed_vertical_out = convertVerticalSpeed(vertical_speed);
    messageData[7] = speed_vertical_out;

    // --------------------------------------------------------------------------
    // Latitude                 Longitude

    int32_t latitude_out = convertLatLon(latitude);
    messageData[8] = (latitude_out >> 24) & 0xFF;
    messageData[9] = (latitude_out >> 16) & 0xFF;
    messageData[10] = (latitude_out >> 8) & 0xFF;
    messageData[11] = latitude_out & 0xFF;

    int32_t longitude_out = convertLatLon(longitude);
    messageData[12] = (longitude_out >> 24) & 0xFF;
    messageData[13] = (longitude_out >> 16) & 0xFF;
    messageData[14] = (longitude_out >> 8) & 0xFF;
    messageData[15] = longitude_out & 0xFF;

    // --------------------------------------------------------------------------
    // Altitude

    uint16_t altitude_out = convertAltitude(altitude);
    messageData[16] = (altitude_out >> 8) & 0xFF;
    messageData[17] = altitude_out & 0xFF;
    messageData[18] = (altitude_out >> 8) & 0xFF;
    messageData[19] = altitude_out & 0xFF;

    // --------------------------------------------------------------------------
    // Height Above Takeoff

    float height_above_takeoff = altitude - initialAlt;
    uint16_t height_above_takeoff_out = convertAltitude(height_above_takeoff);
    messageData[20] = (height_above_takeoff_out >> 8) & 0xFF;
    messageData[21] = height_above_takeoff_out & 0xFF;

    // --------------------------------------------------------------------------
    // Timestamp

    uint16_t time_out = (6000 * minutes) + (100 * seconds) + centiseconds;
    messageData[24] = (time_out >> 8) & 0xFF;
    messageData[25] = time_out & 0xFF;

    // --------------------------------------------------------------------------

    Serial.println();

    Serial.printf("%d - Message (transmitted): ", millis());
    for (int i = 0; i < 29; i++) {
        Serial.print(messageData[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void setup() {
    delay(1000);
    serial.begin(9600);
    Serial.begin(9600);

    while (!Serial) ;

    Serial.println("Bluefruit52 Beacon Example");
    Serial.println("--------------------------\n");

    int err = Bluefruit.begin();
    if (!err) {
        Serial.println("! Failed to initialize Bluetooth !");
    }

    Bluefruit.setTxPower(8);
    Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED); 

    err = Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, messageData, sizeof(messageData));
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
    
    gpsLED.On();    
    batLED.On();
}

void loop() {

    while (serial.available() > 0) {

        Serial.print(".");

        if (gps.encode(serial.read())) {
            processGPSData();
        }

    }

    if (millis() - bluetoothTimer >= BLUETOOTH_INTERVAL) {
        bluetoothTimer = millis();
        updatePacketLocationMessage(speed, course, latitude, longitude, altitude, hour, minute, second, centisecond);

        Serial.printf("\n%d- transmitted: %.3f, %.3f, %.3f, %.3f, %.3f, %d:%d:%d.%d\n", millis(), speed, course, latitude, longitude, altitude, hour, minute, second, centisecond);

        Bluefruit.Advertising.clearData(); 

        Serial.print("Packet cleared...");

        int err = Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, messageData, sizeof(messageData)); //update the packet with the gps data 
        if (!err) {
            Serial.println("! Failed to add data in updatePacketLocationMessage() !");
        }

        Serial.println("...Data added.");

        err = Bluefruit.Advertising.isRunning();
        Serial.printf("Bluetooth status: %d\n", err);
        if (!err) {  
            bleLED.Off();
        }
        else {
            bleLED.On();
        }
    }
}