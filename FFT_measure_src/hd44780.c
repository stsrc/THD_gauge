#include "hd44780.h"

#define LCD_port GPIOA
#define RS 0x001
#define E 0x002
#define D4 0x100
#define D5 0x200
#define D6 0x400
#define D7 0x800
#define MARGIN 8 /*Look to the LCD_send_byte definition*/
#define DELAY 10U /* [ms] */
#define LCD_x_cnt 16
#define LCD_y_cnt 2

#define GPIO_setBit(PORT, PIN) (PORT->BSRR |= PIN)
#define GPIO_clearBit(PORT, PIN) (PORT->BSRR |= (PIN << 0x10))

/*
 * TODO: INLINES!
 * */

void LCD_Enable_Strobe(void){
	GPIOA->BSRR |= GPIO_BSRR_BS1;
	delay_us(1);
	GPIOA->BSRR |= GPIO_BSRR_BR1;
	delay_us(1);
}

void LCD_send_byte(uint8_t data, const uint8_t valRS){
	uint32_t cmd = 0;
	uint8_t upper_half = (data & 0b11110000) >> 4;
	uint8_t lower_half = data & 0b00001111;
	if(valRS) GPIOA->BSRR |= GPIO_BSRR_BS0;
	else GPIOA->BSRR |= GPIO_BSRR_BR0;
	cmd = (uint32_t)upper_half << MARGIN;
	cmd |= ((uint32_t)((~upper_half) & 0b00001111) << MARGIN) << 16;
	/*Above: wicked command, I want to clear D4-7 pins.*/
	LCD_port->BSRR = cmd;
	LCD_Enable_Strobe();	
	if(valRS) GPIOA->BSRR |= GPIO_BSRR_BS0;
	else GPIOA->BSRR |= GPIO_BSRR_BR0;
	cmd = (uint32_t)lower_half << MARGIN;
	cmd |= ((uint32_t)((~lower_half) & 0b00001111) << MARGIN) << 16;
	LCD_port->BSRR = cmd;
	LCD_Enable_Strobe();
}

void LCD_send_halfbyte(uint8_t data, const uint8_t valRS){
	uint32_t cmd = 0;
	uint8_t lower_half = data & 0b00001111;
	if(valRS) GPIOA->BSRR |= GPIO_BSRR_BS0;
	else GPIOA->BSRR |= GPIO_BSRR_BR0;
	cmd = (uint32_t)lower_half << MARGIN;
	cmd |= ((uint32_t)((~lower_half) & 0b00001111) << MARGIN) << 16;
	LCD_port->BSRR = cmd;
	LCD_Enable_Strobe();
}
/*TODO: make something that delay will be based on D7 busy line*/
void LCD_send_command(uint8_t command, uint32_t delay){
	LCD_send_byte(command, 0);
	delay_ms(delay);
}
/*sending only half of command - needed by LCD_init function*/
void LCD_send_init_command(uint8_t command, uint32_t delay){
	LCD_send_halfbyte(command, 0);
	delay_ms(delay);
}
void LCD_send_data(uint8_t data, uint32_t delay){
	LCD_send_byte(data, 1);
	delay_ms(delay);
}

void LCD_clear(){
	LCD_send_command(0x01, DELAY);
}



void LCD_goto(uint8_t x, uint8_t y){
	uint8_t cmd = 0x80;
	uint8_t temp = 0;
	if(x > LCD_x_cnt - 1) return;
	if(y > LCD_y_cnt - 1) return;
	temp = x/16;
	if(!y) cmd |= temp<<4;
	else cmd |= (temp<<4) + 0x40;	
	cmd |= (x-temp*16);
	LCD_send_command(cmd, DELAY);
}

/*
 * String must be NULL terminated!
 *
 * Function is temporary:
 * TODO:
 * add \n option
 * add super intelligent auto-line-change feature
 * */
void LCD_writeString(char *string){	
	uint32_t length = strlen(string);
	for(uint32_t it = 0; it < length; it++){
		LCD_send_data(string[it], DELAY);
	}
}

void LCD_writeUINT32(uint32_t value){
	char buf[10];
	if(value >= 1000000000) goto print_err;
	if(sprintf(buf, "%lu", value) <= 0) goto print_err;
	LCD_writeString(buf);
	return;

	print_err:
		LCD_writeString("ERR!\0");
		return;
}
/*TODO: solve issue with sprintf & float*/
void LCD_writeFLOAT(float value){
	LCD_writeString("FLOAT DOESNT WORK");
	return;
	char buf[10];
	if(value >= 1000000000.0f) goto print_err;	
	if(sprintf(buf, "%8.2f", 666.6f) <= 0) goto print_err;
	LCD_writeString(buf);
	return;

	print_err:
		LCD_writeString("ERR!\0");
		return;	
}

void LCD_init(){
	//TODO: THIS IS SO STUPID THAT I CAN'T EVEN LOOK ON IT, YUK
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	LCD_port->CRL |= GPIO_CRL_MODE0_0;
	LCD_port->CRL &= ~GPIO_CRL_CNF0;
	LCD_port->CRL |= GPIO_CRL_MODE1_0;
	LCD_port->CRL &= ~GPIO_CRL_CNF1;
	LCD_port->CRH |= GPIO_CRH_MODE8_0;
	LCD_port->CRH |= GPIO_CRH_MODE9_0;
	LCD_port->CRH |= GPIO_CRH_MODE10_0;
	LCD_port->CRH |= GPIO_CRH_MODE11_0;
	LCD_port->CRH &= ~(GPIO_CRH_CNF8 | GPIO_CRH_CNF9 | GPIO_CRH_CNF10 | GPIO_CRH_CNF11);
	LCD_port->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR8 | GPIO_BSRR_BR9 | GPIO_BSRR_BR10 | GPIO_BSRR_BR11;

	delay_ms(50);
	LCD_send_halfbyte(0x03, 10); /*Initiating program reset*/
	LCD_send_halfbyte(0x03, 5); /*Program reset*/
	LCD_send_halfbyte(0x03, 5);  /*Program reset*/
	LCD_send_halfbyte(0x02, DELAY);  /*Switching to 4 bit mode*/
	LCD_send_command(0x08, DELAY); /*display off*/
	LCD_send_command(0x2F, DELAY); /*Setting 2 line mode*/
	LCD_send_command(0x04, DELAY); /*Entry mode set*/
	LCD_send_command(0x01, DELAY); /*clearing DDRAM*/
	LCD_send_command(0x0C, DELAY); /*display on*/	
}
