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
#include "bootloader.h"


#include "lcd_control.h"
#include "timer.h"

#include "jsmn.h"
#include "json.h"
#include "usart.h"

#include <string.h>

#include "util.h"

void clock_init(){
  /* ErrorStatus HSEStartUpStatus; */

  RCC_DeInit();
  /* RCC system reset(for debug purpose) */
  /* RCC_HSEConfig(RCC_HSE_ON); */
  /* /\* Enable HSE *\/ */
  /* HSEStartUpStatus = RCC_WaitForHSEStartUp(); */
  /* /\* Wait till HSE is ready *\/ */

  /* if(HSEStartUpStatus == SUCCESS) */
  /*   { */
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
      RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);

      RCC_PLLCmd(ENABLE);
      /* Enable PLL */
      while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
      /* Wait till PLL is ready */

      RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
      /* Select PLL as system clock source */
      while(RCC_GetSYSCLKSource() != 0x08);
      /* Wait till PLL is used as system clock source */
    /* } */

      SystemCoreClock = 16*4000000;
      // 64 MHz is the highest it can go on HSI, don't know about
      // current but wake from sleep is faster at least
}


void Delay(volatile unsigned long delay) {
  for(; delay; --delay );
}

int main(int argc, char *argv[])
{
  //BOOTLOADER_reset();

  int r;
  __enable_irq();
  //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4000); // make sure that
  // interrupts work
  clock_init(); // hey, you can overclock later here. maybe.



  JSON_init();
  USART1_Init();
  TIM_init();

  LCD_Configuration();
  LCD_Initialization();
  LCD_Clear(LCD_Black);

  USART1_PutString("***** INIT DONE *****\n");

  while(1) {

    r = JSON_render();

    // todo: better error reporting
    if(r == 3) {
      // no free bufs
      continue;
    } else if(r != 0) {
      JSON_init(); // reset buffers in case of errors
      char abuf[32];
      itoa(abuf, r, 10);
      USART1_PutString(abuf);
      USART1_PutChar('N');
      USART1_PutChar('\n');
    }
    //USART1_PutChar('A');
    __asm__("WFI"); // sleep for a bit.
  }
  return 0;
}
