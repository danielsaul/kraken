/**
 * counter.h
 *
 * Part of the Kraken Ocean Drifter
 * http://poseidon.sgsphysics.co.uk
 *
 * From the Apex III project
 * http://www.apexhab.org/apex-iii/
 *
 * Priyesh Patel
 *
 * (c) Copyright ApexHAB 2011
 * team@apexhab.org
 */

#ifndef COUNTER_H
#define COUNTER_H

#include "Arduino.h"
#include <EEPROM.h>

#define EEPROM_LOW_BYTE 0
#define EEPROM_HIGH_BYTE 1

#define EEPROM_IMU_LOW_BYTE 2
#define EEPROM_IMU_HIGH_BYTE 3

uint16_t counter_get();
void counter_set(uint16_t new_counter);
void counter_inc();
void counter_reset();

uint16_t imu_counter_get();
void imu_counter_set(uint16_t new_counter);
void imu_counter_inc();
void imu_counter_reset();

#endif
