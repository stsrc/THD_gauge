#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include "stupid_delay.h"
#include "hd44780.h"

int main(void){
	uint32_t del = 500;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIOC->CRH=1<<5;
	GPIOC->BSRR = 1<<9;
	delay_init();
	LCD_init();
	delay_ms(del);
	GPIOC->BSRR = 1<<25;
	while(1){
		GPIOC->BSRR = 1<<9;
		delay_ms(del);
		GPIOC->BSRR = 1<<25;
		delay_ms(del);
	}
}

void assert_failed(uint8_t *file, uint32_t line){
	while(1);
}
