/*
 * usr_tim.c
 *
 *  Created on: Aug 6, 2024
 *      Author: eacarel
 */


#include "usr_tim.h"


void UsrTim2Init(void)
{
	uint16_t prescalarValue = 7200 - 1;
	uint16_t autoReloadValue = 10000 - 1;

	TIM2->PSC = prescalarValue;
	TIM2->ARR = autoReloadValue;

	TIM2->SMCR |= (uint32_t)0x56; // bit[6:4] 101: Filtered Timer Input 1 (TI1FP1)  and  bit[2:0] 10: Trigger mode

	TIM2->DIER |= (uint32_t)0x01;  // update interrupt enable

	NVIC_EnableIRQ(TIM2_IRQn);
	TIM2->CR1 |= (uint32_t)0x01;  // counter enable
}
