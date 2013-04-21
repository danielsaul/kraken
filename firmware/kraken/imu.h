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

const bool IMU_EN = true;

// Send IMU data every X transmissions
const uint8_t IMU_TRANSMISSIONS = 5;

void imu_setup(int16_t* imu_xs, int16_t* imu_ys, int16_t* imu_zs);
void imu_sample();
void imu_measure();
void imu_get(int16_t* output);

#endif
