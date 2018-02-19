#ifndef USART_LIB_H_
#define USART_LIB_H_

#include "stm32f10x.h"
#include <string.h>
#include <stdio.h>

// =================== KONFIGURACJA ====================

//Rozmiar bufora wejsciowego i wyjsciowego
#define RX_BUF_SIZE 128
#define TX_BUF_SIZE 128
#define CMD_SIZE 50
#define END_TRANSMISSION_SIGN '$'

#define RX_BUF_MASK (RX_BUF_SIZE-1)
#define TX_BUF_MASK (TX_BUF_SIZE-1)

extern void send_char(char c);
extern void send_string(const char* s);
extern int __io_putchar(int c);
extern char buf_getchar();
extern void buf_getcmd(char tab[]);
extern void USART2_IRQHandler();
extern void USART_SendChar(USART_TypeDef* USARTx, uint8_t znak);
extern void USART_Puts(USART_TypeDef* USARTx, char komenda[] );







#endif /* USART_LIB_H_ */


