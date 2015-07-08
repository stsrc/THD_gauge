#ifndef _HD44780_H_
#define _HD44780_H_
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include "stupid_delay.h"
#include <string.h>
void LCD_init();
void LCD_goto(uint8_t x, uint8_t y);
/* CAUTION: string must be NULL terminated */
void LCD_writeString(char *string);
#endif
