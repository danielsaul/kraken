/**
  * gps.h
  *
  * Part of the Kraken Ocean Drifter
  * http://poseidon.sgsphysics.co.uk
  *
  * Greg Brooks & Daniel Saul
  * (C) SGS Poseidon Project 2013
  */

#ifndef GPS_H
#define GPS_H

#include "Arduino.h"



const uint16_t gps_timeout = 10000;

void gps_setup();
bool setSeaMode();
bool setOutputUBX();
bool setNavSolOff();
bool setNavPosLLH();

void gps_sleep();
void gps_wake();
void sendUBX(uint8_t *MSG, uint8_t len);
bool getUBX_ACK(uint8_t *MSG);


#endif

