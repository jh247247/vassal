/*
 * A quick test program to test the LCD on the dev board mentioned in
 * the readme. Also blinks an LED and does some floating point calcs for
 * fun and testing :D.
 *
 * If this helps you do something cool let me know!
 */

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_flash.h"
#include "misc.h"
#include "math.h"


#include "lcd_control.h"
#include "timer.h"

#include "jsmn.h"
#include "json.h"
#include "usart.h"

#include <string.h>

void clock_init(){
  ErrorStatus HSEStartUpStatus;

  RCC_DeInit();
  /* RCC system reset(for debug purpose) */
  RCC_HSEConfig(RCC_HSE_ON);
  /* Enable HSE */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();
  /* Wait till HSE is ready */

  if(HSEStartUpStatus == SUCCESS)
    {
      FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
      /* Enable Prefetch Buffer */
      FLASH_SetLatency(FLASH_Latency_2);
      /* Flash 2 wait state */
      RCC_HCLKConfig(RCC_SYSCLK_Div1);
      /* HCLK = SYSCLK */
      RCC_PCLK2Config(RCC_HCLK_Div1);
      /* PCLK2 = HCLK */
      RCC_PCLK1Config(RCC_HCLK_Div2);
      /* PCLK1 = HCLK/2 */
      RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

      RCC_PLLCmd(ENABLE);
      /* Enable PLL */
      while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
      /* Wait till PLL is ready */

      RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
      /* Select PLL as system clock source */
      while(RCC_GetSYSCLKSource() != 0x08);
      /* Wait till PLL is used as system clock source */
    }
}

extern char g_jsonInBuf[8192*2];
extern unsigned int g_jsonLen;

int main(int argc, char *argv[])
{
  char* ptr;
  int r;
  LCD_Configuration();
  LCD_Initialization();
  clock_init();

  TIM_init();
  USART1_Init();

  JSON_init();
  LCD_Clear(LCD_Black);
  ptr = g_jsonInBuf;

  USART1_PutString("***** INIT DONE *****\n");

  while(1) {
    //while(!TIM_GetITStatus(TIM2,TIM_IT_Update) != RESET);
    //TIM_ClearITPendingBit(TIM2,TIM_IT_Update);


    if(USART_GetFlagStatus(USART1, USART_IT_RXNE) != RESET) {
      *(ptr+g_jsonLen) = USART_ReceiveData(USART1);

      // make sure that the start of the packet is an object brace.
      if(g_jsonLen == 0 &&
         *(ptr+g_jsonLen) != '{') {
        continue;
      }

      // check for end of string
      if(*(ptr+g_jsonLen) == '\0') {
        g_jsonLen++; // have to include null char
        r = JSON_render();

	// todo: better error reporting
        if(r == 1) {
          USART1_PutString("Error: 1!\n");
        } else if(r == 2){
          USART1_PutString("Error: 2!\n");
        } else if(r == -1){
          USART1_PutString("Error: JSMN_NOMEM!\n");
        } else if(r == -2){
          USART1_PutString("Error: JSMN_INVAL!\n");
        } else if(r == -3){
          USART1_PutString("Error: JSMN_ERROR_PART!\n");
        } else if(r != 0){
	  USART1_PutString("Error: GENERAL\n");
	}
        g_jsonLen = 0; // reset for next command
	USART1_PutString("\nDone!\n");
      } else {
        g_jsonLen++;
      }



    }
  }
}
