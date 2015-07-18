#include <stm32f1xx.h>
#include <core_cm3.h>
#include <stdlib.h>
#include "stupid_delay.h"
#include "hd44780.h"
#include "ADC.h"
#include "DAC.h"
#include "timers.h"
#include <arm_math.h>
#include <math.h>
#include <arm_const_structs.h>


#define F_DAC 100000U
#define F_ADC 10000.0f

#define FFT_SIZE 256 //DANGER, you should also change const struct in FFT();

extern __IO uint16_t ADC_result[FFT_SIZE];

struct cos_tab{
	uint16_t *cos_val;
	uint16_t it;
	uint16_t cnt;
};

struct cos_tab glob_cos = {NULL, 0, 0};
float32_t output[FFT_SIZE];
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
	for(uint16_t it = 0; it < FFT_SIZE; it++){
		tab[2*it] = ((float32_t)(ADC_result[it]))/4096.0f;
		tab[2*it+1] = 0;
	}
	arm_cfft_f32(&arm_cfft_sR_f32_len256, tab, 0, 1);
	arm_cmplx_mag_f32(tab, output, FFT_SIZE);
	results_ready = 1;
}

uint32_t get_max_bar(float32_t *tab, uint32_t FFT_size){
	float32_t temp[FFT_size/2 + 1];
	float32_t max_val;
	uint32_t bin_no;
	for(uint32_t it = 0; it <= FFT_size/2; it++){
		temp[it] = tab[it];
	}
	temp[0] = 0;
	arm_max_f32(temp, FFT_size/2 + 1, &max_val, &bin_no);
	return bin_no;
}	

float32_t calculate_THD(float32_t *tab, uint32_t FFT_size, uint32_t fundamental_it){
	float32_t *temp;
	float32_t rt;
	temp = malloc(sizeof(float32_t)*(FFT_size/2)/fundamental_it);
	for(uint32_t it = 2; it*fundamental_it <= FFT_size/2; it++){
		temp[it - 2] = tab[it*fundamental_it];
	}
	arm_rms_f32(temp, FFT_size/2/fundamental_it, &rt);
	free(temp);
	return rt/tab[fundamental_it];
}

float32_t calculate_frequency(uint32_t tab_no, uint32_t FFT_size, uint32_t ADC_freq){
	float32_t rt = (float32_t)ADC_freq/2.0f;
	rt = rt/((float32_t)FFT_size / 2.0f);
	rt = rt*(float32_t)tab_no;
	return rt;
}

float32_t calculate_dB(float32_t val, float32_t fft_size){
	return log10f(val/fft_size);
}

void present_results(){
	float32_t THD;
	uint32_t fundamental_it, frequency;
	if(!results_ready) return;
	fundamental_it = get_max_bar(output, FFT_SIZE);
	THD = calculate_THD(output, FFT_SIZE, fundamental_it);
	frequency = (uint32_t)calculate_frequency(fundamental_it, FFT_SIZE, F_ADC);
	LCD_clear();
	LCD_writeString("THD = ");
	LCD_writeFLOAT(THD);
	LCD_goto(0, 1);
	LCD_writeUINT32(frequency);
	LCD_writeString("Hz: ");
	LCD_writeFLOAT(calculate_dB(output[fundamental_it], (float32_t)FFT_SIZE));
	LCD_writeString("dB");
	results_ready = 0;
}

void TIM2_IRQHandler(){
	if(TIM2->SR & TIM_SR_UIF){
		if(!results_ready) FFT();
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
	if(DAC->SR & DAC_SR_DMAUDR1){
		LCD_clear();
		LCD_writeString("DMA undrr.");
		return 1;
	}
	if(DMA1->ISR & DMA_ISR_TEIF3){
		LCD_clear();
		LCD_writeString("DMA err.");
		return 1;
	}
	return 0;
}
int main(void){
	NVIC_prioritySet();
	delay_init();	
	LCD_init();	
	generate_cos(4759, 250, 1000, 0);
	ADC_init();
	DAC_init();
	TIM6_init(glob_cos.cnt, glob_cos.cos_val);
	TIM2_init();
	TIM3_init();
	while(1){
		present_results();
		if(check_DAC_DMA_errflags()) while(1);
	}
	if(glob_cos.cos_val != NULL) free(glob_cos.cos_val);
}

