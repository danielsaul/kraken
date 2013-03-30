/**
 * imu.h
 *
 * Part of the Kraken Ocean Drifter
 * http://poseidon.sgsphysics.co.uk
 *
 * Priyesh Patel
 * (C) SGS Poseidon Project 2013
 */

#ifndef IMU_H
#define IMU_H

#include "Arduino.h"
#include "Wire.h"
#include "TimerOne.h"

#define IMU_ADDR 0x53

void imu_setup(int16_t* imu_xs, int16_t* imu_ys, int16_t* imu_zs);
void imu_sample();
void imu_measure();
void imu_get(int16_t* output);

#endif
