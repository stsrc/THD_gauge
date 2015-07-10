#ifndef ARM_MATH_CM3
#define ARM_MATH_CM3
#endif
#include <stm32f10x.h>
#include <core_cm3.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <arm_math.h>
#include <arm_const_structs.h>
#include <math.h>
#include "stupid_delay.h"
#include "hd44780.h"
#include "ADC.h"
#include "DAC.h"

#define TIM2_INTR_NO 28
#define TIM3_INTR_NO 29

extern __IO q15_t ADC_Result[FFT_SIZE];
__IO uint8_t invokeFFT = 0;

uint16_t privCos(float32_t t, float32_t f,
		float32_t ph, uint16_t A, uint16_t offset){
	return (uint16_t)(((float32_t)A)*cos(2*M_PI*f*t + ph)
		+ (float32_t)offset);
}

void TIM2_init(){
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	/*update request source only from over/under-flow, or DMA*/
	TIM2->CR1 |= TIM_CR1_URS;
	/*Counter enabled*/
	TIM2->CR1 |= TIM_CR1_CEN;
	/*Prescaler = 240 => Fclock = 1MHz;*/
	TIM2->PSC = 240;
	/*Overflow after 10us, what is equal to fn = 100kHz*/
	TIM2->ARR = 10;
	/*NVIC updating*/
	NVIC_EnableIRQ((IRQn_Type)TIM2_INTR_NO);
	/*Update interrupt enabled*/
	TIM2->DIER |= TIM_DIER_UIE;
}

void TIM3_init(){	//Invoking FFT transform
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	/*update request source only from over/under-flow, or DMA*/
	TIM3->CR1 |= TIM_CR1_URS;
	/*Counter enabled*/
	TIM3->CR1 |= TIM_CR1_CEN;
	/*Prescaler = 48000 => Fclock = 500Hz;*/
	TIM3->PSC = 48000;
	/*Overflow after 1s*/
	TIM3->ARR = 500;
	/*NVIC updating*/
	NVIC_EnableIRQ((IRQn_Type)TIM3_INTR_NO);
	/*Update interrupt enabled*/
	TIM3->DIER |= TIM_DIER_UIE;
}

void TIM2_IRQHandler(){
	static float32_t t = 0;
	const float32_t f = 1000;
	const float32_t ph = 0;
	const uint16_t A = 500;
	const uint16_t ofst = 800;	
	if(TIM2->SR & TIM_SR_UIF){
		DAC_writeData_12b(privCos(t, f, ph, A, ofst));
		t += 10e-6;
		if(t < 0) t = 0;
		TIM2->SR &= ~TIM_SR_UIF;
	}
}

void TIM3_IRQHandler(){
	if(TIM3->SR & TIM_SR_UIF){
		invokeFFT = 1;
	}
}

void FFT_transform(){
	if(!invokeFFT) return;
	invokeFFT = 0; 
	arm_rfft_instance_q15 S;
	arm_status rt;
	uint32_t ifftFlagR = 0;
	uint32_t bitReverseFlag = 0;
	q15_t buf[FFT_SIZE/2];
	q15_t buf_mag[FFT_SIZE/2];
	q15_t pResult;
	uint32_t pIndex;
	rt = arm_rfft_init_q15(&S, FFT_SIZE, ifftFlagR, bitReverseFlag);
	if(rt == ARM_MATH_ARGUMENT_ERROR){
		LCD_clear();
		LCD_writeString("ERR in FFT!\0");
		return;
	}
	arm_rfft_q15(&S, ADC_Result, buf);
	arm_cmplx_mag_q15(buf, buf_mag, FFT_SIZE/2);
	arm_max_q15(buf_mag, FFT_SIZE/2, &pResult, &pIndex);
	
	LCD_clear();
	LCD_writeUINT32((uint32_t)pResult);
	LCD_goto(0, 1);
	LCD_writeUINT32((uint32_t)pIndex);
}

int main(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	/*Diode as quasi-debbuger*/
	//GPIOC->CRH=1<<5;
	//GPIOC->BSRR = 1<<9;
	delay_init();
	LCD_init();
	ADC_init();
	DAC_init();
	TIM2_init();
	while(1){
		FFT_transform();	
	}
}

void assert_failed(uint8_t *file, uint32_t line){
	while(1);
}
