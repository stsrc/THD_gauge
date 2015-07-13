#ifndef _HD44780_H_
#define _HD44780_H_
#include <stm32f1xx.h>
#include "stupid_delay.h"
#include <string.h>
#include <stdio.h>
void LCD_init();
void LCD_goto(uint8_t x, uint8_t y);
/* CAUTION: string must be NULL terminated */
void LCD_writeString(char *string);
void LCD_clear();
void LCD_writeUINT32(uint32_t value);
void LCD_writeFLOAT(float value);
#endif
