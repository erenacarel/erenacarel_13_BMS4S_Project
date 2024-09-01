#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usr_system.h"

uint32_t g_adcBuffer[USR_ADC_BUFFER_SIZE];
char g_rxData[40];
char g_rxTempBuf[10];
uint8_t g_rxIndex;
char g_sendData[40];
int pil[4] = {0};
uint16_t g_pwmValue;

void UsrSystemInitial(void)
{
	HAL_UART_Receive_IT(&huart1, (uint8_t*)g_rxTempBuf, 40);
//	HAL_ADCEx_Calibration_Start(&hadc1);
//	HAL_ADC_Start_DMA(&hadc1, g_adcBuffer, USR_ADC_BUFFER_SIZE);
//	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 200);  // 2000-> CCRx value
}

// 1 IN+ -> CELL4+     1 IN- -> CELL4-  1 OUT->ADC1
// 2 IN+ -> CELL3-     2 IN- -> CELL4-  2 OUT->ADC2
// 3 IN+ -> CELL3-     3 IN- -> CELL2-  3 OUT->ADC3
// 4 IN+ -> CELL2-     4 IN- -> CELL1-  4 OUT->ADC4
void UsrSystemGeneral(void)
{
//	UsrCellCalculate();
//	HAL_UART_Transmit(&huart1, (uint8_t*)g_sendData, sprintf(g_sendData, "%d%d%d%d", pil[0], pil[1], pil[2], pil[3]), HAL_MAX_DELAY);
//	for(uint16_t i = 100; i <= 1000; i+=100)
//	{
//		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, i);  // 2000-> CCRx value
//		HAL_Delay(5);
//	}

//	if (g_rxData[0] == '1')
//	{
//		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
//	}
//	else if()
//	{
//
//
//	}
}

/*
	TIM_CLCOK = APB_TIM_CLOCK / PRESCALAR    72000000 / 72 = 1 MHz
	FREQUENCY = TIM_CLOCK / ARR              1000000 / 1000 = 1 KHz
	DUTY = (CCRx / ARR) x 100                200 / 1000 x100 = 20
*/
void UsrSetPwm(uint16_t pulseValue)
{
	TIM_OC_InitTypeDef sConfigOC;

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = pulseValue;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}


uint32_t UsrVrefVoltage(void)
{
	float vref = 0.0;
	uint32_t deger = 0;
    vref = (1.20 / g_adcBuffer[6]) * 4095;
    deger = vref * 1000;
    return deger;
}


uint32_t UsrVrefConversion(uint32_t kanal, uint32_t vrefint)
{
	return (vrefint * kanal) / 4095;
}


void UsrCellCalculate(void)
{
	for (uint8_t i = 0; i < 4; ++i)
	{
		pil[i] = (uint16_t)UsrVrefConversion(g_adcBuffer[i], UsrVrefVoltage());
	}
}




