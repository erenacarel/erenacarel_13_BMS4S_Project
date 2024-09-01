/*
 * usr_adc.c
 *
 *  Created on: Aug 6, 2024
 *      Author: eacarel
 */

#include "usr_adc.h"

#define YUKARI 1
#define ASAGI 0

uint8_t direction[3] = {0};
uint8_t histeris = 20;

uint16_t g_rawAdcReadValues[5];
uint16_t g_batteryValues[4];
uint16_t g_batteryValuesModified[4];

uint16_t g_adcBuffer[4][_USR_ADC_BUF_FILTER_SIZE];
uint16_t g_adcBufferIndex;
uint16_t fark0, fark1, fark2;
uint8_t g_balansComplete;

ADC_ChannelConfTypeDef sConfigPrivate = {0};

static uint16_t AdcValuesMovingAverageProc(uint16_t* buffer, uint8_t size);

static uint16_t AdcValuesMovingAverageProc(uint16_t* buffer, uint8_t size)
{
    uint32_t sum = 0;
    for(uint8_t i = 0; i < size; i++)
    {
        sum += buffer[i];
    }
    return (uint16_t)(sum / size);  // uint16_t
}

void UsrReadAdc(void)
{
	sConfigPrivate.Rank = ADC_REGULAR_RANK_1;
	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_4;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	g_rawAdcReadValues[0] = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_3;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	g_rawAdcReadValues[1] = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_2;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	g_rawAdcReadValues[2] = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_1;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	g_rawAdcReadValues[3] = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_VREFINT;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	g_rawAdcReadValues[4] = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	// lcd panele yazirma islemleri
	int tens[4] = {0};
	int ones[4] = {0};

    for (uint8_t i = 0; i < 4; i++)
    {
    	g_adcBuffer[i][g_adcBufferIndex] = g_rawAdcReadValues[i];
        g_batteryValues[i] = AdcValuesMovingAverageProc(g_adcBuffer[i], _USR_ADC_BUF_FILTER_SIZE);
//        g_batteryValuesModified[i] = g_batteryValues[i] * 0.0310;
//        tens[i] = g_batteryValuesModified[i] / 10;
//        ones[i] = g_batteryValuesModified[i] % 10;
        g_batteryValuesModified[i] = g_batteryValues[i] * 3.1;
        tens[i] = g_batteryValuesModified[i] / 1000;
        ones[i] = g_batteryValuesModified[i] % 10;

        sprintf((char*)m_batteryBufferUpLine, "B0:%d,%d  B1:%d,%d", tens[0], ones[0], tens[1], ones[1]);
        sprintf((char*)m_batteryBufferDownLine, "B2:%d,%d  B3:%d,%d", tens[2], ones[2], tens[3], ones[3]);
    }
    g_adcBufferIndex = (g_adcBufferIndex + 1) % _USR_ADC_BUF_FILTER_SIZE;

//    fark0 = ABS_DIFF(g_batteryValues[0], g_batteryValues[1]);
//    fark1 = ABS_DIFF(g_batteryValues[1], g_batteryValues[2]);
//    fark2 = ABS_DIFF(g_batteryValues[2], g_batteryValues[3]);

	UsrLcdSetCursor(0, 0);
	UsrLcdSendString(m_batteryBufferUpLine);
	UsrLcdSetCursor(1, 0);
	UsrLcdSendString(m_batteryBufferDownLine);
}


void UsrCheckBat0Bat1(void)
{
	uint32_t alt, ust = 0;

	ust = g_batteryValuesModified[1] + histeris;
	alt = g_batteryValuesModified[0] + histeris;

	if (g_batteryValuesModified[1] > alt)
	{
		direction[0] = YUKARI;
		UsrTrigSwapPwm1Channel3();
	}
	if (g_batteryValuesModified[0] > ust)
	{
		direction[0] = ASAGI;
		UsrTrigPwm1Channel1();
	}
//	if (alt == ust)
//	{
//		TIM1->CCER &= ~(uint32_t)0x500;
//	}

//	if ((g_batteryValuesModified[0] > g_batteryValuesModified[1]) && (fark0 > 50))
//	{
//		UsrTrigSwapPwm1Channel1();
////		UsrTrigPwm1Channel1();
//	}
//	if ((g_batteryValuesModified[1] > g_batteryValuesModified[0]) && (fark0 > 50))
//	{
//		UsrTrigPwm1Channel1();
////		UsrTrigSwapPwm1Channel1();
//	}
//	if (fark0 <= 20)
//	{
//		TIM1->CCER &= ~(uint32_t)0x5;
//		g_balansComplete = 1;
//	}
}


void UsrCheckBat1Bat2(void)
{
	uint32_t alt, ust = 0;

	ust = g_batteryValuesModified[2] + histeris;
	alt = g_batteryValuesModified[1] + histeris;

	if (g_batteryValuesModified[2] > alt)
	{
		direction[1] = YUKARI;
		UsrTrigSwapPwm1Channel2();
	}
	if (g_batteryValuesModified[1] > ust)
	{
		direction[1] = ASAGI;
		UsrTrigPwm1Channel2();
	}
//	if (alt == ust)
//	{
//		TIM1->CCER &= ~(uint32_t)0x50;
//	}



//	if (g_balansComplete == 1)
//	{
//		if ((g_batteryValuesModified[1] > g_batteryValuesModified[2]) && (fark1 > 50))
//		{
//			UsrTrigSwapPwm1Channel2();
//	//		UsrTrigPwm1Channel2();
//		}
//		if ((g_batteryValuesModified[2] > g_batteryValuesModified[1]) && (fark1 > 50))
//		{
//			UsrTrigPwm1Channel2();
//	//		UsrTrigSwapPwm1Channel2();
//		}
//		if (fark1 <= 20)
//		{
//			TIM1->CCER &= ~(uint32_t)0x50;
//			g_balansComplete = 2;
//		}
//	}

}


void UsrCheckBat2Bat3(void)
{
	uint32_t alt, ust = 0;

	ust = g_batteryValuesModified[3] + histeris;
	alt = g_batteryValuesModified[2] + histeris;

	if (g_batteryValuesModified[3] > alt)
	{
		direction[2] = YUKARI;
		UsrTrigSwapPwm1Channel1();
	}
	if (g_batteryValuesModified[2] > ust)
	{
		direction[2] = ASAGI;
		UsrTrigPwm1Channel1();
	}
	if (alt == ust)
	{
		TIM1->CCER &= ~(uint32_t)0x4; // close compliment channel
	}


//	if (g_balansComplete == 2)
//	{
//		if ((g_batteryValuesModified[2] > g_batteryValuesModified[3]) && (fark2 > 50))
//		{
//			UsrTrigSwapPwm1Channel3();
//	//		UsrTrigPwm1Channel3();
//		}
//		if ((g_batteryValuesModified[3] > g_batteryValuesModified[2]) && (fark2 > 50))
//		{
//			UsrTrigPwm1Channel3();
//	//		UsrTrigSwapPwm1Channel3();
//		}
//		if (fark2 <= 20)
//		{
//			TIM1->CCER &= ~(uint32_t)0x500;
//			g_balansComplete = 3;
//		}
//	}

}
