#ifndef _ADC_H_
#define _ADC_H_
#include <stm32f1xx.h>
#include <core_cm3.h>
void ADC_init();
__IO uint16_t ADC_result[256];
#endif
