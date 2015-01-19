#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_flash.h"
#include "esp8266.h"
#include "timer.h"

#include "usart.h"

void USART1_Init(void)
{
  /* USART configuration structure for USART1 */
  USART_InitTypeDef usart1_init_struct;
  /* Bit configuration structure for GPIOA PIN9 and PIN10 */
  GPIO_InitTypeDef gpio_init_struct;

  /* Enalbe clock for USART1, AFIO and GPIOA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 |
                         RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA,
                         ENABLE);
  //RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);


  /* Set the usart output to the remapped pins */
  //GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

  /* GPIOB PIN6 alternative function Tx */
  gpio_init_struct.GPIO_Pin = GPIO_Pin_9;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &gpio_init_struct);
  /* GPIOB PIN7 alternative function Rx */
  gpio_init_struct.GPIO_Pin = GPIO_Pin_10;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &gpio_init_struct);

  /* Baud rate 9600, 8-bit data, One stop bit
   * No parity, Do both Rx and Tx, No HW flow control
   */
  usart1_init_struct.USART_BaudRate = 9600;
  usart1_init_struct.USART_WordLength = USART_WordLength_8b;
  usart1_init_struct.USART_StopBits = USART_StopBits_1;
  usart1_init_struct.USART_Parity = USART_Parity_No ;
  usart1_init_struct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  usart1_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  /* Configure USART1 */
  USART_Init(USART1, &usart1_init_struct);
  /* Enable RXNE interrupt */
  //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  /* Enable USART1 global interrupt */
  //NVIC_EnableIRQ(USART1_IRQn);

  /* Enable USART1 */
  USART_Cmd(USART1, ENABLE);
}


void USART1_PutChar(char ch)
{
  while(!(USART1->SR & USART_SR_TXE));
  USART1->DR = ch;
}

void USART1_PutString(char * str)
{
  while(*str != 0)
    {
      USART1_PutChar(*str);
      str++;
    }
  USART1_PutChar('\0');
}
