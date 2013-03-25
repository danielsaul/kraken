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
#include <SD.h>
#include <SoftwareSerial.h>
#include "OneWire.h"
#include "DallasTemperature.h"


// Settings
#define SERIAL_DEBUG
const uint8_t STATUS_LED_PIN = A0;
const uint8_t TEMP_PIN = 2;
const uint8_t SD_CS_PIN = 10;



void setup(){

    #ifdef SERIAL_DEBUG
        // Setup serial for debugging
        Serial.begin(115200);
        Serial.println("------------------");
        Serial.println("|   The Kraken   |");
        Serial.println("------------------");
    #endif

    // Initialise and turn on status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);

    // Setup SD Card
    pinMode(SD_CS_PIN, OUTPUT);
    if(!SD.begin(SD_CS_PIN)){
        #ifdef SERIAL_DEBUG
            Serial.begin(115200);
            Serial.println("SD card failed to initialise.")
        #endif
    }

    // Setup GPS
    // GPS stuff should have something in it that changes baud to GPS baud each time
    // Don't need to change to lower baud though - 9600 is fine.
    // Please please please put all the GPS stuff in a seperate file.
    
    // Setup RockBlock
    //

    // Finished Initialising
    #ifdef SERIAL_DEBUG
        Serial.begin(115200);
        Serial.println("\n Kraken booted successfully. \n");
    #endif
    digitalWrite(STATUS_LED_PIN, LOW);
}
 
void loop(){
 

    // Counter +1
    //
    // Get temperature
    //
    // Turn on GPS, get data, turn off GPS
    //
    // Get IMU data
    //
    // Store data on SD Card
    //
    // Print data to serial ifdef SERIAL_DEBUG
    //
    // Pack all data into binary format
    //
    // Send via RockBlock
    //
    // Check for messages from iridium
    //
    // Sleep for a while
    // if within first 3 hours of being turned on, do loop more frequently
    // else every x hours, based on GPS time?

}
