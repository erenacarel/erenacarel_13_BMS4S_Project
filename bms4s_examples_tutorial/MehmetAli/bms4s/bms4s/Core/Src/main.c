/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gnd16x2_i2c.h"
#include "gnd_uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#define ADC_BUFFER_SIZE 7
#define voltajFark 10
#define azamiVoltaj 1100 //çarpi 4 = pilin gerilimi
#define asgariVoltaj 850 // 3.2 pil gerilimi
#define asgariGerekliVolt 900
#define MAX_PWM_DUTY_CYCLE 70

//uint16_t adcDmaDeger[7], adcHamDeger[7];
char giden_data[40] = { 0 }, gelen_data[20] = { 0 };
int counter = 0;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

osThreadId KontrolTaskHandle;
osThreadId AdcTaskHandle;
osThreadId BalansTaskHandle;
/* USER CODE BEGIN PV */
uint32_t adc_buffer[ADC_BUFFER_SIZE];
bool adc_ready = false;
int pil[4] = { 0 };
int akim = 0;

int temperature;
const uint16_t V25 = 1790; // when V25=1.41V at ref 3.3V 1750
const uint16_t Avg_Slope = 5; //when avg_slope=4.3mV/C at ref 3.3V

volatile bool asiri_akim = false;
volatile bool kisa_devre = false;

uint32_t counterPeriod = 0;
uint32_t prescalerDeger = 0;
uint32_t frekans = 0;
uint8_t duty = 0;
uint8_t hesaplananPwm[3];

bool volatile azamiDurum = true;
bool volatile asgariDurum = true;
bool volatile balansDurum = false;

volatile int16_t voltFark[3] = { 0 };
volatile uint16_t max = 0;
uint16_t min = 0;
int maxIndex = 0;
int minIndex = 0;
uint32_t ortalama = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM3_Init(void);
void StartKontrolTask(void const *argument);
void StartAdcTask(void const *argument);
void StartBalansTask(void const *argument);

/* USER CODE BEGIN PFP */

uint32_t vref(void) {
	float Vref = 0.0;
	uint32_t dgr = 0;
	Vref = (1.20 / adc_buffer[6]) * 4095.0;
	dgr = Vref * 1000;
	return dgr;
}

int tempMCU(void) {
	temperature =
			(uint16_t) ((V25 - (uint16_t) adc_buffer[5]) / Avg_Slope + 25);
	return temperature;
}
uint32_t analogtan_veriye(uint32_t kanal, uint32_t Vrefint) {
	return (Vrefint * kanal) / 4095.0;
}

void pilleri_olc(void) {
	for (uint8_t i = 0; i < 4; ++i) {
		pil[i] = (uint16_t) analogtan_veriye(adc_buffer[i], vref());
	}
}
void lcdGoster(void) {
	if (adc_ready != true) {

	}
	adc_ready = false;

	pilleri_olc();
	sprintf(giden_data, "%d%d%d%d", pil[0], pil[1], pil[2], pil[3]);
	LCD16X2_SetCursor(0, 0);
	LCD16X2_PrintStr(giden_data);

	sprintf(giden_data, "temp:%d, %ld", tempMCU(), vref());
	LCD16X2_SetCursor(0, 1);
	LCD16X2_PrintStr(giden_data);

	HAL_ADC_Start_DMA(&hadc1, adc_buffer, ADC_BUFFER_SIZE);
}

void prescalerVeCounterBul(uint32_t sayi) {
	int8_t basamak = 0, bolum = 0, sifirSayisi = -1;
	uint32_t sayi1 = sayi;
	while (sayi1 > 0) {
		sayi1 = sayi1 / 10;
		basamak++;
	}

	/*sprintf(giden_data, "basamak Sayisi:%d\n", basamak);
	 Uart_sendstring(giden_data);*/

	uint32_t sayi2 = sayi;
	while (bolum == 0) {
		bolum = sayi2 % 10;
		sayi2 = sayi2 / 10;
		sifirSayisi++;

	}
	/*sprintf(giden_data, "Sifir Sayisi:%d\n", sifirSayisi);
	 Uart_sendstring(giden_data);*/

	//int r =0;
	//r=pow(10,sifirSayisi);
	prescalerDeger = (sayi / pow(10, sifirSayisi)) - 1;
	counterPeriod = pow(10, sifirSayisi) - 1;

	/*sprintf(giden_data, "prc: %ld, cnt: %ld\n", prescalerDeger, counterPeriod);
	 Uart_sendstring(giden_data);*/

}

