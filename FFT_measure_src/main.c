#include "stupid_delay.h"
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
int main(void){
	uint32_t del = 1000;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	delay_init();
	GPIOC->CRH=1<<5;
	while(1){
		GPIOC->BSRR=1<<25;
		delay_ms(del);
		GPIOC->BSRR=1<<9;
		delay_ms(del);
	}
}

void assert_failed(uint8_t *file, uint32_t line){
	while(1);
}
