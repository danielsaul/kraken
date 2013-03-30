/**
 * battery.h
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

#ifndef BATTERY_H
#define BATTERY_H

#include "Arduino.h"

#define BATTERY_PIN 6
/*
 * R1 = 33K
 * R2 = 22K
 *
 * Ratio = R2 / (R1 + R2)
 */
#define BATTERY_RESISTOR_RATIO 0.4
#define BATTERY_REFERENCE_VOLTS 3.3
#define BATTERY_ADJUSTMENT 1.01

float battery_get_voltage();

#endif
