#include "timers.h"

#define TIM2_INTR_NO 28
#define TIM3_INTR_NO 29

inline void TIM6_DMA_configure(uint16_t data_cnt, uint16_t *mem_ptr){
	DMA1_Channel3->CCR = 0;
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA1_Channel3->CPAR = (uint32_t)((uint32_t*)DAC + 2); 
	DMA1_Channel3->CMAR = (uint32_t)mem_ptr;
	DMA1_Channel3->CNDTR = data_cnt; 
	DMA1_Channel3->CCR |= DMA_CCR_PL;
	DMA1_Channel3->CCR |= DMA_CCR_DIR;
	DMA1_Channel3->CCR |= DMA_CCR_CIRC;
	DMA1_Channel3->CCR |= DMA_CCR_MINC;
	DMA1_Channel3->CCR |= DMA_CCR_MSIZE_0;
	DMA1_Channel3->CCR |= DMA_CCR_PSIZE_1;
	AFIO->MAPR2 |= AFIO_MAPR2_TIM67_DAC_DMA_REMAP;
	DMA1_Channel3->CCR |= DMA_CCR_EN;	
}

/*DAC timer*/
void TIM6_init(uint16_t data_cnt, uint16_t *mem_ptr){
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	TIM6->CR1 |= TIM_CR1_URS;
	TIM6->CR2 |= TIM_CR2_MMS_1;
	TIM6->PSC = 15;
	TIM6->ARR = 14;
	TIM6->CR1 |= TIM_CR1_CEN;
	TIM6_DMA_configure(data_cnt, mem_ptr);
}

/*FFT perform timer*/
void TIM2_init(){
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	/*update request source only from over/under-flow, or DMA*/
	TIM2->CR1 |= TIM_CR1_URS;
	/*Prescaler = 54000 => Fclock = 500Hz;*/
	TIM2->PSC = 54000;
	/*Overflow after 1s*/
	TIM2->ARR = 500;
	/*NVIC updating*/
	NVIC_EnableIRQ((IRQn_Type)TIM2_INTR_NO);
	/*Update interrupt enabled*/
	TIM2->DIER |= TIM_DIER_UIE;
	/*Counter enabled*/
	TIM2->CR1 |= TIM_CR1_CEN;
}

void TIM3_init(){
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	/*update request source only from over/under-flow, or DMA*/
	TIM3->CR1 |= TIM_CR1_URS;
	/*Prescaler = 20 => Fclock = 1.2MHz;*/
	//TIM3->PSC = 19;
	TIM3->PSC = 49;
	/*Event after 25 ticks => T = 20.833(3)us, f = 48kHz.*/
	//TIM3->ARR = 24;
	TIM3->ARR = 47;
	/*Enabling trigger output event on update event*/
	TIM3->CR2 &= ~TIM_CR2_MMS;
	TIM3->CR2 |= TIM_CR2_MMS_1;
	/*Counter enabled*/
	TIM3->CR1 |= TIM_CR1_CEN;
}

void NVIC_prioritySet(){
	uint32_t encoded_priority;
	uint32_t adc, systick, fft;
	fft = 0;
	adc = 1;
	systick = 2;
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
	NVIC_SetPriority((IRQn_Type)TIM2_INTR_NO, encoded_priority);
}


