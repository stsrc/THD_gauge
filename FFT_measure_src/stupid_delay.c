#include "stupid_delay.h"
 volatile uint32_t delay_val;

void delay_init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	if(SysTick_Config(SystemCoreClock / 1000000)){
		GPIOC->CRH = 1<<1; //for debug purpose
		GPIOC->BSRR = 1<<8; 
		int dummy = 2;
		while(1){
			dummy *= 2;
		}
	}
}

void delay_ms(uint32_t value){	//unit of value = [ms]
	delay_val = value*1000;
	while(delay_val){}
}

void delay_us(uint32_t value){	//unit - [us]
	delay_val = value;
	while(delay_val){}
}

void SysTick_Handler(void){
	if(delay_val) delay_val--;
}
