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
    unsigned char testmsg[] = "Kraken Initialising";

    if (rockblock_send(&testmsg[0], 19)) {
        if (SERIAL_EN)
            Serial.println("RB: Sent");
    } else {
        if (SERIAL_EN)
            Serial.println("RB: Not sent");
    }
}

bool rockblock_send(unsigned char *msg, int length) {
    rb.begin(19200);
    rockblock_on();

    if (SERIAL_EN) {
        if(iridium.isSatAvailable()) Serial.println("RB: Sat");

        Serial.print("RB: Sig ");
        Serial.println(iridium.checkSignal());
    }

    if (rockblock_sendmsg(msg, length)) {
        rockblock_off();
        return true;
    } else {
        rockblock_off();
        return false;
    }
}

bool rockblock_sendmsg(unsigned char *msg, int length) {
    if(iridium.isSatAvailable() && (iridium.checkSignal() >= minimumSignalRequired)) {
        uint8_t i = 0;
        while(!iridium.loadMOMessage(msg, length)) {
            i += 1;
            if (i > 5) return false;
        }

        if (SERIAL_EN)
            Serial.println("RB: Loaded");
        iridium.initiateSBDSession(responseLost);

        if((iridium.lastSessionResult() >= 0) && (iridium.lastSessionResult() <= 4)) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void rockblock_on() {
    iridium.powerOn();
}

void rockblock_off() {
    iridium.powerOff();
}
