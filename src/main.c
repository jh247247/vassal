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
#include "misc.h"
#include "lcd_control.h"
#include "rfft.h"
#include "math.h"

int TIM_init(){
  // I want to set this up for a sampling frequency of around 44KHz
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  TIM_TimeBaseInitTypeDef timerInitStructure;
  timerInitStructure.TIM_Prescaler = 100;
  timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
  timerInitStructure.TIM_Period = 1;
  timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  timerInitStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &timerInitStructure);
  TIM_Cmd(TIM2, ENABLE);
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

  // enable interrupts for timer
  NVIC_InitTypeDef nvicStructure;
  nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
  nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
  nvicStructure.NVIC_IRQChannelSubPriority = 1;
  nvicStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvicStructure);

  return 0;
}

void ADC_Configuration(void)
{
  ADC_InitTypeDef  ADC_InitStructure;
  /* PCLK2 is the APB2 clock */
  /* ADCCLK = PCLK2/6 = 72/6 = 12MHz*/
  RCC_ADCCLKConfig(RCC_PCLK2_Div6);

  /* Enable ADC1 clock so that we can talk to it */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  /* Put everything back to power-on defaults */
  ADC_DeInit(ADC1);

  /* ADC1 Configuration ------------------------------------------------------*/
  /* ADC1 and ADC2 operate independently */
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  /* Disable the scan conversion so we do one at a time */
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  /* Don't do contimuous conversions - do them on demand */
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  /* Start conversin by software, not an external trigger */
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  /* Conversions are 12 bit - put them in the lower 12 bits of the result */
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  /* Say how many channels would be used by the sequencer */
  ADC_InitStructure.ADC_NbrOfChannel = 1;

  /* Now do the setup */
  ADC_Init(ADC1, &ADC_InitStructure);
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibaration register */
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));
  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));

  GPIO_InitTypeDef GPIO_InitStructure;

  /* GPIOA Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

  /* Configure PC12 to mode: slow rise-time, pushpull output */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOB,&GPIO_InitStructure);//GPIOA init
}


unsigned int readADC1(unsigned char channel)
{
  ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_1Cycles5);
  // Start the conversion
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  // Wait until conversion completion
  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
  // Get the conversion value
  return ADC_GetConversionValue(ADC1);
}

volatile unsigned int g_adcFlag;

void TIM2_IRQHandler()
{
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
      TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
      g_adcFlag = 1;
    }
}

void drawLine(int x, int y1,int y2) {
  int i;
  if(y1>240)y1=240;
  for(i = y1; i < y2; i++) {
    LCD_SetPoint(x,i,0xF800);
  }
}

void USART1_Init(void)
{
  /* USART configuration structure for USART1 */
  USART_InitTypeDef usart1_init_struct;
  /* Bit configuration structure for GPIOA PIN9 and PIN10 */
  GPIO_InitTypeDef gpio_init_struct;

  /* Enalbe clock for USART1, AFIO and GPIOA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO |
                         RCC_APB2Periph_GPIOB, ENABLE);

  /* Set the usart output to the remapped pins */
  GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

  /* GPIOA PIN9 alternative function Tx */
  gpio_init_struct.GPIO_Pin = GPIO_Pin_6;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &gpio_init_struct);
  /* GPIOA PIN9 alternative function Rx */
  gpio_init_struct.GPIO_Pin = GPIO_Pin_7;
  gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &gpio_init_struct);


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


void USART_PutChar(char ch)
{
  while(!(USART1->SR & USART_SR_TXE));
  USART1->DR = ch;
}

void USART_PutString(char * str)
{
  while(*str != 0)
    {
      USART_PutChar(*str);
      str++;
    }
}

// has to be a power of 2, otherwise you get a hang.
#define FFT_LEN 512

#define AVG 5

int main(int argc, char *argv[])
{
  int i = 0;
  int cnt = 0;
  float fft[FFT_LEN];
  float avgfft[FFT_LEN];
  LCD_Configuration();
  LCD_Initialization();
  ADC_Configuration();
  TIM_init();
  USART1_Init();

  while(1)
    {

      if(g_adcFlag == 1) {
        fft[i++] = readADC1(ADC_Channel_8);
        g_adcFlag = 0;
      }

      // FFT is full, render the screen.
      if(i == FFT_LEN) {
        TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);


        for(i=0;i< FFT_LEN; i++) {
          fft[i]/=256;
        }

        rfft(fft,FFT_LEN);
        for(i = 0; i < FFT_LEN/4; i++) {
          avgfft[i] += fft[i]/AVG;
        }
        cnt++;
        i = 0;
        TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
      }

      if(cnt == AVG) {
	USART_PutString("AT\r\n");

        TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
        LCD_Clear(0);
        for(i = 0; i < FFT_LEN/4; i++) {
          drawLine(i,0,fft[i]);
        }
        TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
        i = 0;
        cnt = 0;
      }


    }
}
