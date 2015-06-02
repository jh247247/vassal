#ifndef TIMER_H
#define TIMER_H

#define sysTicksPerMillisecond 440
extern volatile unsigned int g_sysTick;

int TIM_init();
unsigned int TIM_getSysTick();

#endif /* TIMER_H */
