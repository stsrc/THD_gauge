#ifndef _HD44780_H_
#define _HD44780_H_
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include "stupid_delay.h"
void LCD_init();
void LCD_send_byte(uint8_t data, const uint8_t valRS);
#endif
