#include "ADC.h"
#include "stupid_delay.h"
#define ADC_INTR_NO 18 /*STM32F10x manual, p. 130*/
void ADC_init(){

	/*PC3 - probe pin, ADC channel = 13 */

	/*enabling clock source for ADC and GPIOC*/
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_IOPCEN;
	/*setting ADC preslacer to PLCK divided by 8 (24MHz/8?)*/
	RCC->CFGR |= RCC_CFGR_ADCPRE_0 | RCC_CFGR_ADCPRE_1;
	/*reseting PC3 to input - analog mode*/
	GPIOC->CRL &= ~GPIO_CRL_MODE3;
	/*enabling end of conv. interrupt*/
	ADC1->CR1 |= ADC_CR1_EOCIE;
	NVIC_EnableIRQ((IRQn_Type)ADC_INTR_NO);

	/*ADC continous work*/
	ADC1->CR2 |= ADC_CR2_CONT;
	/*Alignment to right*/
	ADC1->CR2 &= ~(ADC_CR2_ALIGN); 
	/*sample time = 28.5 cycles*/
	ADC1->SMPR2 &= ~ADC_SMPR2_SMP3_2;
	ADC1->SMPR2 |= ADC_SMPR2_SMP3_1 | ADC_SMPR2_SMP3_0;
	ADC1->SQR1 = 0; //only 1 conversion
	ADC1->SQR3 = ADC_SQR3_SQ1_3 | ADC_SQR3_SQ1_2 | ADC_SQR3_SQ1_0;
	/*A/D converter ON*/
	ADC1->CR2 |= ADC_CR2_ADON;
	/*initalizing calibration registers, and wait to end*/
	ADC1->CR2 |= ADC_CR2_RSTCAL;
	while(ADC1->CR2 & ADC_CR2_RSTCAL);
	/*A/D calibration*/
	ADC1->CR2 |= ADC_CR2_CAL;
	while(ADC1->CR2 & ADC_CR2_CAL);
	/*^waiting till end of calib*/
	/*ADC start*/
	delay_ms(10);
	ADC1->CR2 |= ADC_CR2_ADON;
}

void ADC_getVal(uint16_t *result){
	while(!(ADC1->SR & ADC_SR_EOC)); //waiting for end
	*result = ADC1->DR;
}

void ADC1_IRQHandler(void){
	ADC_Result = ADC1->DR;
}
