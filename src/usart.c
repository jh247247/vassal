#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_flash.h"
#include "misc.h"
#include "json.h"
#include "timer.h"

#include "usart.h"

#include "string.h"

// good enough for 2 ports.
#define USART2PORT(x) (x==BLUETOOTH_USART)

// rx buffer so we don't really need timeouts anymore.
//volatile char g_usart_rx[USART_AMOUNT][USART_BUF_LEN];
//volatile int g_usart_rx_index[USART_AMOUNT];

// match stuff so we can be on the lookout for specific strings.
char g_usart_rx_match[USART_AMOUNT][USART_BUF_LEN]; // store the string we are
// searching for
volatile int g_usart_rx_match_index[USART_AMOUNT]; // keep the current string we
// are looking for
int g_usart_rx_match_length[USART_AMOUNT]; // total length of the string


/* TODO:  Baud rate changes on the fly. */

void USART_Config(void)
{

  /* Bit configuration structure for GPIOA PIN9 and PIN10 */
  GPIO_InitTypeDef gpio_init_struct;

  /* Enalbe clock for USART1, AFIO and GPIOA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA |
			 RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,
                         ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);


  /* Set the usart output to the remapped pins */
  //GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

  /* GPIOA PIN2 alternative function Tx */
  gpio_init_struct.GPIO_Pin = GPIO_Pin_10;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &gpio_init_struct);
  /* GPIOA PIN3 alternative function Rx */
  gpio_init_struct.GPIO_Pin = GPIO_Pin_11;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &gpio_init_struct);

  /* GPIOA PIN9 alternative function Tx */
  gpio_init_struct.GPIO_Pin = GPIO_Pin_9;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &gpio_init_struct);
  /* GPIOA PIN10 alternative function Rx */
  gpio_init_struct.GPIO_Pin = GPIO_Pin_10;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &gpio_init_struct);

  /* Setup host usart */
  USART_SetBaudRate(HOST_USART, 115200);
  /* Enable RXNE interrupt */
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  /* Enable USART1 RXNE interrupt */
  NVIC_InitTypeDef n;
  n.NVIC_IRQChannel = USART1_IRQn;
  n.NVIC_IRQChannelPreemptionPriority = 0;
  n.NVIC_IRQChannelSubPriority = 1;
  n.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&n);



  // Setup bluetooth usart
  // hm-10 expects this.

  USART_SetBaudRate(BLUETOOTH_USART, 19200);
  /* Enable RXNE interrupt */
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  n.NVIC_IRQChannel = USART2_IRQn;
  NVIC_Init(&n);

}


void USART_SetBaudRate(USART_TypeDef* USARTx, int baud) {
  /* USART configuration structure for USART */
  USART_InitTypeDef usart_init;

  usart_init.USART_BaudRate = baud;
  usart_init.USART_WordLength = USART_WordLength_8b;
  usart_init.USART_StopBits = USART_StopBits_1;
  usart_init.USART_Parity = USART_Parity_No ;
  usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  /* Configure USART */
  USART_Init(USARTx, &usart_init);
  /* Enable USART*/
  USART_Cmd(USARTx, ENABLE);

}

void USART_setMatch(USART_TypeDef* USARTx, char* str) {
  int port = USART2PORT(USARTx);
  strcpy(g_usart_rx_match[port], str);
  g_usart_rx_match_length[port] = strlen(str);
  g_usart_rx_match_index[port] = 0; // reset index so that we restart
  // the state machine
}

// reset matches index
void USART_resetMatch(USART_TypeDef* USARTx) {
  g_usart_rx_match_index[USART2PORT(USARTx)] = 0;
}

int USART_checkMatch(USART_TypeDef* USARTx) {
  int port = USART2PORT(USARTx);
  return g_usart_rx_match_index[port] - g_usart_rx_match_length[port];
}

/* void USART_resetRXBuffer(USART_TypeDef* USARTx) { */
/*   int port = USART2PORT(USARTx); */
/*   memset((char*)g_usart_rx[port], 0, USART_BUF_LEN); */
/*   g_usart_rx_index[port] = 0; */
/* } */

void USART_PutChar(USART_TypeDef* USARTx, char ch)
{
  while(!(USARTx->SR & USART_SR_TXE));
  USARTx->DR = ch;
}

void USART_PutString(USART_TypeDef* USARTx, char * str)
{
  while(*str != 0)
    {
      USART_PutChar(USARTx, *str);
      str++;
    }
}


unsigned char USART_waitForString(USART_TypeDef* USARTx, char* ref, int timeout) {
  g_sysTick = 0;
  USART_setMatch(USARTx, ref);
  while(g_sysTick < timeout) {
    if(USART_checkMatch(USARTx) == 0) {
      return 1;
    }
  }
  return 0;
}



void USART_rxCheck(int usart, char rx) {
  /* // add to the buffer */
  /* if(g_usart_rx_index[usart] < USART_BUF_LEN) { */
  /*   g_usart_rx[usart][g_usart_rx_index[usart]] = rx; */
  /*   g_usart_rx_index[usart]++; */
  /* } */
  
  USART_PutChar(HOST_USART, rx);
  JSON_appendToBuf(rx);
  
  if(g_usart_rx_match[usart][g_usart_rx_match_index[usart]] == rx) {
    g_usart_rx_match_index[usart]++;
  }

}

void USART1_IRQHandler()
{
  if(USART_GetFlagStatus(USART1, USART_IT_RXNE) != RESET) {
    char c = USART_ReceiveData(USART1);
    USART_rxCheck(0, c);
    USART_PutChar(BLUETOOTH_USART, c);
  }
}

void USART3_IRQHandler()
{
  if(USART_GetFlagStatus(USART3, USART_IT_RXNE) != RESET) {
    USART_rxCheck(1, USART_ReceiveData(USART3));
  }
}

