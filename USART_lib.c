#include "USART_lib.h"
#include "stm32f10x.h"

char bufor_rx[RX_BUF_SIZE];
int rx_beg = 0;
int rx_end = 0;
char bufor_tx[TX_BUF_SIZE];
int tx_beg = 0;
int tx_end = 0;

void send_char(char c)			//wyslanie znaku przez UART
{
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	USART_SendData(USART2, c);
}

void send_string(const char* s)	//wyslanie stringu przez UART (bez zmiennych)
{
	while (*s){
		if (*s=='\n'){
			send_char('\r');
		}
		send_char(*s++);
	}
}

int __io_putchar(int c)			//wyslanie znaku zgodne z printf
{
	if (c=='\n')
		send_char('\r');
	send_char(c);
	return c;
}

char buf_getchar(){
	char c;
	if(rx_beg == rx_end){
		return 0;
	}
	rx_end = (rx_end + 1) & RX_BUF_MASK;
	c = bufor_rx[rx_end];
	//printf("numer %i znak %c", rx_end, c);
	return c;
}

void buf_getcmd(char tab[]){
	char tempch = 0;
	int i = 0;
	while(1){
		tempch = buf_getchar();
		if(tempch && tempch != END_TRANSMISSION_SIGN){
			//printf("Odczytano znak %c z pozycji %i\n" , tempch, rx_end);
			tab[i] = tempch;
			i++;
		}
		else{
			break;
		}
	}
}

void USART2_IRQHandler(){
	if( USART_GetITStatus(USART2, USART_IT_TXE) == SET )
	{
		USART_ClearITPendingBit(USART2, USART_IT_TXE);

		if( tx_beg != tx_end ){
			tx_end = (tx_end + 1) & TX_BUF_MASK;
			USART2->DR = bufor_tx[tx_end];
		}
		else
		{
			USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		}
	}

	if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET){
		register uint8_t data;
		register uint8_t temp_head;
		temp_head = (rx_beg + 1) & RX_BUF_MASK;
		data = (uint8_t)(USART2->DR);
		if( temp_head == rx_end){
			rx_beg = rx_end;
		}
		else{
			rx_beg = temp_head;
			bufor_rx[temp_head] = data;
			//printf("numer %i znak %c\n" , rx_beg, data);
		}
	}
}

void USART_SendChar(USART_TypeDef* USARTx, uint8_t znak)
{

    uint8_t tmp_head;
    tmp_head = (tx_beg + 1) & TX_BUF_MASK;
    while (tmp_head == tx_end) {}
    bufor_tx[tmp_head] = znak;
    tx_beg = tmp_head;

    USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
}

void USART_Puts( USART_TypeDef* USARTx, char data[] )
{
    register uint8_t c;
    int i=0;
    while(1){
    	c = data[i];
    	if(c){
    		if (c=='\n'){
    			USART_SendChar(USARTx, '\r');
    		}
    		USART_SendChar( USARTx, c );
    		i++;
    	}
    	else{
    		break;
    	}
    }
}
