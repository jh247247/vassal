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

void USART12_Init(void)
{
  /* USART configuration structure for USART1 */
  USART_InitTypeDef usart1_init_struct;
  /* Bit configuration structure for GPIOA PIN9 and PIN10 */
  GPIO_InitTypeDef gpio_init_struct;

  /* Enalbe clock for USART1, AFIO and GPIOA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO |
                         RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA,
                         ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);


  /* Set the usart output to the remapped pins */
  GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

  /* GPIOB PIN6 alternative function Tx */
  gpio_init_struct.GPIO_Pin = GPIO_Pin_6;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &gpio_init_struct);
  /* GPIOB PIN7 alternative function Rx */
  gpio_init_struct.GPIO_Pin = GPIO_Pin_7;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &gpio_init_struct);

  /* GPIOA PIN9 alternative function Tx */
  gpio_init_struct.GPIO_Pin = GPIO_Pin_2;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &gpio_init_struct);
  /* GPIOA PIN9 alternative function Rx */
  gpio_init_struct.GPIO_Pin = GPIO_Pin_3;
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

  USART_Init(USART2, &usart1_init_struct);
  USART_Cmd(USART2, ENABLE);
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
}

void USART2_PutChar(char ch)
{
  while(!(USART2->SR & USART_SR_TXE));
  USART2->DR = ch;
}

void USART2_PutString(char * str)
{
  while(*str != 0)
    {
      USART2_PutChar(*str);
      str++;
    }
}

unsigned char USART_waitForString(USART_TypeDef* USARTx, char* ref, int timeout) {
  int state = 0;
  char c = 0;
  g_sysTick = 0;

  while(g_sysTick < timeout) {
    if(USART_GetFlagStatus(USART1, USART_IT_RXNE) != RESET) {
      c = USART_ReceiveData(USART1);
      if(c == ref[state]) {
        // found next char, continue
        state++;
      } else if(ref[state] == 0) {
        // got to end of string!
        // return success
        return 1;
      } else {
        // char does not match, continue
        state = 0;
      }
    }
  }
  return 0;
}
