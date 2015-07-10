#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include "stupid_delay.h"
#include "hd44780.h"
#include "ADC.h"
#include "DAC.h"
#define TIM2_INTR_NO 28
extern volatile uint16_t __IO ADC_Result;

void TIM2_init(){
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	/*update request source only from over/under-flow, or DMA*/
	TIM2->CR1 |= TIM_CR1_URS;
	/*Counter enabled*/
	TIM2->CR1 |= TIM_CR1_CEN;
	/*Prescaler = 240 => Fclock = 1MHz;*/
	TIM2->PSC = 240;
	/*Overflow after 10us, what gives fn = 100kHz;*/
	TIM2->ARR = 10;
	/*NVIC updating*/
	NVIC_EnableIRQ((IRQn_Type)TIM2_INTR_NO);
	/*Update interrupt enabled*/
	TIM2->DIER |= TIM_DIER_UIE;
}

void TIM2_IRQHandler(){
	uint16_t test[5] = {200, 400, 600, 800, 1000};
       static uint8_t it = 0;	
	if(TIM2->SR & TIM_SR_UIF){
		DAC_writeData_12b(test[it]);
		it = (it + 1) % 5;
		TIM2->SR &= ~TIM_SR_UIF;
	}
}

int main(void){
	uint32_t del = 250;
	uint8_t cnt = 0;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIOC->CRH=1<<5;
	GPIOC->BSRR = 1<<9;
	delay_init();
	LCD_init();
	ADC_init();
	DAC_init();
	TIM2_init();
	while(1){
		GPIOC->BSRR = 1<<9;
		cnt = (cnt + 1)%5;
		delay_ms(del);
		GPIOC->BSRR = 1<<25;
		LCD_clear();
		LCD_writeString("RES=");
		LCD_writeUINT32(ADC_Result);
		delay_ms(del);
		
	}
}

void assert_failed(uint8_t *file, uint32_t line){
	while(1);
}
