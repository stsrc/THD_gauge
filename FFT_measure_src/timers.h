#ifndef _TIMERS_H_
#define _TIMERS_H_
#include <stm32f1xx.h>
#include <core_cm3.h>
void TIM6_init(uint16_t data_cnt, uint16_t *data_ptr);
void TIM3_init();
void TIM2_init();
void NVIC_prioritySet();
#endif
