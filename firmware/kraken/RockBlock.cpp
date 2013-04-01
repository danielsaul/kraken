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
    iridium.powerOn();
}

void rockblock_off() {
    iridium.powerOff();
}
