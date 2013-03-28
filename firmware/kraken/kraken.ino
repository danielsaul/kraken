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
#include "EEPROM.h"

// Include Files
#include "RockBlock.h"
#include "gps.h"
#include "counter.h"
#include "temperature.h"
#include "battery.h"

// Settings
const uint8_t STATUS_LED_PIN = A0;
const uint8_t SD_CS_PIN = 10;

uint8_t TEMP_ADDR[8] = {}; // NEEDS SETTING
char SD_LOG[] = "KRAKEN.LOG";
char SD_GPS[] = "GPS.LOG";
char SD_IMU[] = "IMU.LOG";


// Data struct
struct Data{
    uint16_t counter;
    
    uint8_t hour;
    uint8_t mins;
    uint8_t secs;
    
    uint32_t lat;
    uint32_t lon;
    uint32_t alt;
    
    uint8_t sats;

    float temp;
    
    float battery;

    uint8_t imu;  // IMU data on its way?

}

void setup(){

    // Setup serial for debugging
    Serial.begin(9600);
    Serial.println("------------------");
    Serial.println("|   The Kraken   |");
    Serial.println("------------------");

    // Initialise and turn on status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);

    // Setup SD Card
    pinMode(SD_CS_PIN, OUTPUT);
    if(!SD.begin(SD_CS_PIN)){
            Serial.println("SD card failed to initialise.");
    }

    // Setup rockblock
    rockblock_init();
    
    // Setup GPS
    gps_setup(); 
   
    // Finished Initialising
    Serial.println("\nKraken booted successfully. \n");
    digitalWrite(STATUS_LED_PIN, LOW);
}
 
void loop(){
 
  while(1)
  {
    uint8_t fx,st = 0;
    for (int j = 0; j < 3; j++){
      int32_t lat,lon,alt;
      uint8_t b = getLocation(&lat,&lon,&alt);
      char buf[50];
      
      snprintf(buf,50,"position: %li, %li, %li",lat,lon,alt);
       Serial.println(buf);
  
      
      
       uint8_t hr,mn,sc;
       b = gps_get_time(&hr,&mn,&sc);
       snprintf(buf,50,"time: %d:%d:%d",hr,mn,sc);
       Serial.println(buf);
  
      
      
       
       b = gps_check_lock(&fx,&st);   
       snprintf(buf,50,"lock: %d, %d",fx,st);
       Serial.println(buf);
       
       delay(500);
   
      
    }
    if ((fx == 3 || fx == 2)&&(st>6)){
      gps_sleep();
      delay(5000);
      delay(5000);
      delay(5000);
      delay(5000);
      gps_wake();
    }
  
  }

    // Counter +1
    counter_inc();

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

void sdcard_log(data* sentence, int16_t length)
{

    File logFile = SD.open(SD_LOG, FILE_WRITE);
    if (logFile)
    {
        logFile.println();
    }
    logFile.close();

}
