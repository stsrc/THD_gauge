#include "hd74880.h"

#define RS GPIO_Pin_0
#define E GPIO_Pin_1
#define D4 GPIO_Pin_2
#define D5 GPIO_Pin_3
#define D6 GPIO_Pin_4
#define D7 GPIO_Pin_5


void LCD_init(){
	GPIO_InitTypeDef gpio_struct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_StructInit(&gpio_struct);
	gpio_struct.GPIO_Pin = RS | E | D4 | D5 | D6 | D7;
	gpio_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_struct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &gpio_struct);
	GPIO_SetBits(GPIOC, RS | E | D4 | D5 | D6 | D7, 0);
	return;
}

void LCD_send_command(uint8_t command){

	return;
}
