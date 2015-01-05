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

void delay() {
  u16 a,b,c;
  for(; a < 65000; a++) {
    for(; b < 65000; b++) {
      for(; c < 65000; c++) {
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
      }
    }
  }
}


int main(int argc, char *argv[])
{
  int cnt = 0;
  /* int max, maxi; */

  /* float fft[FFT_LEN]; */
  /* float avgfft[FFT_LEN]; */
  /* char buf[32]; */
  LCD_Configuration();
  LCD_Initialization();
  clock_init();
  /* LCD_Configuration(); */
  /* LCD_Initialization(); */
  /* ADC_Configuration(); */
  /* TIM_init(); */
  /* USART12_Init(); */
  /* ESP8266_init(); */

  LCD_Clear(LCD_Black);
  while(1) {
    LCD_DrawLine(cnt%240,cnt%240,0,cnt%320,LCD_Black);
    LCD_DrawLine(0,cnt%240,cnt%320,cnt%320,LCD_Black);
    LCD_DrawLine(0,cnt%240,0,cnt%320,LCD_Black);
    LCD_DrawString(cnt%240,cnt%320,"Hello World!\0",LCD_Black,0,0);
    cnt++;

    //LCD_FillRect(72,0,240,320,cnt);
    LCD_DrawLine(cnt%240,cnt%240,0,cnt%320,LCD_Yellow);
    LCD_DrawLine(0,cnt%240,cnt%320,cnt%320,LCD_Yellow);
    LCD_DrawLine(0,cnt%240,0,cnt%320,LCD_Yellow);
    LCD_DrawString(cnt%240,cnt%320,"Hello World!\0",LCD_Yellow,0,0);
  }
}
