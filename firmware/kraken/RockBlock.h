/**
  * RockBlock.h
  *
  * Part of the Kraken Ocean Drifter
  * http://poseidon.sgsphysics.co.uk
  *
  * Daniel Saul
  * (C) SGS Poseidon Project 2013
  */

#ifndef ROCKBLOCK_H
#define ROCKBLOCK_H

#include "Arduino.h"
#include "Iridium9602.h"
#include <SoftwareSerial.h>
#include "debug.h"

const uint8_t RB_RX = 5;     // Serial RX
const uint8_t RB_TX = 6;     // Serial TX
const uint8_t RB_CTS = 3;    
const uint8_t RB_RTS = 4;
const uint8_t RB_SLEEP = 7;  // POWER ON/OFF 
const uint8_t RB_RING = 8;   // RING INDICATOR
const uint8_t RB_NET = 9;    // NETWORK AVAILABLE

const uint8_t minimumSignalRequired = 2;
const uint16_t maxTelemetryLength = 340;
const uint16_t responseLost = 30000;

void rockblock_init();
bool rockblock_send(unsigned char* msg, int length);
bool rockblock_sendmsg(unsigned char* msg, int length);
void rockblock_on();
void rockblock_off();

#endif
