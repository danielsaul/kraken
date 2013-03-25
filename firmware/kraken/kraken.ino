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


void setup(){

    #ifdef SERIAL_DEBUG
        // Setup serial for debugging
        Serial.begin(19200);
        Serial.println("------------------");
        Serial.println("|   The Kraken   |");
        Serial.println("------------------");
    #endif

    // Initialise and turn on status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);

}
 
void loop(){
  
}
