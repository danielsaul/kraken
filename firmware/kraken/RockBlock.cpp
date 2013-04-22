/**
 * RockBlock.cpp
 *
 * Part of the Kraken Ocean Drifter
 * http://poseidon.sgsphysics.co.uk
 *
 * Daniel Saul
 * (C) SGS Poseidon Project 2013
 */

#include "RockBlock.h"

SoftwareSerial rb(RB_RX, RB_TX);
Iridium9602 iridium = Iridium9602(rb);

void rockblock_init() {
    pinMode(RB_SLEEP, OUTPUT);
    digitalWrite(RB_SLEEP, HIGH); // Turn RB on so if can be powered off without timing out
    rockblock_off();
}

bool rockblock_send(unsigned char *msg, int length) {
    uint8_t tries = 0;
    bool success = false;

    while (tries < 15 && !success) {
        if (tries > 0) {
            if (SERIAL_EN)
                Serial.println("RB: Retry in 1 min");
            delay(60000);
        }
        success = rockblock_sendmsg(msg, length);
        tries++;
    }

    return success;
}

bool rockblock_sendmsg(unsigned char* msg, int length) {
    bool sat = iridium.isSatAvailable();
    int sig = iridium.checkSignal();

    if (SERIAL_EN) {
        if(sat) Serial.println("RB: Sat");

        Serial.print("RB: Sig ");
        Serial.println(sig);
    }

    if(sat && (sig >= minimumSignalRequired)) {
        uint8_t i = 0;
        while(!iridium.loadMOMessage(msg, length)) {
            i++;
            if (i > 5) return false;
        }

        if (SERIAL_EN)
            Serial.println("RB: Loaded");

        iridium.initiateSBDSession(responseLost);

        bool lastResult = iridium.lastSessionResult();

        if((lastResult >= 0) && (lastResult <= 4)) {
            return true;
        } 
    }
    return false;
}

void rockblock_on() {
    rb.begin(19200);
    //iridium.powerOn();

    // Set variables
    receivedIdx = 0;

    // Wait 5secs to ensure modem is fully powered down before turning on again
    unsigned long starttime = millis();
    while(millis() < starttime + 5000){}

    if(SERIAL_EN)
        Serial.println("RB: Turning Sat Modem On...");

    digitalWrite(RB_SLEEP, HIGH);
    while(!sendCommandandExpectPrefix("AT", "OK", 500)){}

    if(SERIAL_EN)
        Serial.println("RB: Responding");

    sendCommandandExpectPrefix("ATE0", "OK", 1000);     // Serial Character Echo Off
    sendCommandandExpectPrefix("AT&K0", "OK", 1000);    // No flow control handshaking
    sendCommandandExpectPrefix("ATQ0", "OK", 1000);     // Turn responses on
    // Turn off incoming message alert?
    sendCommandandExpectPrefix("AT&W0", "OK", 1000);    // Write defaults to modem & flush to eeprom
    sendCommandandExpectPrefix("AT&Y0", "OK", 1000);    // Set default reset Profile
    // Set indicator reporting to true?
    // Turn on incoming message alert?
    
    if(SERIAL_EN)
        Serial.println("RB: Setup complete");

}

void rockblock_off() {
    //iridium.powerOff();
    sendCommandandExpectPrefix("AT*F", "OK", 10000);
    digitalWrite(RB_SLEEP, LOW);
    if(SERIAL_EN)
        Serial.println("RB: Turned off");
}

/////

bool sendCommandandExpectPrefix(const char * command, const char * response, unsigned long timeout) {
    sendCommand(command);
    return expectResponse(response, timeout);
}

void sendCommand(const char * command){
    rb.print(command);
    rb.print("\r\n");
}

bool expectResponse(const char * response, unsigned long timeout){
    unsigned long starttime = millis();

    do {
        unsigned long timeleft = timeout - (millis() - starttime);
        if (timeleft == 0) timeleft = 1;
        if (!receiveCmdCRLF(timeleft)) continue;
   
        if(strncmp(receivedCmd, response, strlen(response) == 0)){
            clearReceivedCmd();
            return true;
        }

        // Didn't match, see if it was anything else
        checkUnexpectedResponse();
        clearReceivedCmd();

    } while(timeout == 0 || millis() - starttime < timeout);

    return false;
}

void checkUnexpectedResponse(){
   if(strncmp(receivedCmd, "+SBDIX:", 7) == 0){
        //SBDIX response
   }
   if(strncmp(receivedCmd, "SBDRING", 8) == 0){

   }
   if(strncmp(receivedCmd, "+CIEV:", 6) == 0){

   }
}

// We want an entire line with \n\r at the end
bool receiveCmdCRLF(unsigned long timeout){
    unsigned long endtime = 0;
    char inChar;
    if(timeout > 0)
        endtime = millis() + timeout;

    do{

        while(rb.available()){
            
            if(receivedIdx >= 99) clearReceivedCmd();

            inChar = rb.read();
            if(receivedIdx== 0 && (inChar == '\r' || inChar == '\n')) continue; //Ignore whitespace at start of line
            receivedCmd[receivedIdx++] = (unsigned char)inChar;
            receivedCmd[receivedIdx] = 0;
            if(receivedIdx >= 2 && receivedCmd[receivedIdx - 2] == '\r' && receivedCmd[receivedIdx - 1] == '\n'){
                // We've got out complete line - remove whitespace at end
                while(receivedCmd[receivedIdx-1] == '\r' || receivedCmd[receivedIdx-1] == '\n'){
                    receivedCmd[--receivedIdx] = '\0';
                }
                return true;
            }
        }
    } while(timeout == 0 || millis() < endtime);

    return false;
}

void clearReceivedCmd(){
    receivedIdx = 0;
    receivedCmd[0] = '\0';
}
