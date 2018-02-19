/*
 * encoder.h
 *
 *  Created on: 6 Feb 2018
 *      Author: barte
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#define AS5600_ADDR	0x36<<1
#define IMU_ADDR 0x68<<1

extern volatile char I2CWatchDog;

extern void I2CWrite(uint8_t device_addr, uint8_t reg, const void* data, int size);
extern void I2CRead(uint8_t device_addr, uint8_t reg, void* data, int size);

extern void I2CWriteReg(uint8_t device_addr, uint8_t reg, uint8_t value);
extern uint8_t I2CReadReg(uint8_t device_addr, uint8_t reg);
extern int16_t I2CReadValue(uint8_t device_addr, uint8_t reg);
extern void resetI2C();

#endif /* ENCODER_H_ */
