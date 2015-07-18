#include <stm32f1xx.h>
#include <core_cm3.h>
#include <stdlib.h>
#include "stupid_delay.h"
#include "hd44780.h"
#include "ADC.h"
#include "DAC.h"
#include "timers.h"
#include <arm_math.h>
#include <arm_const_structs.h>
#define TIM2_INTR_NO 28
#define TIM3_INTR_NO 29

#define F_DAC 100000U
#define F_ADC 10000.0f
extern __IO uint16_t ADC_result[256];

struct cos_tab{
	uint16_t *cos_val;
	uint16_t it;
	uint16_t cnt;
};

struct cos_tab glob_cos = {NULL, 0, 0};
float32_t output[256];
uint8_t __IO results_ready = 0;



uint16_t cosine(uint16_t amp, uint16_t ph, uint16_t freq, uint16_t off, float32_t t){
	float32_t theta = 2.0f*PI*(float32_t)freq*t + (float32_t)ph;
	float32_t cos = arm_cos_f32(theta);
	float32_t rt = cos*(float32_t)amp + (float32_t)off + 0.5f;
	if(rt < 0.0f) rt = 0;
	return (uint16_t)rt;
}

void FFT(){
	float32_t tab[512];
	for(uint16_t it = 0; it < 256; it++){
		tab[2*it] = ((float32_t)(ADC_result[it]))/4096.0f;
		tab[2*it+1] = 0;
	}
	arm_cfft_f32(&arm_cfft_sR_f32_len256, tab, 0, 1);
	arm_cmplx_mag_f32(tab, output, 256);
	results_ready = 1;
}

void present_results(){
	if(!results_ready) return;
	results_ready = 0;
	LCD_clear();
	for(uint16_t it = 0; it < 128; it++){
		if(2.0f*output[it]*4096.0f/256.0f > 50.0f){
			LCD_clear();
			LCD_writeUINT32((uint32_t)(0.5f + (float32_t)F_ADC/2.0f/128.0f*(float32_t)it));
			LCD_writeString("Hz");
			LCD_goto(0, 1);
			if(it != 128 && it != 0)LCD_writeUINT32((uint32_t)(
						2.0f*output[it]*4096.0f/256.0f));
			else LCD_writeUINT32((uint32_t)(output[it]*4096.0f/256.0f));
			delay_ms(300);
		}
	}
}

void TIM2_IRQHandler(){
	if(TIM2->SR & TIM_SR_UIF){
		FFT();
		TIM2->SR &= ~TIM_SR_UIF;
	}
}

void generate_cos(uint16_t freq, uint16_t amplitude, uint16_t offset, uint16_t phase){
	float32_t time = 0.0f;
	glob_cos.cnt = (uint16_t)((float32_t)F_DAC/(float32_t)freq + 0.5f);
	if(glob_cos.cos_val != NULL) free(glob_cos.cos_val);
	glob_cos.cos_val = malloc(sizeof(uint16_t)*glob_cos.cnt);
	for(uint16_t it = 0; it < glob_cos.cnt; it++){
		glob_cos.cos_val[it] = cosine(amplitude, phase, freq, offset, time);
		time += 1.0f / ((float32_t)freq) / (float32_t)glob_cos.cnt;
	}
}

uint8_t check_DAC_DMA_errflags(){
	/*if(DAC->SR & DAC_SR_DMAUDR1){
		LCD_clear();
		LCD_writeString("DMA undrr.");
		return 1;
	}*/
	if(DMA1->ISR & DMA_ISR_TEIF3){
		LCD_clear();
		LCD_writeString("DMA err.");
		return 1;
	}
	return 0;
}
int main(void){
	delay_init();
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	NVIC_prioritySet();
	generate_cos(2500, 250, 1000, 0);
	LCD_init();	
	ADC_init();
	DAC_init();
	TIM6_init(glob_cos.cnt, glob_cos.cos_val);
	TIM2_init();
	TIM3_init();
	while(1){
		present_results();
		if(check_DAC_DMA_errflags())while(1);
	}
	if(glob_cos.cos_val != NULL) free(glob_cos.cos_val);
}

