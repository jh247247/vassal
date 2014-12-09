#ifndef TIMER_H
#define TIMER_H

#define sysTicksPerMillisecond 439
extern volatile unsigned int g_adcFlag;
extern volatile unsigned int g_sysTick;

int TIM_init();

#endif /* TIMER_H */
