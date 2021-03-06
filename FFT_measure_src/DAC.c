#include "DAC.h"

void DAC_init(){
	DAC->CR = 0;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;
	GPIOA->CRL &= ~(GPIO_CRL_CNF4 | GPIO_CRL_MODE4);
	GPIOA->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_MODE5);
	DAC->CR &= ~DAC_CR_TSEL1;
	DAC->CR |= DAC_CR_TEN1;
	DAC->CR |= DAC_CR_DMAEN1;
	DAC->CR |= DAC_CR_EN1;
}

void DAC_writeData_12b(uint16_t data){
	DAC->DHR12R1 = data;
}