void setPWMFrequencyAndDutyCycle(TIM_HandleTypeDef *htim, uint8_t kanal,
		uint32_t frequency, uint8_t dutyCycle) {
	//uint32_t prescalerValue = 0;
	//prescalerValue = ((HAL_RCC_GetSysClockFreq() / (frequency * (htim->Init.Period+1)))-1);
	//prsc = prescalerValue;
	if (dutyCycle > 0) {
		prescalerVeCounterBul(72000000 / frequency);
		switch (kanal) {
		case 1:
			HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_1);
			HAL_TIMEx_PWMN_Stop(htim, TIM_CHANNEL_1);
			break;
		case 2:
			HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_2);
			HAL_TIMEx_PWMN_Stop(htim, TIM_CHANNEL_2);
			break;
		case 3:
			HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_3);
			HAL_TIMEx_PWMN_Stop(htim, TIM_CHANNEL_3);
			break;

		}
		// Timer'ı durdur

		// Prescaler ve Period değerlerini güncelle
		htim->Instance->PSC = prescalerDeger;	//prescalerValue;
		htim->Instance->ARR = counterPeriod;//(HAL_RCC_GetSysClockFreq() / (prescalerValue + 1)) / frequency - 1;
		// Duty cycle'ı güncelle
		TIM_OC_InitTypeDef sConfigOC = { 0 };
		sConfigOC.OCMode = TIM_OCMODE_PWM1;
		sConfigOC.Pulse = (dutyCycle * (counterPeriod + 1)) / 100;
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

		// PWM kanal konfigürasyonunu güncelle
		if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1)
				!= HAL_OK) {
			Error_Handler();
		}
		if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2)
				!= HAL_OK) {
			Error_Handler();
		}
		if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3)
				!= HAL_OK) {
			Error_Handler();
		}

		// Timer'ı tekrar başlat

		switch (kanal) {
		case 1:
			HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
			HAL_TIMEx_PWMN_Start(htim, TIM_CHANNEL_1);
			break;
		case 2:
			HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2);
			HAL_TIMEx_PWMN_Start(htim, TIM_CHANNEL_2);
			break;
		case 3:
			HAL_TIM_PWM_Start(htim, TIM_CHANNEL_3);
			HAL_TIMEx_PWMN_Start(htim, TIM_CHANNEL_3);
			break;

		}

		frekans = (72000000 / (prescalerDeger + 1) * (counterPeriod + 1))
				/ 10000;
		duty = (dutyCycle * (counterPeriod + 1)) / 100;
	}

}

void hesaplaAdcDegerleri(int adcDegerleri[], int boyut) {
	max = adcDegerleri[0];
	min = adcDegerleri[0];
	maxIndex = 0;
	minIndex = 0;
	uint32_t toplam = 0;

	for (int i = 0; i < boyut; i++) {
		if (adcDegerleri[i] > max) {
			max = adcDegerleri[i];
			maxIndex = i;
		}
		if (adcDegerleri[i] < min) {
			min = adcDegerleri[i];
			minIndex = i;
		}
		toplam += adcDegerleri[i];
	}

	ortalama = toplam / boyut;

	/*
	 printf("Maksimum ADC degeri: %d, Indeksi: %d\n", max, maxIndex);
	 printf("Minimum ADC degeri: %d, Indeksi: %d\n", min, minIndex);
	 printf("Ortalama ADC degeri: %d\n", ortalama);*/
}

