/*
 * encoder.c
 *
 *  Created on: 6 Feb 2018
 *      Author: barte
 */

#include "stm32f10x.h"
#include "encoder.h"
#include <stdio.h>
#include "delay.h"

extern uint8_t I2CResult;
extern void i2c_initialize();

uint8_t I2C_Wait_Condition(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT){			//zwraca 1, jesli wystapil timeout
	TIM_Cmd(TIM3, ENABLE);
	while ((I2C_CheckEvent(I2Cx, I2C_EVENT) != SUCCESS) && I2CWatchDog == 0);

	//printf("Czas operacji: %d ms\n", TIM_GetCounter(TIM3));
	TIM_Cmd(TIM3, DISABLE);
	TIM_SetCounter(TIM3, 0);
	if(I2CWatchDog == 1){
		I2CWatchDog = 0;
		return 1;
	}
	else{
		return 0;
	}
}

void I2CSetReg(uint8_t device_addr , uint8_t reg)
{
	I2C_GenerateSTART(I2C1, ENABLE);
	I2CResult |= I2C_Wait_Condition(I2C1, I2C_EVENT_MASTER_MODE_SELECT);
	I2C_Send7bitAddress(I2C1, device_addr, I2C_Direction_Transmitter);
	I2CResult |= I2C_Wait_Condition(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
	I2C_SendData(I2C1, reg);
	I2CResult |= I2C_Wait_Condition(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING);
}

void I2CWrite(uint8_t device_addr, uint8_t reg, const void* data, int size)
{
	int i;
	const uint8_t* buffer = (uint8_t*)data;

	I2CSetReg(device_addr, reg);
	for (i = 0; i < size; i++) {
		I2C_SendData(I2C1, buffer[i]);
		I2CResult |= I2C_Wait_Condition(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING);
	}
	I2C_GenerateSTOP(I2C1, ENABLE);
}

void I2CRead(uint8_t device_addr, uint8_t reg, void* data, int size)
{
	int i;
	uint8_t* buffer = (uint8_t*)data;
	I2CSetReg(device_addr, reg);
	I2C_GenerateSTART(I2C1, ENABLE);
	I2CResult |= I2C_Wait_Condition(I2C1, I2C_EVENT_MASTER_MODE_SELECT);
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	I2C_Send7bitAddress(I2C1, device_addr, I2C_Direction_Receiver);
	I2CResult |= I2C_Wait_Condition(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);
	for (i = 0; i < size - 1; i++) {
		I2CResult |= I2C_Wait_Condition(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED);
		buffer[i] = I2C_ReceiveData(I2C1);
	}
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	I2C_GenerateSTOP(I2C1, ENABLE);
	I2CResult |= I2C_Wait_Condition(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED);
	buffer[i] = I2C_ReceiveData(I2C1);
}

void I2CWriteReg(uint8_t device_addr, uint8_t reg, uint8_t value)
{
	I2CWrite(device_addr, reg, &value, sizeof(value));
}

uint8_t I2CReadReg(uint8_t device_addr, uint8_t reg)
{
	uint8_t value = 0;
	I2CRead(device_addr, reg, &value, sizeof(value));
	return value;
}

int16_t I2CReadValue(uint8_t device_addr, uint8_t reg)
{
	int16_t value = 0;
	I2CRead(device_addr, reg, &value, sizeof(value));
	return value;
}

void resetI2C(){
	I2C_Cmd(I2C1, DISABLE);

	GPIO_InitTypeDef gpio;
	gpio.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7; // SCL, SDA
	gpio.GPIO_Mode = GPIO_Mode_Out_OD;
	gpio.GPIO_Speed = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &gpio);

	delay_ms(100);

	gpio.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7; // SCL, SDA
	gpio.GPIO_Mode = GPIO_Mode_AF_OD;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio);


	I2C_SoftwareResetCmd(I2C1, ENABLE);
	I2C_SoftwareResetCmd(I2C1, DISABLE);

	i2c_initialize();
}

uint8_t AS_readEncoder(float *angleValue){
	uint8_t res = 0, poprawny = 0;
	uint8_t result[4];
	uint8_t temp_result;
	uint16_t angle = 0;

	temp_result = I2CResult;	//zachowanie wartosci I2CResult, zeby przywrocic ja na koniec funkcji
	I2CResult = 0;

	res = I2CReadReg(AS5600_ADDR, 0x0B);
	if (I2CResult == 0){
		if(res & 0x10){	//magnes za daleko
			I2CResult = temp_result;
			poprawny = 2;
			return poprawny;
		}
		if(res & 0x08){ //magnes za blisko
			I2CResult = temp_result;
			poprawny = 3;
			return poprawny;	//mozna w 1 linijce?
		}
		if(res & 0x20){	//wykrycie magnesu
			I2CRead(AS5600_ADDR, 0x0C, &result, 4);
			if(I2CResult == 0){
				angle |= (uint16_t)result[2] << 8;
				angle |= (uint16_t)result[3];
				*angleValue = (360.0/4096.0)*(float)angle;
				I2CResult = temp_result;
				poprawny = 1;
				return poprawny;
			}
			else{
				poprawny = 0;
				return poprawny;
			}
		}
	}
	else{		//odczyt nieudany
		I2CResult = temp_result;
		poprawny = 0;
		return poprawny;
	}
	return 0;
}
