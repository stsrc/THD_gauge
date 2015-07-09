#ifndef _ADC_H_
#define _ADC_H_
#include <stm32f10x.h>
#include <core_cm3.h>
void ADC_init();
void ADC_getVal(uint16_t *result);
volatile uint16_t __IO ADC_Result;
#endif
