#include "hd44780.h"

#define LCD_port GPIOA
#define RS GPIO_Pin_0
#define E GPIO_Pin_1
#define D4 GPIO_Pin_2
#define D5 GPIO_Pin_3
#define D6 GPIO_Pin_4
#define D7 GPIO_Pin_5
#define MARGIN 2 /*Look to the LCD_send_byte definition*/

void LCD_init(){
	GPIO_InitTypeDef gpio_struct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_StructInit(&gpio_struct);
	gpio_struct.GPIO_Pin = RS | E | D4 | D5 | D6 | D7;
	gpio_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_struct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(LCD_port, &gpio_struct);
	GPIO_ResetBits(LCD_port, RS | E | D4 | D5 | D6 | D7);
	return;
}

void LCD_Enable_Strobe(void){
	GPIO_WriteBit(LCD_port, E, 1);
	delay_ms(1000);
	GPIO_WriteBit(LCD_port, E, 0);
	delay_ms(1000);
}

void LCD_send_byte(uint8_t data, const uint8_t valRS){
	uint32_t cmd = 0;
	uint8_t upper_half = (data & 0b11110000) >> 4;
	uint8_t lower_half = data & 0b00001111;
	if(valRS) GPIO_WriteBit(LCD_port, RS, 1);
	else GPIO_WriteBit(LCD_port, RS, 0);
	cmd = (upper_half << MARGIN);
	cmd |= (((~upper_half) & 0b00001111) << MARGIN) << 16;
	/*Above: wicked command, I want to clear D4-7 pins.*/
	LCD_port->BSRR = cmd;
	LCD_Enable_Strobe();	
	if(valRS) GPIO_WriteBit(LCD_port, RS, 1);
	else GPIO_WriteBit(LCD_port, RS, 0);
	cmd = (lower_half << MARGIN);
	cmd |= (((~lower_half) & 0b00001111) << MARGIN) << 16;
	/*Above: wicked command, I want to clear D4-7 pins.*/
	LCD_port->BSRR = cmd;
	LCD_Enable_Strobe();
}

void LCD_send_command(uint8_t command){
	LCD_send_byte(command, 0);
}
