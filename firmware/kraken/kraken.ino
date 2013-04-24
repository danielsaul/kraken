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


//uint8_t TEMP_ADDR[8] = {0x28, 0xB8, 0x90, 0x64, 0x04, 0x00, 0x00, 0x0F};   //Kraken the First
uint8_t TEMP_ADDR[8] = {0x28, 0xFE, 0xD5, 0x64, 0x04, 0x00, 0x00, 0xF3};     //Kraken the Second

//char SD_LOG[] = "KRAKEN.LOG";
//char SD_GPS[] = "GPS.LOG";
//char SD_IMU[] = "IMU.LOG";

// Sleep counter
volatile uint16_t sleep_cycles = 2700; // 2700 = 6 hours
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

// Iridium Received Msg struct
struct rcvdata {
    uint8_t cmd;
    uint16_t value;
};

void setup() {
    // Setup watchdog
    cli();

    // Clear the watchdog reset flag
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
    rcvdata rcv;

    // Counter +1
    counter_inc();

    // Store counter
    msg.counter = counter_get();

    if (SERIAL_EN) {
        Serial.println("");
        Serial.print("Count: ");
        Serial.println(msg.counter);
    }

    if (imuEnabled()) {
        // Only send IMU data every Nth transmission
        if (msg.counter % imuFrequency() == 0) {
            // Get IMU data
            imu_setup(&msg.imu_x[0], &msg.imu_y[0], &msg.imu_z[0]);
            imu_sample();
        }
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

    if (SERIAL_EN) {
        Serial.print("Tries: ");
        Serial.println(tries);
    }

    // Get temperature
    float temperature = 99.99;
    msg.temp = temperature_get(TEMP_ADDR);

    // Battery Voltage
    float battery_voltage = 99.99;
    msg.battery = battery_get_voltage();

    // Print data to serial 
    if (SERIAL_EN) {
        Serial.print("Hr: ");
        Serial.println(msg.hour);
        Serial.print("Mn: ");
        Serial.println(msg.mins);
        Serial.print("Sc: ");
        Serial.println(msg.secs);
        Serial.print("Lat: ");
        Serial.println(msg.lat);
        Serial.print("Lon: ");
        Serial.println(msg.lon);
        Serial.print("Alt: ");
        Serial.println(msg.alt);
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

    // Load message to be sent into RockBlock Buffer
    bool response = false;
    if (imuEnabled() && (msg.counter % imuFrequency() == 0)) {
        loadMessage((unsigned char*) &msg, sizeof(msg));
    } else {
        loadMessage((unsigned char*) &msg, sizeof(msg) - sizeof(msg.imu_x) - sizeof(msg.imu_y) - sizeof(msg.imu_z));
    }

    // Start a session with Iridium
    tries = 0;
    bool success = false;
    while (tries < 15 && !success) {
        if (tries > 0) {
            if (SERIAL_EN)
                Serial.println("RB: Retry session in 1 min");
            delay(60000);
        }
        success = initiateSession();
        tries++;
    }

    // Check iridium session result
    // Send message result
    if (messageSent()) {
        if (SERIAL_EN)
            Serial.println("RB: Sent");
    } else {
        if (SERIAL_EN)
            Serial.println("RB: Not sent");
        sleep_counter = sleep_cycles - 225; // Only sleep for 30 min if unsuccessful
    }
    
    // Any messages to receive?
    if(SERIAL_EN){
        Serial.print("RB: Messages to receive - ");
        Serial.println(messagesToReceive());
    }
    unsigned long endtime = millis() + (60000 * 5); //Timeout: 5mins
    while(messagesToReceive() > 0 && millis() < endtime){
        if(messageAvailableToRead()){
            //Message in Iridium buffer to read
            int result = readMessage((unsigned char*) &rcv, sizeof(rcv));
            // Correct byte order
            rcv.value = ((rcv.value & 0xff) << 8) | ((rcv.value & 0xff00) >> 8);
            if(result != -1) executeRcvdCommand(rcv.cmd, rcv.value);
        }
        if(messagesWaitingOnNetwork() > 0){
            initiateSession();
        }
    }

    // Turn RockBlock off
    rockblock_off();

    // Only sleep for 30 min if deployed in the last 24 hours / 48 transmissions
    if (msg.counter < 48)
        sleep_counter = sleep_cycles - 225;

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

void executeRcvdCommand(uint8_t cmd, uint16_t val){

    if(SERIAL_EN){
        Serial.print("Command: ");
        Serial.println(cmd);
        Serial.print("Value: ");
        Serial.println(val);
    }
    switch (cmd){
        
        // Change update frequency
        case 0xAA:
            sleep_cycles = val;
            if(SERIAL_EN)
                Serial.println("Update frequency changed.");
            break;

        // Toggle IMU on/off
        case 0xBB:
            if(val){
                setImuEnabled(true);
            }else{
                setImuEnabled(false);
            }
            if(SERIAL_EN)
                Serial.println("IMU toggled.");
            break;

        // Change IMU update frequency
        case 0xCC:
            if(val > 0 && val < 255){
                setImuFrequency(val);
            }
            if(SERIAL_EN)
                Serial.println("IMU frequency changed.");           
            break;
    }

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
