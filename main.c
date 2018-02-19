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
	i2c.I2C_ClockSpeed = 10000;
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
	uint8_t res = 0;
	uint16_t raw_angle = 0;
	uint16_t angle = 0;
	uint8_t result[4] = {0};
	int i = 0;
	char poprawny = 0;
	//printf("Start!\n");
	while(1){
		delay_ms(100);
		printf("Rozpoczynam petle : %d\n" , i++);
		//printf("dzialam! \n");
		//delay_ms(100);
		//printf("zapytanie do imu! \n");
		uint8_t who_am_i = I2CReadReg(IMU_ADDR, 0x75);
		if (who_am_i == 0x71) {
			printf("Akcelerometr OK!\n");
		}
		else {
			//printf("Niepoprawna odpowiedz ukladu(0x%02X)\n", who_am_i);
		}



		//printf("Petla %d" , i++);



		//delay_ms(1000);

		poprawny = 0;
		raw_angle = 0;
		angle = 0;
		res = 0;

		res = I2CReadReg(AS5600_ADDR, 0x0B);

		//printf("Stan enkodera: 0x%02X \n", res);
		if(res & 0x20){
			//printf("Wykryto magnes \n");
			poprawny = 2;
		}
		if(res & 0x10){
			//printf("Magnes za daleko \n");
			poprawny = 1;
		}
		if(res & 0x08){
			//printf("Magnes za blisko \n");
			poprawny = 1;
		}



		if(poprawny == 2){
			printf("Magnes OK!\n");
			I2CRead(AS5600_ADDR, 0x0C, &result, 4);

			//result = I2CReadReg(AS5600_ADDR, 0x0C);
			//printf("Rejestr 0x0C: 0x%02X ; Po konwersji: 0x%04X \n", result[0], (uint16_t)result[0] << 8);
			raw_angle |= (uint16_t)result[0] << 8;

			//result[] = I2CReadReg(AS5600_ADDR, 0x0D);
			//printf("Rejestr 0x0D: 0x%02X Po konwersji: 0x%04X \n", result[1], (uint16_t)result[1]);
			raw_angle |= (uint16_t)result[1];

			//printf("Wartosc raw angle: 0x%04X \nDziesietnie: %f\n", raw_angle, (360.0/4096.0)*(float)raw_angle);
			//printf("Wartosc raw angle: %f\n", (360.0/4096.0)*(float)raw_angle);

			//result[] = I2CReadReg(AS5600_ADDR, 0x0E);
			//printf("Rejestr 0x0E: 0x%02X ; Po konwersji: 0x%04X \n", result[2], (uint16_t)result[2] << 8);
			angle |= (uint16_t)result[2] << 8;

			//result[] = I2CReadReg(AS5600_ADDR, 0x0F);
			//printf("Rejestr 0x0F: 0x%02X Po konwersji: 0x%04X \n", result[3], (uint16_t)result[3]);
			angle |= (uint16_t)result[3];

			//printf("Wartosc angle: 0x%04X \nDziesietnie: %f\n", angle, (360.0/4096.0)*(float)angle);
			printf("%f\n", (360.0/4096.0)*(float)angle);
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
			printf("Wykonano reset I2C!\n\n\n\n\n\n\n\\n\n");
		}
	}
}