uint8_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min,
		uint16_t out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void pwmSecimi(void) {
	for (uint8_t i = 0; i < 3; ++i) {
		voltFark[i] = (voltFark[i] / 10) * 10;
	}

	max = 0;
	*voltFark = 0;

	for (uint8_t i = 0; i < 3; i++) {
		if (pil[i] > pil[i + 1]) {
			voltFark[i] = pil[i] - pil[i + 1];
		} else {
			voltFark[i] = 0;
		}

		if (voltFark[i] > max) {
			max = voltFark[i];

		}
		voltFark[i] = (voltFark[i] / 10) * 10;
		/*printf("%d.volt= %d\n",i,voltFark[i]);
		 printf("%d.fark= %d\n",i,voltFark[i]);
		 printf("max= %d\n",maxV);*/

	}

	max = (max / 10) * 10;

	for (uint8_t i = 0; i < 3; i++) {
		if (voltFark[i] > voltajFark) {
			hesaplananPwm[i] = map(voltFark[i], 0, max, 0, MAX_PWM_DUTY_CYCLE);
		} else {
			hesaplananPwm[i] = 0;
		}

	}
	/*for (uint8_t i = 0; i < 3; i++) {
	 sprintf(giden_data, "%d-%d.Farki = %d,Pwm:%d\n", i, i + 1, voltFark[i],
	 hesaplananPwm[i]);
	 Uart_sendstring(giden_data);

	 }*/
}
void limitKontrol(void) {
	if ((pil[0] > azamiVoltaj) && (pil[1] > azamiVoltaj)
			&& (pil[2] > azamiVoltaj) && (pil[3] > azamiVoltaj)) {
		azamiDurum = false;
		HAL_GPIO_WritePin(kaynak_bagla_GPIO_Port, kaynak_bagla_Pin, RESET);
		HAL_GPIO_WritePin(yuk_bagla_GPIO_Port, yuk_bagla_Pin, RESET);
	}

	if ((pil[0] < asgariVoltaj) && (pil[1] < asgariVoltaj)
			&& (pil[2] < asgariVoltaj) && (pil[3] < asgariVoltaj)) {
		asgariDurum = false;
		HAL_GPIO_WritePin(kaynak_bagla_GPIO_Port, kaynak_bagla_Pin, RESET);
		HAL_GPIO_WritePin(yuk_bagla_GPIO_Port, yuk_bagla_Pin, RESET);
	}

}

