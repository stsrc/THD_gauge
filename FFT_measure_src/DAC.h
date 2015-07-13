#ifndef _DAC_H_
#define _DAC_H_
#include <stm32f1xx.h>
void DAC_init();
void DAC_writeData_12b(uint16_t data);
#endif
