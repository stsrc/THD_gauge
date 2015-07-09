#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include "stupid_delay.h"
#include "hd44780.h"
#include "ADC.h"
#include "DAC.h"
extern volatile uint16_t __IO ADC_Result;
int main(void){
	uint32_t del = 500;
	uint16_t output[5] = {0, 1000, 2000, 3000, 4000};
	uint8_t cnt = 0;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIOC->CRH=1<<5;
	GPIOC->BSRR = 1<<9;
	delay_init();
	LCD_init();
	ADC_init();
	DAC_init();
	GPIOC->BSRR = 1<<25;
	while(1){
		GPIOC->BSRR = 1<<9;
		DAC_writeData_12b(output[cnt]);
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