void balansTest(void) {
	/*for(uint i=0;i>4;i++){
	 if((abs(adcHamDeger[i]-adcHamDeger[i+1]))>voltajFark){
	 setPWMFrequencyAndDutyCycle(&htim1, 1, 100000, 10);
	 }
	 }*/
	//pwmSecimi();
	if (azamiDurum && asgariDurum && balansDurum) {	//balansDurum){//
		if (hesaplananPwm[0] != 0 || hesaplananPwm[1] != 0
				|| hesaplananPwm[2] != 0) {
			static uint8_t sayac = 0;

			sayac++;
			if (sayac > 2)
				sayac = 0;

//		if ((sayac == 0) && (hesaplananPwm[0]>0)) {
//		 HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2|TIM_CHANNEL_3);
//		 HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2|TIM_CHANNEL_3);
//		 /*HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
//		 HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);*/
//		 } else if ((sayac == 1) && (hesaplananPwm[1]>0)) {
//		 HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1|TIM_CHANNEL_3);
//		 HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1|TIM_CHANNEL_3);
//		 /*HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
//		 HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);*/
//
//		 } else if ((sayac == 2) && (hesaplananPwm[2]>0)) {
//		 HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1|TIM_CHANNEL_2);
//		 HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1|TIM_CHANNEL_2);
//		 /*HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
//		 HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);*/
//		 }

			switch (sayac) {
			case 0:

				if (hesaplananPwm[2] > 0) {
					HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
					HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
				}
				if (hesaplananPwm[1] > 0) {
					HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
					HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
				}
				if (hesaplananPwm[0] > 0) {

					setPWMFrequencyAndDutyCycle(&htim1, 1, 100000,
							hesaplananPwm[0]);
				}
				break;
				/*case 1:
				 if(hesaplananPwm[0]>0){
				 HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
				 HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
				 }


				 break;*/
			case 1:

				if (hesaplananPwm[0] > 0) {
					HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
					HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
				}
				if (hesaplananPwm[2] > 0) {
					HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
					HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
				}
				if (hesaplananPwm[1] > 0) {
					setPWMFrequencyAndDutyCycle(&htim1, 2, 100000,
							hesaplananPwm[1]);
				}
				break;
				/*case 3:
				 if(hesaplananPwm[1]>0){
				 HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
				 HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
				 }
				 break;*/
			case 2:

				if (hesaplananPwm[1] > 0) {
					HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
					HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
				}
				if (hesaplananPwm[0] > 0) {
					HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
					HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
				}
				if (hesaplananPwm[2] > 0) {
					setPWMFrequencyAndDutyCycle(&htim1, 3, 100000,
							hesaplananPwm[2]);
				}
				break;
				/*case 5:
				 if(hesaplananPwm[2]>0){
				 HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
				 HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
				 }
				 break;*/
			}

		}
	}

	/*if (adcHamDeger[4] > 100 * 30) {//azami akim
	 HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	 HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
	 HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
	 HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
	 HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
	 HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
	 }*/

	/*for(uint i=0;i>4;i++){
	 if(adcHamDeger[i]>azamiVoltaj){

	 }
	 }*/

}
void send_Lcd(void) {

	if (adc_ready != true) {

	}
	adc_ready = false;

	pilleri_olc();

	static uint8_t cnt = 0;
	cnt++;
	LCD16X2_Clear();

	if (cnt == 1) {
		sprintf(giden_data, "B1:%d", pil[3]);
		LCD16X2_SetCursor(0, 0);
		LCD16X2_PrintStr(giden_data);

		sprintf(giden_data, "B2:%d", pil[2]);
		LCD16X2_SetCursor(0, 1);
		LCD16X2_PrintStr(giden_data);
	} else if (cnt == 2) {
		sprintf(giden_data, "B3:%d", pil[1]);
		LCD16X2_SetCursor(0, 0);
		LCD16X2_PrintStr(giden_data);

		sprintf(giden_data, "B4:%d", pil[0]);
		LCD16X2_SetCursor(0, 1);
		LCD16X2_PrintStr(giden_data);
	} else if (cnt == 3) {
		sprintf(giden_data, "CR:%ld -> %ld", adc_buffer[4],
				analogtan_veriye(adc_buffer[4], vref()) * 7);
		LCD16X2_SetCursor(0, 0);
		LCD16X2_PrintStr(giden_data);

		sprintf(giden_data, "KD:%d - AA:%d", kisa_devre, asiri_akim);
		LCD16X2_SetCursor(0, 1);
		LCD16X2_PrintStr(giden_data);

		/*sprintf(giden_data, "LM:%04ld -> %04d", adcHamDeger[5],
		 degerDonusum(adcHamDeger[5]));
		 LCD16X2_SetCursor(0, 1);
		 LCD16X2_PrintStr(giden_data);*/
	} else if (cnt == 4) {
		sprintf(giden_data, "Frq:%ld00", frekans);
		LCD16X2_SetCursor(0, 0);
		LCD16X2_PrintStr(giden_data);

		sprintf(giden_data, "Dty:%d0", duty);
		LCD16X2_SetCursor(0, 1);
		LCD16X2_PrintStr(giden_data);
	} else if (cnt == 5) {
		sprintf(giden_data, "Mx:%d - Indx:%d", pil[maxIndex], (4 - maxIndex));
		LCD16X2_SetCursor(0, 0);
		LCD16X2_PrintStr(giden_data);

		sprintf(giden_data, "Mn:%d - Indx:%d", pil[minIndex], (4 - minIndex));
		LCD16X2_SetCursor(0, 1);
		LCD16X2_PrintStr(giden_data);
	}

	else {
		/*float Vref = 0.0;
		 int dgr = 0;
		 Vref = (1.20 / adcHamDeger[6]) * 4095.0;
		 dgr = Vref * 1000;*/

		sprintf(giden_data, "Vr:%ld -> %ld", adc_buffer[6], vref());
		LCD16X2_SetCursor(0, 0);
		LCD16X2_PrintStr(giden_data);

		sprintf(giden_data, "TP:%ld -> %d%cC", adc_buffer[5], tempMCU(), 0xDF);
		LCD16X2_SetCursor(0, 1);
		LCD16X2_PrintStr(giden_data);

		/*sprintf(giden_data,"Vref:%04d",dgr);
		 LCD16X2_SetCursor(0,1);
		 LCD16X2_PrintStr(giden_data);*/
		cnt = 0;
	}
	/*static uint8_t cnt = 0;
	 cnt++;
	 LCD16X2_Clear();
	 sprintf(giden_data, "Sayar: %d", cnt);
	 LCD16X2_SetCursor(0, 0);
	 LCD16X2_PrintStr(giden_data);

	 sprintf(giden_data, "sayar:%d", cnt);
	 LCD16X2_SetCursor(6, 1);
	 LCD16X2_PrintStr(giden_data);*/

	HAL_ADC_Start_DMA(&hadc1, adc_buffer, ADC_BUFFER_SIZE);
}

