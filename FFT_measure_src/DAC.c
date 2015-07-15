#include "DAC.h"

void DAC_init(){
	/*
	 * PA4 - DAC1 output. 
	 *
	 * PA4 and PA5 as input, analog mode
	 * */
	GPIOA->CRL &= ~(GPIO_CRL_MODE4 | GPIO_CRL_CNF4);
	GPIOA->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5);
	/* Enabling clocks */
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	/*DAC channel1 enable*/
	DAC->CR |= DAC_CR_EN1;	
}

void DAC_writeData_12b(uint16_t data){
	DAC->DHR12R1 = data;
}
