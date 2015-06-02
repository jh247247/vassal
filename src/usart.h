#ifndef USART_H
#define USART_H
#include "stm32f10x_usart.h"

#define USART_HAS_DATA(x) (USART_GetITStatus(x, USART_IT_RXNE) != RESET)

#define USART_BUF_LEN 50
#define USART_AMOUNT 2

#define BLUETOOTH_USART USART3
#define HOST_USART USART1


void USART_Config(void);
void USART_SetBaudRate(USART_TypeDef* USARTx, int baud);
void USART_PutChar(USART_TypeDef* USARTx, char ch);
void USART_PutString(USART_TypeDef* USARTx, char * str);
unsigned char USART_waitForString(USART_TypeDef* USARTx, char* ref,
				  int timeout);
void USART_setMatch(USART_TypeDef* USARTx, char* str);
void USART_resetMatch(USART_TypeDef* USARTx);
void USART_resetRXBuffer(USART_TypeDef* USARTx);
int USART_checkMatch(USART_TypeDef* USARTx);

#endif /* USART_H */
