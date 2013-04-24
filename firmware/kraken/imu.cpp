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

// Enable/disable the IMU
bool IMU_EN = false;
bool imuEnabled(){
    return IMU_EN;
}
void setImuEnabled(bool a){
    IMU_EN = a;
}

// Send IMU data every X transmissions
uint8_t IMU_TRANSMISSIONS = 5;
uint8_t imuFrequency(){
    return IMU_TRANSMISSIONS;
}
void setImuFrequency(uint8_t i){
    IMU_TRANSMISSIONS = i;
}

/*
    Set up the IMU. This should be called in the main setup().
*/
void imu_setup(int16_t* imu_xs, int16_t* imu_ys, int16_t* imu_zs)
{
    Wire.begin();

    // Put the gyro to sleep as it is not used
    Wire.beginTransmission(GYRO_ADDR);
    Wire.write(0x3E);
    Wire.write(0x40);
    Wire.endTransmission();

    // Sleep mode
    Wire.beginTransmission(ACCEL_ADDR);
    Wire.write(0x2D);
    Wire.write(0x04);
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
    // Standby
    Wire.beginTransmission(ACCEL_ADDR);
    Wire.write(0x2D);
    Wire.write(0x00);
    Wire.endTransmission();

    // Measurement mode
    Wire.beginTransmission(ACCEL_ADDR);
    Wire.write(0x2D);
    Wire.write(0x08);
    Wire.endTransmission();

    // Set range
    Wire.beginTransmission(ACCEL_ADDR);
    Wire.write(0x31);
    Wire.write(0x01);
    Wire.endTransmission();

    _imu_sample_pos = 0;

    if (SERIAL_EN)
    {
        Serial.println("IMU: Sampling");
        delay(50);
    }

    Timer1.initialize(100000);
    Timer1.attachInterrupt(imu_measure);
    
    while (_imu_sample_pos < 50)
    {
        delay(100); 
    }

    // Standby
    Wire.beginTransmission(ACCEL_ADDR);
    Wire.write(0x2D);
    Wire.write(0x00);
    Wire.endTransmission();

    // Sleep mode
    Wire.beginTransmission(ACCEL_ADDR);
    Wire.write(0x2D);
    Wire.write(0x04);
    Wire.endTransmission();
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

    if (_imu_sample_pos >= 50)
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
    Wire.beginTransmission(ACCEL_ADDR);
    Wire.write(0x32);
    Wire.endTransmission();

    Wire.requestFrom(ACCEL_ADDR, 6);

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
