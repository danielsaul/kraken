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
#include "debug.h"

#define ACCEL_ADDR 0x53
#define GYRO_ADDR 0x68

<<<<<<< HEAD
=======
// Enable/disable the IMU
const bool IMU_EN = false;
>>>>>>> 582eec915e24ec3857b903f344a3ba49cf4cf5a0

// Send IMU data every X transmissions
const uint8_t IMU_TRANSMISSIONS = 5;

void imu_setup(int16_t* imu_xs, int16_t* imu_ys, int16_t* imu_zs);
void imu_sample();
void imu_measure();
void imu_get(int16_t* output);

#endif
