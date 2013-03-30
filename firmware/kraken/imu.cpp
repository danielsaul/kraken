/**
 * imu.cpp
 *
 * Part of the Kraken Ocean Drifter
 * http://poseidon.sgsphysics.co.uk
 *
 * Priyesh Patel
 * (C) SGS Poseidon Project 2013
 */

#include "imu.h"

uint16_t _imu_sample_pos;
int16_t* _imu_xs;
int16_t* _imu_ys;
int16_t* _imu_zs;

/*
    Set up the IMU. This should be called in the main setup().
*/
void imu_setup(int16_t* imu_xs, int16_t* imu_ys, int16_t* imu_zs)
{
    Wire.begin();

    // Set range
    Wire.beginTransmission(IMU_ADDR);
    Wire.write(0x31);
    Wire.write(0x01);
    Wire.endTransmission();

    // Measurement mode
    Wire.beginTransmission(IMU_ADDR);
    Wire.write(0x2D);
    Wire.write(0x08);
    Wire.endTransmission();

    _imu_xs = imu_xs;
    _imu_ys = imu_ys;
    _imu_zs = imu_zs;
}

/*
    Get 15s of IMU data at 10Hz. 
*/
void imu_sample()
{
    _imu_sample_pos = 0;

    Timer1.initialize(100000);
    Timer1.attachInterrupt(imu_measure);
    
    while (_imu_sample_pos < 150);
}

/*
    ISR routine.
*/
void imu_measure()
{
    sei();

    int16_t reading[3];
    imu_get(reading);
    
    _imu_xs[_imu_sample_pos] = reading[0];
    _imu_ys[_imu_sample_pos] = reading[1];
    _imu_zs[_imu_sample_pos] = reading[2];

    _imu_sample_pos++;

    if (_imu_sample_pos >= 150)
    {
        Timer1.detachInterrupt();
        Timer1.stop();
    }
}

/*
    Get an IMU reading. All values are 1e-3.
*/
void imu_get(int16_t* output)
{
    Wire.beginTransmission(IMU_ADDR);
    Wire.write(0x32);
    Wire.endTransmission();

    Wire.requestFrom(IMU_ADDR, 6);

    int16_t values[6] = {};
    uint8_t count = 0;

    while(Wire.available())
    { 
        values[count] = Wire.read();
        count++;
    }

    output[0] = (int16_t) (values[1]<<8 | values[0]) * 78; // x
    output[1] = (int16_t) (values[3]<<8 | values[2]) * 78; // y
    output[2] = (int16_t) (values[5]<<8 | values[4]) * 78; // z
}
