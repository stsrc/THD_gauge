#include "hd44780.h"

int main(void){
	int it = 0;
	lcdInit();
	clearLCD();
	printString("TEST TEST\n");
	while(1){
		it++;
	}
}

void assert_failed(uint8_t *file, uint32_t line){
	while(1);
}
