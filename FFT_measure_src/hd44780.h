#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
void lcdInit(void);
void sendCMD(uint8_t c);
void printChar(uint8_t c);
void printString(uint8_t *s);
void clearLCD(void);
void toLine1(void);
void toLine2(void);
