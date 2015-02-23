#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_flash.h"
#include "misc.h"

#include "timer.h"
#include "usart.h"

volatile unsigned int g_sysTick;

int TIM_init(){
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  TIM_TimeBaseInitTypeDef timerInitStructure;
  timerInitStructure.TIM_Prescaler = 12000; // aim for 60 fps
  timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
  timerInitStructure.TIM_Period = 100;
  timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  timerInitStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &timerInitStructure);
  TIM_Cmd(TIM2, ENABLE);
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

  NVIC_InitTypeDef n;
  n.NVIC_IRQChannel = TIM2_IRQn;
  n.NVIC_IRQChannelPreemptionPriority = 0;
  n.NVIC_IRQChannelSubPriority = 1;
  n.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&n);

  g_sysTick = 0;

  return 0;
}

void TIM2_IRQHandler()
{
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
    g_sysTick++;
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  }
}

unsigned int TIM_getSysTick() {
  return g_sysTick;
}
