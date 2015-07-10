#ifndef _ADC_H_
#define _ADC_H_

#define FFT_SIZE 128

#include <stm32f10x.h>
#include <core_cm3.h>

#ifndef ARM_MATH_CM3
#define ARM_MATH_CM3
#endif

#include <arm_math.h>
void ADC_init();
__IO q15_t ADC_Result[FFT_SIZE];
#endif
