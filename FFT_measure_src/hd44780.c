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


void LCD_setToWrite();
void LCD_setToRead();

static uint8_t LCD_x = 0, LCD_y = 0;

inline void LCD_Enable_Strobe(void){
	GPIO_setBit(LCD_port, E);
	delay_us(1);
	GPIO_clearBit(LCD_port, E);
	delay_us(1);
}

inline void LCD_wait(){
	LCD_setToRead();
	GPIO_setBit(LCD_port, E);
	while(LCD_port->IDR & GPIO_IDR_IDR11);
	GPIO_clearBit(LCD_port, E);
	LCD_setToWrite();
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

void LCD_send_command(uint8_t command){
	LCD_send_byte(command, 0);
	LCD_wait();
}
/*sending only half of command - needed by LCD_init function*/
void LCD_send_init_command(uint8_t command, uint32_t delay){
	LCD_send_halfbyte(command, 0);
	delay_ms(delay);
}
void LCD_send_data(uint8_t data){
	LCD_send_byte(data, 1);
	LCD_wait();
}

void LCD_clear(){
	LCD_x = 0;
	LCD_y = 0;
	LCD_send_command(0x01);
}



void LCD_goto(uint8_t x, uint8_t y){
	uint8_t cmd = 0x80;
	uint8_t temp = 0;
	if((x > LCD_x_cnt - 1) || (y > LCD_y_cnt - 1)){
		LCD_clear();
		LCD_writeString("LCD_ERR!\nWRONG GOTO!");
		delay_ms(3000);
		LCD_clear();
	       	return;
	}
	LCD_x = x;
	LCD_y = y;
	temp = x/16;
	if(!y) cmd |= temp<<4;
	else cmd |= (temp<<4) + 0x40;	
	cmd |= (x-temp*16);
	LCD_send_command(cmd);
}

void LCD_writeString(char *string){	
	uint8_t length = strlen(string);
	for(uint8_t it = 0; it < length; it++){
		if(LCD_x > LCD_x_cnt - 1){
			LCD_x = 0;
			LCD_y++;
			if(LCD_y > LCD_y_cnt - 1){
				LCD_y = 0;
				LCD_goto(LCD_x, LCD_y);
			}
			else{
				LCD_goto(LCD_x, LCD_y);
			}
		}
		else if(string[it] == '\n'){
			LCD_x = 0;
			LCD_y++;
			if(LCD_y > LCD_y_cnt - 1) LCD_y = 0;
			LCD_goto(LCD_x, LCD_y);
			continue;	
		}
		LCD_send_data(string[it]);
		LCD_x++;
	}
}

void LCD_writeUINT32(uint32_t value){
	char buf[10];
	if(value >= 1000000000){
	       	LCD_writeString("ERR!\0");
		return;
	}
	if(sprintf(buf, "%lu", value) <= 0){
	       	LCD_writeString("ERR!\0");
		return;
	}
	LCD_writeString(buf);
}

#ifdef HD_FLOAT 
void LCD_writeFLOAT(float value){
	char buf[10];
	if(value >= 1000000000.0) goto print_err;	
	if(sprintf(buf, "%.3f", value) <= 0) goto print_err;
	LCD_writeString(buf);
	return;
	print_err:
		LCD_writeString("ERR!\0");
		return;	
}
#else
void LCD_writeFLOAT(float value){
	char buf[10];
	int32_t d1, d2;
	if(value >= 1000000000.0) goto print_err;	
	//if(sprintf(buf, "%.3f", value) <= 0) goto print_err;
	d1 = (int32_t)value;
	d2 = abs((int32_t)((value - d1)*100.0f));
	if(sprintf(buf, "%ld.%ld", d1, d2) <= 0) goto print_err;
	LCD_writeString(buf);
	return;
	print_err:
		LCD_writeString("ERR!\0");
		return;	
}
#endif

void LCD_setToRead(){
	LCD_port->CRL |= GPIO_CRL_MODE0_0;
	LCD_port->CRL &= ~GPIO_CRL_CNF0;
	LCD_port->CRL |= GPIO_CRL_MODE1_0;
	LCD_port->CRL &= ~GPIO_CRL_CNF1;
	LCD_port->CRL |= GPIO_CRL_MODE2_0;
	LCD_port->CRL &= ~GPIO_CRL_CNF2;
	LCD_port->CRH &= ~GPIO_CRH_MODE8;
	LCD_port->CRH &= ~GPIO_CRH_MODE9;
	LCD_port->CRH &= ~GPIO_CRH_MODE10;
	LCD_port->CRH &= ~GPIO_CRH_MODE11;
	LCD_port->CRH &= ~(GPIO_CRH_CNF8 | GPIO_CRH_CNF9 | GPIO_CRH_CNF10 | GPIO_CRH_CNF11);
	LCD_port->CRH |= GPIO_CRH_CNF8_1 | GPIO_CRH_CNF9_1 | GPIO_CRH_CNF10_1 | GPIO_CRH_CNF11_0;
	LCD_port->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR8 | GPIO_BSRR_BR9 | GPIO_BSRR_BR10 | GPIO_BSRR_BS11;
	LCD_port->BSRR |= GPIO_BSRR_BS2;
}

void LCD_setToWrite(){
	LCD_port->CRL |= GPIO_CRL_MODE0_0;
	LCD_port->CRL &= ~GPIO_CRL_CNF0;
	LCD_port->CRL |= GPIO_CRL_MODE1_0;
	LCD_port->CRL &= ~GPIO_CRL_CNF1;
	LCD_port->CRL |= GPIO_CRL_MODE2_0;
	LCD_port->CRL &= ~GPIO_CRL_CNF2;
	LCD_port->CRH |= GPIO_CRH_MODE8_0;
	LCD_port->CRH |= GPIO_CRH_MODE9_0;
	LCD_port->CRH |= GPIO_CRH_MODE10_0;
	LCD_port->CRH |= GPIO_CRH_MODE11_0;
	LCD_port->CRH &= ~(GPIO_CRH_CNF8 | GPIO_CRH_CNF9 | GPIO_CRH_CNF10 | GPIO_CRH_CNF11);
	LCD_port->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR8 | GPIO_BSRR_BR9 | GPIO_BSRR_BR10 | GPIO_BSRR_BR11;
}

void LCD_test(){
	LCD_clear();
	for(uint8_t it = 0; it < LCD_x_cnt - 3; it++){
		LCD_goto(it, 0);
		LCD_writeString("te");
		LCD_goto(2 + it, 1);
		LCD_writeString("st");
		delay_ms(250);
		it++;
	}
	delay_ms(500);
	LCD_goto(LCD_x_cnt, 0);
	LCD_writeString("line_0\nline_1");
	while(1);
}

void LCD_init(){
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	LCD_setToWrite();
	delay_ms(50);
	LCD_send_init_command(0x03, 10); /*Initiating program reset*/
	LCD_send_init_command(0x03, 5); /*Program reset*/
	LCD_send_init_command(0x03, 5);  /*Program reset*/
	LCD_send_init_command(0x02, DELAY);  /*Switching to 4 bit mode*/
	LCD_send_command(0x08); /*display off*/
	LCD_send_command(0x2F); /*Setting 2 line mode*/
	LCD_send_command(0x04); /*Entry mode set*/
	LCD_send_command(0x01); /*clearing DDRAM*/
	LCD_send_command(0x0C); /*display on*/	
}
