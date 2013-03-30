/**
 * counter.cpp
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

#include "counter.h"

uint16_t counter_get()
{
    byte lowByte = EEPROM.read(EEPROM_LOW_BYTE);
    byte highByte = EEPROM.read(EEPROM_HIGH_BYTE);
    return ((highByte << 8) | lowByte);
}

void counter_set(uint16_t new_counter)
{
    EEPROM.write(EEPROM_LOW_BYTE, (byte) (0xFF & new_counter));
    EEPROM.write(EEPROM_HIGH_BYTE, (byte) (new_counter >> 8));
}

void counter_inc()
{
    counter_set(counter_get() + 1);
}

void counter_reset()
{
    counter_set(0);
}

uint16_t imu_counter_get()
{
    byte lowByte = EEPROM.read(EEPROM_IMU_LOW_BYTE);
    byte highByte = EEPROM.read(EEPROM_IMU_HIGH_BYTE);
    return ((highByte << 8) | lowByte);
}

void imu_counter_set(uint16_t new_counter)
{
    EEPROM.write(EEPROM_IMU_LOW_BYTE, (byte) (0xFF & new_counter));
    EEPROM.write(EEPROM_IMU_HIGH_BYTE, (byte) (new_counter >> 8));
}

void imu_counter_inc()
{
    imu_counter_set(imu_counter_get() + 1);
}

void imu_counter_reset()
{
    imu_counter_set(0);
}
