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

    uint16_t imu_counter;
    uint8_t imu_hour;
    uint8_t imu_mins;
    uint16_t imu_position;
    int16_t imu_x[50];
    int16_t imu_y[50];
    int16_t imu_z[50];
};

// IMU Data
uint8_t imu_hour = 0;
uint8_t imu_mins = 0;

uint16_t imu_position = 150;

int16_t imu_x[150];
int16_t imu_y[150];
int16_t imu_z[150];

void setup() {

    // Setup serial
    Serial.begin(9600);
    if (SERIAL_EN) {
        Serial.println("----------");
        Serial.println("| Kraken |");
        Serial.println("----------");
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

    // Setup IMU
    imu_setup(&imu_x[0], &imu_y[0], &imu_z[0]);
   
    // Finished Initialising
    if (SERIAL_EN) Serial.println("\nKraken booted successfully.\n");
    digitalWrite(STATUS_LED_PIN, LOW);
}
 
void loop(){
    // Data
    data msg;

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

    // Get IMU data
    if (imu_position >= 150) {
        if (SERIAL_EN)
            Serial.println("IMU: Taking new sample");
        imu_sample();
        imu_position = 0;

        imu_counter_inc();

        imu_hour = msg.hour;
        imu_mins = msg.mins;
    }

    msg.imu_counter = imu_counter_get();
    msg.imu_position = imu_position;

    // Print data to serial 
    if (SERIAL_EN) {
        char buf[50];
        snprintf(buf, 50, "position: %li, %li, %li", msg.lat, msg.lon, msg.alt);
        Serial.println(buf);

        snprintf(buf, 50, "time: %d:%d:%d", msg.hour, msg.mins, msg.secs);
        Serial.println(buf);

        snprintf(buf, 50, "lock: %d, %d", fx, msg.sats);
        Serial.println(buf);

        Serial.print("count: ");
        Serial.println(msg.counter);

        Serial.print("temp: ");
        Serial.println(msg.temp);

        Serial.print("battery: ");
        Serial.println(msg.battery);
    }

    // Send via RockBlock
    digitalWrite(STATUS_LED_PIN, HIGH);
    if (rockblock_send((unsigned char*) &msg, sizeof(msg))) {
        if (SERIAL_EN)
            Serial.println("RockBlock: Message sent");
    } else {
        if (SERIAL_EN)
            Serial.println("RockBlock: Message could not be sent");
    }
    digitalWrite(STATUS_LED_PIN, LOW);

    // Sleep for 2 minutes
    if (SERIAL_EN)
        Serial.println("Kraken: Sleeping");
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
