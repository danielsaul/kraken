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
  
  Serial.println(iridium.isSatAvailable());
  Serial.println(iridium.checkSignal());
  
}

void rockblock_on(){
 
 iridium.powerOn();
  
}

void rockblock_off(){
  
  iridium.powerOff();
  
}


