/**
 ******************************************************************************
 * @file    main.c
 * @author  Ac6
 * @version V1.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
 */


#include "stm32f10x.h"
#include "delay.h"
#include "USART_lib.h"
#include "encoder.h"

volatile char I2CWatchDog = 0;
uint8_t I2CResult = 0;
uint8_t I2CErrorCount = 0;


void TIM3_IRQHandler()
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		I2CWatchDog = 1;
	}
}

void usart_initialize(){			//ENABLE UART i przerwania
	USART_InitTypeDef uart;
	USART_StructInit(&uart);
	uart.USART_BaudRate = 115200;
	USART_Init(USART2, &uart);
	USART_Cmd(USART2, ENABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void nvic_initialize(){
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = USART2_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 1;
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	nvic.NVIC_IRQChannel = TIM3_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
}

void gpio_initialize(){
	GPIO_InitTypeDef gpio;

	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_2;			//TX
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &gpio);

	gpio.GPIO_Pin = GPIO_Pin_3;			//RX
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpio);

	gpio.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7; // SCL, SDA
	gpio.GPIO_Mode = GPIO_Mode_AF_OD;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio);
}
void i2c_initialize(){
	I2C_InitTypeDef i2c;
	I2C_StructInit(&i2c);
	i2c.I2C_Mode = I2C_Mode_I2C;
	i2c.I2C_ClockSpeed = 100000;
	I2C_Init(I2C1, &i2c);
	I2C_Cmd(I2C1, ENABLE);
}

void tim3_ititialize(){					//timeout I2C
	TIM_TimeBaseInitTypeDef tim;
	TIM_TimeBaseStructInit(&tim);
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	tim.TIM_Prescaler = 64000 - 1;
	tim.TIM_Period = 50;
	TIM_TimeBaseInit(TIM3, &tim);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}

void STM_StartUp(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	gpio_initialize();
	usart_initialize();
	nvic_initialize();
	i2c_initialize();
	tim3_ititialize();

	SysTick_Config(SystemCoreClock / 1000);
}

int main(void)
{

	STM_StartUp();

	float angleValue = 0;
	uint8_t result = 0;

	while(1){
		delay_ms(100);

		result = AS_readEncoder(&angleValue);
		switch(result){
		case 0:
			printf("Odczyt nieudany!\n");
			break;
		case 1:
			printf("%f\n", angleValue);
			break;
		case 2:
			printf("Magnes za daleko!\n");
			break;
		case 3:
			printf("Magnes za blisko!\n");
			break;
		}

		if(I2CResult == 1){
			printf("Wystapily bledy timeout I2C!\n");
			I2CErrorCount += 1;
			I2CResult = 0;
		}
		else{
			I2CErrorCount = 0;
		}
		if(I2CErrorCount >= 10){
			I2CErrorCount = 0;
			resetI2C();
			printf("Wykonano reset I2C!\n\n\n\n\n\n\n\n\n");
		}
	}
}



