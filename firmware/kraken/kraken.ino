/**
  * kraken.ino
  *
  * Part of the Kraken Ocean Drifter
  * http://poseidon.sgsphysics.co.uk
  *
  * Various Authors
  *
  * (C) SGS Poseidon Project 2013
  */

// Include libraries
//#include <SD.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include "OneWire.h"

// Include Files
#include "debug.h"
#include "RockBlock.h"
#include "gps.h"
#include "counter.h"
#include "temperature.h"
#include "battery.h"
#include "imu.h"

// Settings
const uint8_t STATUS_LED_PIN = A0;
//const uint8_t SD_CS_PIN = 10;

uint8_t TEMP_ADDR[8] = {0x28, 0xB8, 0x90, 0x64, 0x04, 0x00, 0x00, 0x0F};
//char SD_LOG[] = "KRAKEN.LOG";
//char SD_GPS[] = "GPS.LOG";
//char SD_IMU[] = "IMU.LOG";

// Iridium Data Struct
struct data {
    uint16_t counter;
    
    uint8_t hour;
    uint8_t mins;
    uint8_t secs;
    
    int32_t lat;
    int32_t lon;
    int32_t alt;
    
    uint8_t sats;

    float temp;
    
    float battery;

    int16_t imu_x[50];
    int16_t imu_y[50];
    int16_t imu_z[50];
};

void setup() {
    // Setup serial
    Serial.begin(9600);
    if (SERIAL_EN) {
        Serial.println("Kraken");
    }

    // Initialise and turn on status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);

    // Setup SD Card
    /*
    pinMode(SD_CS_PIN, OUTPUT);
    if(!SD.begin(SD_CS_PIN)){
        Serial.println("SD card failed to initialise.");
    }
    */

    // Setup rockblock
    rockblock_init();
    
    // Setup GPS and put to sleep
    gps_setup(); 

    // Setup temperature
    temperature_get(TEMP_ADDR);
   
    // Finished Initialising
    if (SERIAL_EN) Serial.println("\nBooted\n");
    digitalWrite(STATUS_LED_PIN, LOW);
}
 
void loop(){
    // Data
    data msg;

    // Get IMU data
    imu_setup(&msg.imu_x[0], &msg.imu_y[0], &msg.imu_z[0]);
    imu_sample();

    // GPS
    gps_wake();

    uint8_t count = 0;
    uint8_t tries = 0;
    msg.lat = 0;
    msg.lon = 0;
    msg.alt = 0;
    msg.hour = 0;
    msg.mins = 0;
    msg.secs = 0;
    msg.sats = 0;
    uint8_t fx = 0;
    
    while (count < 3 && tries < 100) {
        tries++;

        if (gps_get(&msg.lat, &msg.lon, &msg.alt, &msg.hour, &msg.mins, &msg.secs, &msg.sats, &fx)) {
            count++;
        } else {
            count = 0;
        }
    }

    gps_sleep();

    // Counter +1
    counter_inc();
    msg.counter = counter_get();

    // Get temperature
    float temperature = 99.99;
    msg.temp = temperature_get(TEMP_ADDR);

    // Battery Voltage
    float battery_voltage = 99.99;
    msg.battery = battery_get_voltage();

    // Print data to serial 
    if (SERIAL_EN) {
        Serial.println(msg.lat);
        Serial.println(msg.lon);
        Serial.println(msg.alt);
        Serial.println(msg.hour);
        Serial.println(msg.mins);
        Serial.println(msg.secs);
        Serial.println(msg.sats);
        Serial.println(msg.counter);
        Serial.println(msg.temp);
        Serial.println(msg.battery);
    }

    // Send via RockBlock
    digitalWrite(STATUS_LED_PIN, HIGH);
    if (rockblock_send((unsigned char*) &msg, sizeof(msg))) {
        if (SERIAL_EN)
            Serial.println("RB: Sent");
    } else {
        if (SERIAL_EN)
            Serial.println("RB: Not sent");
    }
    digitalWrite(STATUS_LED_PIN, LOW);

    // Sleep for 2 minutes
    if (SERIAL_EN)
        Serial.println("Sleeping");
    delay(120000);
}

/*
void sdcard_log(data* sentence, int16_t length)
{
    File logFile = SD.open(SD_LOG, FILE_WRITE);
    if (logFile)
    {
        logFile.println();
    }
    logFile.close();
}
*/
