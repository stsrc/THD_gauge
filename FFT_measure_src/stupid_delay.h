#ifndef _STUPID_DELAY_H_
#define _STUPID_DELAY_H_
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
void delay_ms(uint32_t delay_in_ms);
void delay_us(uint32_t delay_in_us);
#endif
