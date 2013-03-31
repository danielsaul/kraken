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
#include <avr/sleep.h>
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

// Send IMU data every X transmissions
const uint8_t imu_transmissions = 5;

// Sleep counter
const uint16_t sleep_cycles = 2700; // 2700 = 6 hours
volatile uint16_t sleep_counter = sleep_cycles; // Initialise at max value

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
    // Setup watchdog
    cli();

    //Clear the watchdog reset flag
    MCUSR &= ~(1<<WDRF);

    // Set WDCE so we can change the prescaler
    WDTCSR |= 1<<WDCE | 1<<WDE;

    // Set watchdog prescaler to 8 seconds
    WDTCSR = 1<<WDP0 | 1<<WDP3;

    // Setup sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    sei();

    // Setup serial
    Serial.begin(9600);
    if (SERIAL_EN) {
        Serial.println("Kraken");
    }

    // Initialise and turn on status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    if (SERIAL_EN) 
        digitalWrite(STATUS_LED_PIN, HIGH);
    else
        digitalWrite(STATUS_LED_PIN, LOW);

    // Setup SD Card
    /*
    pinMode(SD_CS_PIN, OUTPUT);
    if(!SD.begin(SD_CS_PIN)){
        Serial.println("SD card failed to initialise.");
    }
    */

    // Setup rockblock
    if (SERIAL_EN) {
        Serial.println("RB: Starting up");
        delay(50);
    }
    rockblock_init();
    
    // Setup GPS and put to sleep
    gps_setup(); 

    // Setup temperature
    temperature_get(TEMP_ADDR);
   
    // Finished Initialising
    if (SERIAL_EN) {
        Serial.println("\nBooted\n");
        digitalWrite(STATUS_LED_PIN, LOW);
    }
}
 
void loop(){
    if (sleep_counter < sleep_cycles) {
        sleep_mode();
        return;
    }

    cli();

    // Disable the watchdog interrupt
    WDTCSR &= ~(1<<WDIE);

    // Disable sleep mode
    sleep_disable();

    sei();

    // Reset the sleep counter
    sleep_counter = 0;

    // Rockblock on
    rockblock_on();

    // Data struct
    data msg;

    // Counter +1
    counter_inc();

    // Store counter
    msg.counter = counter_get();

    if (SERIAL_EN) {
        Serial.println("");
        Serial.print("Count: ");
        Serial.println(msg.counter);
    }

    // Only send IMU data every Nth transmission
    if (msg.counter % imu_transmissions == 0) {
        // Get IMU data
        imu_setup(&msg.imu_x[0], &msg.imu_y[0], &msg.imu_z[0]);
        imu_sample();
    }

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

    if (SERIAL_EN)
        Serial.print("Tries: ");
        Serial.println(tries);

    // Get temperature
    float temperature = 99.99;
    msg.temp = temperature_get(TEMP_ADDR);

    // Battery Voltage
    float battery_voltage = 99.99;
    msg.battery = battery_get_voltage();

    // Print data to serial 
    if (SERIAL_EN) {
        Serial.print("Lat: ");
        Serial.println(msg.lat);
        Serial.print("Lon: ");
        Serial.println(msg.lon);
        Serial.print("Alt: ");
        Serial.println(msg.alt);
        Serial.print("Hr: ");
        Serial.println(msg.hour);
        Serial.print("Mn: ");
        Serial.println(msg.mins);
        Serial.print("Sc: ");
        Serial.println(msg.secs);
        Serial.print("Sats: ");
        Serial.println(msg.sats);
        Serial.print("Temp: ");
        Serial.println(msg.temp);
        Serial.print("Batt: ");
        Serial.println(msg.battery);
    }

    // Send via RockBlock
    if (SERIAL_EN)
        digitalWrite(STATUS_LED_PIN, HIGH);

    bool rockblock_response = false;
    if (msg.counter % imu_transmissions == 0) {
        rockblock_response = rockblock_sendmsg((unsigned char*) &msg, sizeof(msg));
    } else {
        rockblock_response = rockblock_sendmsg((unsigned char*) &msg, sizeof(msg) - sizeof(msg.imu_x) - sizeof(msg.imu_y) - sizeof(msg.imu_z));
    }

    // Turn RockBlock off
    rockblock_off();

    if (SERIAL_EN) {
        if (rockblock_response)
            Serial.println("RB: Sent");
        else 
            Serial.println("RB: Not sent");
    }

    if (SERIAL_EN)
        digitalWrite(STATUS_LED_PIN, LOW);
    
    // Sleep 
    if (SERIAL_EN) {
        Serial.println("Sleeping");
        delay(50);
    }

    cli();

    // Enable the watchdog interrupt without a system reset
    WDTCSR |= 1<<WDIE;

    // Enable sleep mode
    sleep_enable();

    sei();

    sleep_mode();
}

ISR(WDT_vect) {
    sleep_counter++;
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
