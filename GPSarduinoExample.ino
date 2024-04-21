#include <TinyGPSPlus.h>
#include <bluefruit.h>
#include <SoftwareSerial.h>
#include <rimLED.h>
#include <rimBattery.h>

// Create instances of GPS and Bluetooth
SoftwareSerial serial(A0, A1); //   A0 is RX    A1 is TX
TinyGPSPlus gps;
BLEUart bleuart;

rimBattery bat;

rimLED batLED(10);
rimLED gpsLED(11);
rimLED bleLED(12);

// Create a timer for Bluetooth advertising updates
unsigned long bluetoothTimer = 0;
const unsigned long BLUETOOTH_INTERVAL = 1000; // Update every 1 second

// GPS and Bluetooth status variables
bool takeoff_logged = false;
float initialLat = 0.0;
float initialLng = 0.0;
float initialAlt = 0.0;

void processGPSData() {
    // Update GPS LED status
    Serial.println("Entered processGPSData()");

    if (gps.time.isValid()) {
        gpsLED.On();
    } else {
        gpsLED.Off();
    }

    // Check if takeoff location needs to be logged
    if (!takeoff_logged && gps.satellites.value() >= 4) {
        initialLat = gps.location.lat();
        initialLng = gps.location.lng();
        initialAlt = gps.altitude.meters();
        takeoff_logged = true;
    }

    // Read battery voltage and update battery LED
    bat.readBattery();
    float battMeasure = bat.vBat;
    batLED.status = bat.updateLED(batLED);
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

void updateBluetoothAdvertising() {
    
    // Create a new message data array
    uint8_t newMessageData[29] = {
        0x00,                   // 0    AD Flags
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

    // Fill the message data array with GPS information
    float latitude = gps.location.lat();
    float longitude = gps.location.lng();
    float altitude = gps.altitude.meters();
    float speed = gps.speed.mps();
    float course = gps.course.deg();
    int hours = gps.time.hour();
    int minutes = gps.time.minute();
    int seconds = gps.time.second();
    int centiseconds = gps.time.centisecond();

    // Convert and store GPS data in the message data array
    int32_t latitude_out = convertLatLon(latitude);
    memcpy(&newMessageData[8], &latitude_out, sizeof(latitude_out));

    int32_t longitude_out = convertLatLon(longitude);
    memcpy(&newMessageData[12], &longitude_out, sizeof(longitude_out));

    uint16_t altitude_out = convertAltitude(altitude);
    memcpy(&newMessageData[16], &altitude_out, sizeof(altitude_out));
    memcpy(&newMessageData[18], &altitude_out, sizeof(altitude_out));

    float speed_ns, speed_ew;
    bool speed_ns_multiplier, speed_ew_multiplier;
    calculateSpeeds(speed, course, speed_ns, speed_ew);
    int8_t speed_ns_out = convertSpeed(speed_ns, speed_ns_multiplier);
    int8_t speed_ew_out = convertSpeed(speed_ew, speed_ew_multiplier);
    newMessageData[5] = speed_ns_out;
    newMessageData[6] = speed_ew_out;
    newMessageData[4] = (speed_ns_multiplier ? 0x02 : 0x00) + (speed_ew_multiplier ? 0x01 : 0x00);

    uint16_t time_out = (6000 * minutes) + (100 * seconds) + centiseconds;
    memcpy(&newMessageData[24], &time_out, sizeof(time_out));


    // Stop the current advertising
    Bluefruit.Advertising.stop();

    // Clear the existing advertising data
    Bluefruit.Advertising.clearData();

    // Add the new message data to the advertising payload
    int err = Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, newMessageData, sizeof(newMessageData));
    if (!err) {
        Serial.println("Failed to add data to Bluetooth advertising!");
    }

    Serial.printf("\n%d:- %.3f, %.3f, %.3f, %.3f, %.3f, %d:%d:%d.%d\n", millis(), speed, course, latitude, longitude, altitude, hours, minutes, seconds, centiseconds);

    // Start advertising
    err = Bluefruit.Advertising.start();
    if (err) {
        bleLED.On();
    } else {
        bleLED.Off();
    }
}


void setup() {

    // Initialize Bluetooth
    Bluefruit.begin();
    Bluefruit.setTxPower(8);
    Bluefruit.setName("RemoteID");
    bleuart.begin();

    // Initialize LED indicators
    gpsLED.On();    
    bleLED.On();
    batLED.On();
}

void loop() {
    // Check for incoming GPS data
    while (serial.available() > 0) {
        if (gps.encode(serial.read())) {
            processGPSData();
        }
    }

    // Check if it's time to update Bluetooth advertising
    if (millis() - bluetoothTimer >= BLUETOOTH_INTERVAL) {
        bluetoothTimer = millis();
        updateBluetoothAdvertising();
    }
}