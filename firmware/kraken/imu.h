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

// IMU_EN & IMU_TRANSMISSIONS now in imu.cpp
bool imuEnabled();
void setImuEnabled(bool a);
uint8_t imuFrequency();
void setImuFrequency(uint8_t i);

void imu_setup(int16_t* imu_xs, int16_t* imu_ys, int16_t* imu_zs);
void imu_sample();
void imu_measure();
void imu_get(int16_t* output);

#endif
