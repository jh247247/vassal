#ifndef USART_H
#define USART_H
#include "stm32f10x_usart.h"

#define USART_HAS_DATA(x) (USART_GetITStatus(x, USART_IT_RXNE) != RESET)

void USART1_Init(void);
void USART1_PutChar(char ch);
void USART1_PutString(const char * str);
unsigned char USART_waitForString(USART_TypeDef* USARTx, char* ref, int timeout);

#endif /* USART_H */