void send_Serial(void) {

	sprintf(giden_data, "V_Bat_4 = %d\n", pil[0]);
	Uart_sendstring(giden_data);

	sprintf(giden_data, "V_Bat_3 = %d\n", pil[1]);
	Uart_sendstring(giden_data);

	sprintf(giden_data, "V_Bat_2 = %d\n", pil[2]);
	Uart_sendstring(giden_data);

	sprintf(giden_data, "V_Bat_1 = %d\n", pil[3]);
	Uart_sendstring(giden_data);

	sprintf(giden_data, "A_Pack = %ld -> %ld\n", adc_buffer[4],
			analogtan_veriye(adc_buffer[4], vref()) * 7);
	Uart_sendstring(giden_data);

	sprintf(giden_data, "KisaDevre:%d\nAsiriAkim:%d\n", kisa_devre, asiri_akim);
	Uart_sendstring(giden_data);

	sprintf(giden_data, "Temp = %d*C\n", tempMCU());
	Uart_sendstring(giden_data);
	//(azamiDurum && asgariDurum && balansDurum)

	sprintf(giden_data, "Vref = %ld\n______\n", vref());
	Uart_sendstring(giden_data);
	//pwmSecimi();

	//if (balansDurum) {
	for (uint8_t i = 0; i < 3; i++) {
		sprintf(giden_data, "%d-%d.Farki = %d,Pwm:%d\n", i, i + 1, voltFark[i],
				hesaplananPwm[i]);
		Uart_sendstring(giden_data);

	}
	sprintf(giden_data, "max: %d\n", max);
	Uart_sendstring(giden_data);

	sprintf(giden_data, "azami:%d\nasgari:%d\nbalans:%d\n", azamiDurum,
			asgariDurum, balansDurum);
	Uart_sendstring(giden_data);

	sprintf(giden_data, "______\n");
	Uart_sendstring(giden_data);
	//}
	/*static uint8_t cnt = 0;
	 cnt++;

	 sprintf(giden_data, "Sayar:%d\n", cnt);
	 Uart_sendstring(giden_data);*/

	/*

	 sprintf(giden_data, "V_Bat_4 = %d -> %d\n", adcHamDeger[0],
	 degerDonusum(adcHamDeger[0]));
	 Uart_sendstring(giden_data);

	 sprintf(giden_data, "V_Bat_3 = %d -> %d\n", adcHamDeger[1],
	 degerDonusum(adcHamDeger[1]));
	 Uart_sendstring(giden_data);

	 sprintf(giden_data, "V_Bat_2 = %d -> %d\n", adcHamDeger[2],
	 degerDonusum(adcHamDeger[2]));
	 Uart_sendstring(giden_data);

	 sprintf(giden_data, "V_Bat_1 = %d -> %d\n", adcHamDeger[3],
	 degerDonusum(adcHamDeger[3]));
	 Uart_sendstring(giden_data);

	 sprintf(giden_data, "A_Pack = %d -> %d\n", adcHamDeger[4],
	 degerDonusum(adcHamDeger[4]));
	 Uart_sendstring(giden_data);

	 sprintf(giden_data, "KisaDevre:%d\nAsiriAkim:%d\n", kisa_devre,asiri_akim);
	 Uart_sendstring(giden_data);*/

	/*sprintf(giden_data, "V_Pack = %04ld -> %04d\n", adcHamDeger[5],
	 degerDonusum(adcHamDeger[5]));
	 Uart_sendstring(giden_data);*/

	/*sprintf(giden_data, "VCC = %04ld -> %04d\n", adcHamDeger[6],degerDonusum(adcHamDeger[6]));
	 Uart_sendstring(giden_data);
	 */
	/*sprintf(giden_data, "Temp = %d -> %d*C\n", adcHamDeger[5],
	 tempMCU());
	 Uart_sendstring(giden_data);

	 sprintf(giden_data, "Vref = %d -> %d\n", adcHamDeger[6], vref());
	 Uart_sendstring(giden_data);

	 sprintf(giden_data, "Maksimum: %d, Indeksi: %d\n", max, (4-maxIndex));
	 Uart_sendstring(giden_data);

	 sprintf(giden_data, "Minimum: %d, Indeksi: %d\n", min, (4-minIndex));
	 Uart_sendstring(giden_data);

	 sprintf(giden_data, "Ortalama: %04ld\n______\n", ortalama);
	 HAL_ADC_Start_DMA(&hadc1, adc_buffer, ADC_BUFFER_SIZE);
	 Uart_sendstring(giden_data);*/
}


