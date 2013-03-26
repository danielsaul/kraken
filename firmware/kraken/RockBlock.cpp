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

void rockblock_init(){
  rb.begin(19200);
  rockblock_on();
  
  if(iridium.isSatAvailable()) Serial.println("RockBlock: Satellite Available.");
  Serial.print("RockBlock: Signal Strength ");
  Serial.println(iridium.checkSignal());
  unsigned char testmsg[] = "Kraken Initialising.";
  if(rockblock_sendmsg(&testmsg[0], 20)){
      Serial.println("RockBlock: Test message successful.");
  }else{
      Serial.println("RockBlock: Test message unsuccessful.....");
  }
  rockblock_off();
}

bool rockblock_sendmsg(unsigned char *msg, int length)
{
    if(iridium.isSatAvailable() && (iridium.checkSignal() >= minimumSignalRequired)){
        uint8_t i = 0;
        while(!iridium.loadMOMessage(msg, length)){
            i += 1;
            if (i > 5) return false;
        }
        
        Serial.println("message loaded");
        iridium.initiateSBDSession(responseLost);
        if((iridium.lastSessionResult() >= 0) && (iridium.lastSessionResult() <= 4)){
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}

void rockblock_on(){
 iridium.powerOn();
}

void rockblock_off(){
  iridium.powerOff();
}


