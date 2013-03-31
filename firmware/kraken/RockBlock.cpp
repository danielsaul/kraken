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
    rb.begin(19200);
    rockblock_on();

    if (SERIAL_EN) {
        if(iridium.isSatAvailable()) Serial.println("RB: Sat");

        Serial.print("RB: Sig ");
        Serial.println(iridium.checkSignal());
    }

    bool resp = rockblock_sendmsg(msg, length);

    rockblock_off();
    return resp;
}

bool rockblock_sendmsg(unsigned char *msg, int length) {
    if(iridium.isSatAvailable() && (iridium.checkSignal() >= minimumSignalRequired)) {
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
    iridium.powerOn();
}

void rockblock_off() {
    iridium.powerOff();
}
