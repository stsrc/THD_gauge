#include <stm32f1xx.h>
#include <core_cm3.h>
#include "stupid_delay.h"
#include "hd44780.h"
#include "ADC.h"
#include "DAC.h"

#define TIM2_INTR_NO 28
#define TIM3_INTR_NO 29

void TIM2_init(){
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	/*update request source only from over/under-flow, or DMA*/
	TIM2->CR1 |= TIM_CR1_URS;
	/*Prescaler = 240 => Fclock = 1MHz;*/
	//TIM2->PSC = 240;
	TIM2->PSC = 54000;
	/*Overflow after 10us, what is equal to fn = 100kHz*/
	//TIM2->ARR = 10;
	TIM2->ARR = 500;
	/*Update interrupt enabled*/
	TIM2->DIER |= TIM_DIER_UIE;
	/*NVIC updating*/
	NVIC_EnableIRQ((IRQn_Type)TIM2_INTR_NO);
	/*Counter enabled*/
	TIM2->CR1 |= TIM_CR1_CEN;
}

void TIM3_init(){
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	/*update request source only from over/under-flow, or DMA*/
	TIM3->CR1 |= TIM_CR1_URS;
	/*Prescaler = 54000 => Fclock = 500Hz;*/
	TIM3->PSC = 54000;
	/*Overflow after 1s*/
	TIM3->ARR = 500;
	/*NVIC updating*/
	NVIC_EnableIRQ((IRQn_Type)TIM3_INTR_NO);
	/*Update interrupt enabled*/
	TIM3->DIER |= TIM_DIER_UIE;
	/*Counter enabled*/
	TIM3->CR1 |= TIM_CR1_CEN;
}

void TIM2_IRQHandler(){	
	if(TIM2->SR & TIM_SR_UIF){
		DAC_writeData_12b(1337);
		TIM2->SR &= ~TIM_SR_UIF;
	}
}

void TIM3_IRQHandler(){
	if(TIM3->SR & TIM_SR_UIF){
		LCD_clear();
		LCD_writeString("TEST ");
		TIM3->SR &= ~TIM_SR_UIF;
	}
}


int main(void){
	delay_init();
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	GPIOC->CRH &= ~GPIO_CRH_CNF8;
	GPIOC->CRH |= GPIO_CRH_MODE8_1;
	GPIOC->BSRR |= GPIO_BSRR_BR8;
	LCD_init();	
	ADC_init();
	DAC_init();
	TIM2_init();
	GPIOC->BSRR |= GPIO_BSRR_BS8;
	TIM3_init();
	while(1){	
		delay_ms(500);
		GPIOC->BSRR |= GPIO_BSRR_BS8;
		LCD_writeString("o");
		delay_ms(500);
		GPIOC->BSRR |= GPIO_BSRR_BR8;
	}
}