void PwmVeOnOffKontrol(void) {

	if (Uart_peek() == '1')
		HAL_GPIO_TogglePin(kaynak_kontrol_GPIO_Port, kaynak_kontrol_Pin);//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);//Enable
	//if (Uart_peek () == '2') HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

	if (Uart_peek() == '2')
		HAL_GPIO_TogglePin(yuk_bagla_GPIO_Port, yuk_bagla_Pin);	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);//DisCharge
	//if (Uart_peek () == '4') HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);

	if (Uart_peek() == '3')
		HAL_GPIO_TogglePin(kaynak_bagla_GPIO_Port, kaynak_bagla_Pin);//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);//Charge
	//if (Uart_peek () == '6') HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

	if (Uart_peek() == '4') {
		static int t1 = 0;
		t1++;
		if (t1 == 1) {
			setPWMFrequencyAndDutyCycle(&htim1, 1, 100000, 30);
		} else {
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
			HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
			t1 = 0;
		}
	}

	if (Uart_peek() == '5') {
		static int t2 = 0;
		t2++;
		if (t2 == 1) {
			setPWMFrequencyAndDutyCycle(&htim1, 2, 100000, 40);
		} else {
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
			HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
			t2 = 0;
		}
	}

	if (Uart_peek() == '6') {
		static int t3 = 0;
		t3++;
		if (t3 == 1) {
			setPWMFrequencyAndDutyCycle(&htim1, 3, 100000, 50);
		} else {
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
			HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
			t3 = 0;
		}
	}
	if (Uart_peek() == '7')
		balansDurum = !balansDurum;
	if (Uart_peek() == '8')
		asgariDurum = !asgariDurum;
	if (Uart_peek() == '9')
		azamiDurum = !azamiDurum;

	Uart_flush();
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*int _write(int file, char *ptr, int len){
 HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
 return len;
 }*/
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
	MX_I2C1_Init();
	MX_TIM1_Init();
	MX_USART1_UART_Init();
	MX_TIM3_Init();
	/* USER CODE BEGIN 2 */
	LCD16X2_Init(2);
	Ringbuf_init();
	HAL_ADCEx_Calibration_Start(&hadc1);
	HAL_ADC_Start_DMA(&hadc1, adc_buffer, ADC_BUFFER_SIZE);
	HAL_TIM_Base_Start(&htim3);
	/* USER CODE END 2 */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of KontrolTask */
	osThreadDef(KontrolTask, StartKontrolTask, osPriorityNormal, 0, 128);
	KontrolTaskHandle = osThreadCreate(osThread(KontrolTask), NULL);

	/* definition and creation of AdcTask */
	osThreadDef(AdcTask, StartAdcTask, osPriorityIdle, 0, 128);
	AdcTaskHandle = osThreadCreate(osThread(AdcTask), NULL);

	/* definition and creation of BalansTask */
	osThreadDef(BalansTask, StartBalansTask, osPriorityIdle, 0, 128);
	BalansTaskHandle = osThreadCreate(osThread(BalansTask), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Common config
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 7;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_2;
	sConfig.Rank = ADC_REGULAR_RANK_2;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_3;
	sConfig.Rank = ADC_REGULAR_RANK_3;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = ADC_REGULAR_RANK_4;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_5;
	sConfig.Rank = ADC_REGULAR_RANK_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
	sConfig.Rank = ADC_REGULAR_RANK_6;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_VREFINT;
	sConfig.Rank = ADC_REGULAR_RANK_7;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 400000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 71;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 999;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 499;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3)
			!= HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	HAL_TIM_MspPostInit(&htim1);

}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {

	/* USER CODE BEGIN TIM3_Init 0 */

	/* USER CODE END TIM3_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 71;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 999;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Channel1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA,
	kaynak_kontrol_Pin | yuk_bagla_Pin | kaynak_bagla_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : LED_Pin */
	GPIO_InitStruct.Pin = LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : kaynak_kontrol_Pin yuk_bagla_Pin kaynak_bagla_Pin */
	GPIO_InitStruct.Pin = kaynak_kontrol_Pin | yuk_bagla_Pin | kaynak_bagla_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : kisa_d_algila_Pin asiri_a_algila_Pin */
	GPIO_InitStruct.Pin = kisa_d_algila_Pin | asiri_a_algila_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	UNUSED(hadc);
	adc_ready = true;
	HAL_ADC_Stop_DMA(&hadc1);
	/*if (hadc->Instance == ADC1) {
	 for (int a = 0; a < 7; a++) {
	 adcHamDeger[a] = adcDmaDeger[a];
	 }

	 }
	 adc_tamam = true;*/
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartKontrolTask */
/**
 * @brief  Function implementing the KontrolTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartKontrolTask */
void StartKontrolTask(void const *argument) {
	/* USER CODE BEGIN 5 */
	/* Infinite loop */
	for (;;) {

		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		PwmVeOnOffKontrol();
		limitKontrol();

		//PwmVeOnOffKontrol();
		osDelay(200);
	}
	/* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartAdcTask */
/**
 * @brief Function implementing the AdcTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartAdcTask */
void StartAdcTask(void const *argument) {
	/* USER CODE BEGIN StartAdcTask */
	/* Infinite loop */
	for (;;) {
		//hesaplaAdcDegerleri((uint32_t *)*pil,4);
		//lcdGoster();
		hesaplaAdcDegerleri(pil, 4);
		pwmSecimi();
		send_Lcd();
		send_Serial();
		//lcdGoster();
		osDelay(2000);
	}
	/* USER CODE END StartAdcTask */
}

/* USER CODE BEGIN Header_StartBalansTask */
/**
 * @brief Function implementing the BalansTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartBalansTask */
void StartBalansTask(void const *argument) {
	/* USER CODE BEGIN StartBalansTask */
	/* Infinite loop */
	for (;;) {
		balansTest();
		osDelay(200);
	}
	/* USER CODE END StartBalansTask */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM2 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM2) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
