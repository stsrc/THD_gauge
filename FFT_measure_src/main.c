#include <stm32f1xx.h>
#include <core_cm3.h>
#include <stdlib.h>
#include "stupid_delay.h"
#include "hd44780.h"
#include "ADC.h"
#include "DAC.h"
#include <arm_math.h>
#include <arm_const_structs.h>
#define TIM2_INTR_NO 28
#define TIM3_INTR_NO 29

#define F_DAC 100000U

extern __IO uint16_t ADC_result[256];

struct cos_tab{
	uint16_t *cos_val;
	uint16_t it;
	uint16_t cnt;
};

struct cos_tab glob_cos = {NULL, 0, 0};
float32_t output[256];
uint8_t __IO results_ready = 0;


void TIM2_DMA_configure(uint16_t data_cnt, uint16_t *mem_ptr){
	DMA1_Channel2->CCR = 0;
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA1_Channel2->CPAR = (uint32_t)((uint32_t*)DAC + 2); 
	DMA1_Channel2->CMAR = (uint32_t)mem_ptr;
	DMA1_Channel2->CNDTR = data_cnt; 
	DMA1_Channel2->CCR |= DMA_CCR_PL;
	DMA1_Channel2->CCR |= DMA_CCR_DIR;
	DMA1_Channel2->CCR |= DMA_CCR_CIRC;
	DMA1_Channel2->CCR |= DMA_CCR_MINC;
	DMA1_Channel2->CCR |= DMA_CCR_MSIZE_0;
	DMA1_Channel2->CCR |= DMA_CCR_PSIZE_1;
	DMA1_Channel2->CCR |= DMA_CCR_EN;	
}

/*DAC timer*/
void TIM2_init(uint16_t cnt, uint16_t *ptr){
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->CR1 |= TIM_CR1_URS;
	TIM2->DIER |= TIM_DIER_UDE;
	TIM2->PSC = 11;
	TIM2->ARR = 19;
	TIM2->CR1 |= TIM_CR1_CEN;
	TIM2_DMA_configure(cnt, ptr);
}

/*FFT perform timer*/
void TIM3_init(){
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	/*update request source only from over/under-flow, or DMA*/
	TIM3->CR1 |= TIM_CR1_URS;
	/*Prescaler = 54000 => Fclock = 500Hz;*/
	TIM3->PSC = 54000;
	/*Overflow after 1s*/
	//TIM3->ARR = 500;
	TIM3->ARR = 1500;
	/*NVIC updating*/
	NVIC_EnableIRQ((IRQn_Type)TIM3_INTR_NO);
	/*Update interrupt enabled*/
	TIM3->DIER |= TIM_DIER_UIE;
	/*Counter enabled*/
	TIM3->CR1 |= TIM_CR1_CEN;
}

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
			LCD_writeUINT32(it);
			LCD_goto(0, 1);
			LCD_writeUINT32((uint32_t)(output[it]*4096.0f/256.0f));
			delay_ms(300);
		}
	}
}

void TIM3_IRQHandler(){
	if(TIM3->SR & TIM_SR_UIF){
		FFT();
		TIM3->SR &= ~TIM_SR_UIF;
	}
}

void NVIC_prioritySet(){
	uint32_t encoded_priority;
	uint32_t adc, systick, fft;
	fft = 0;
	adc = 1;
	systick = 2;
	//systick = 0;
	/*0 priority groups*/
	NVIC_SetPriorityGrouping(0);
	/*ADC priority*/
	encoded_priority = NVIC_EncodePriority(0, adc, 0);
	NVIC_SetPriority((IRQn_Type)18, encoded_priority);
	/*SysTick priority*/
	encoded_priority = NVIC_EncodePriority(0, systick, 0);
	NVIC_SetPriority((IRQn_Type)-1, encoded_priority);
	/*FFT priority*/
	encoded_priority = NVIC_EncodePriority(0, fft, 0);
	NVIC_SetPriority((IRQn_Type)TIM3_INTR_NO, encoded_priority);
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
	GPIOC->CRH &= ~GPIO_CRH_CNF8;
	GPIOC->CRH |= GPIO_CRH_MODE8_1;
	GPIOC->BSRR |= GPIO_BSRR_BR8;
	NVIC_prioritySet();
	generate_cos(3000, 250, 1000, 0);
	LCD_init();	
	ADC_init();
	DAC_init();
	TIM2_init(glob_cos.cnt, glob_cos.cos_val);
	GPIOC->BSRR |= GPIO_BSRR_BS8;
	TIM3_init();
	while(1){
		present_results();
		if(check_DAC_DMA_errflags())while(1);
	}
	if(glob_cos.cos_val != NULL) free(glob_cos.cos_val);
}

