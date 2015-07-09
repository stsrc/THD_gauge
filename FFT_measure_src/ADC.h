#ifndef _ADC_H_
#define _ADC_H_
#include <stm32f10x.h>
void ADC_init();
void ADC_getVal(uint16_t *result);
#endif
