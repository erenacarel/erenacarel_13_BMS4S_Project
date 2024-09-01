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
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
ADC_ChannelConfTypeDef sConfigPrivate = {0};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define FILTER_SIZE 5
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t autoReload = 60;
uint16_t prescaler = 12;

bool g_adcFinishedFlg;
bool g_adcConvFlg;

uint16_t readValue[5];
uint16_t pil[4];

uint16_t adcBuffer[4][FILTER_SIZE] = {0};
uint8_t bufferIndex = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void UsrInitPwm(void);
void UsrTim2Init(void);
void UsrInitGpio(void);
void UsrReadAdc(void);
void UsrTrigPwm1Channel1(void);
void UsrTrigSwapPwm1Channel1(void);
void UsrTrigPwm1Channel2(void);
void UsrTrigSwapPwm1Channel2(void);
void UsrTrigPwm1Channel3(void);
void UsrTrigSwapPwm1Channel3(void);
void UsrCheckBat1Bat2(void);
void UsrCheckBat2Bat3(void);
void UsrCheckBat3Bat4(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t MovingAverage(uint16_t* buffer, uint8_t size)
{
    uint32_t sum = 0;
    for(uint8_t i = 0; i < size; i++)
    {
        sum += buffer[i];
    }
    return (uint16_t)(sum / size);
}

void UsrReadAdc(void)
{
	sConfigPrivate.Rank = ADC_REGULAR_RANK_1;
	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_1;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	readValue[0] = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_2;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	readValue[1] = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_3;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	readValue[2] = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_4;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	readValue[3] = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_VREFINT;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	readValue[4] = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

//	pil[0] = (readValue[0] / 4096.0) * 1000;
//	pil[1] = (readValue[1] / 4096.0) * 1000;
//	pil[2] = (readValue[2] / 4096.0) * 1000;
//	pil[3] = (readValue[3] / 4096.0) * 1000;

    for (uint8_t i = 0; i < 4; i++)
    {
        adcBuffer[i][bufferIndex] = readValue[i];
        pil[i] = MovingAverage(adcBuffer[i], FILTER_SIZE);
    }

    bufferIndex = (bufferIndex + 1) % FILTER_SIZE;
}

void UsrInitGpio(void)
{
//	GPIOA->CRL &= (uint32_t)0x00;
//	GPIOB->CRL &= (uint32_t)0x00;

//	GPIOA->CRH &= (uint32_t)0x0000;
	GPIOA->CRH |= (uint32_t)0x0BBB;

//	GPIOB->CRH &= (uint32_t)0x00;
	GPIOB->CRH |= (uint32_t)0xBBB00000;
}

void UsrInitPwm(void)
{
    // Zamanlayıcı ayarları
	TIM1->PSC = prescaler - 1;
	TIM1->ARR = autoReload - 1;  // Auto-reload register

    // PWM mode 1 (upcounting, TIMx_CCRx < TIMx_CNT) oc1 and oc2
	TIM1->CCMR1 |= (uint32_t)0x6464;  // pwm mode 1 and output compare enable

	// Capture/compare register  output compare fast enable
	TIM1->CCMR2 |= (uint32_t)0x006C;

    // Break and dead-time register (BDTR)
	TIM1->BDTR |= (uint32_t)0x8800;  // MOE enable and OSSI disable

	TIM1->EGR |= (uint32_t)0x01; // update generation

    // Counter enable
	TIM1->CR1 |= (uint32_t)0x01;  // Counter enable
}

void UsrTim2Init(void)
{
	TIM2->PSC = 719 - 1;
	TIM2->ARR = 100 - 1;

	TIM2->SMCR |= (uint32_t)0x56; // bit[6:4] 101: Filtered Timer Input 1 (TI1FP1)  and  bit[2:0] 10: Trigger mode

	TIM2->DIER |= (uint32_t)0x01;  // update interrupt enable

	NVIC_EnableIRQ(TIM2_IRQn);
	TIM2->CR1 |= (uint32_t)0x01;  // counter enable
}

void UsrTrigPwm1Channel1(void)
{
	// Capture/compare register
	TIM1->CCR1 = (autoReload) * (0.6);  // Duty cycle
	TIM1->CCER |= (uint32_t)0x5;  // TIM1_CH1 output enable and TIM1_CH1N complementary output enable
	TIM1->CCER &= ~(uint32_t)0xA;  // polarity high
}


void UsrTrigSwapPwm1Channel1(void)
{
	TIM1->CCR1 = (autoReload) * (0.6);
	TIM1->CCER |= (uint32_t)0x5;  // TIM1_CH1 output enable and TIM1_CH1N complementary output enable
	TIM1->CCER |= (uint32_t)0xA;  // polarity low
}


void UsrTrigPwm1Channel2(void)
{
	TIM1->CCR2 = (autoReload) * (0.7);
	TIM1->CCER |= (uint32_t)0x50;  // TIM1_CH2 output enable and TIM1_CH2N complementary output enable
	TIM1->CCER &= ~(uint32_t)0xA0;  // polarity high
}


void UsrTrigSwapPwm1Channel2(void)
{
	TIM1->CCR2 = (autoReload) * (0.7);
	TIM1->CCER |= (uint32_t)0x50;  // TIM1_CH2 output enable and TIM1_CH2N complementary output enable
	TIM1->CCER |= (uint32_t)0xA0;  // polarity low
}


void UsrTrigPwm1Channel3(void)
{
	TIM1->CCR3 = (autoReload) * (0.8);
	TIM1->CCER |= (uint32_t)0x500;  // TIM1_CH3 output enable and TIM1_CH3N complementary output enable
	TIM1->CCER &= ~(uint32_t)0xA00;  // polarity high
}


void UsrTrigSwapPwm1Channel3(void)
{
	TIM1->CCR3 = (autoReload) * (0.8);
	TIM1->CCER |= (uint32_t)0x500;  // TIM1_CH3 output enable and TIM1_CH3N complementary output enable
	TIM1->CCER |= (uint32_t)0xA00;  // polarity low
}

void UsrCheckBat1Bat2(void)
{
	if (pil[0] > pil[1])
	{
		UsrTrigSwapPwm1Channel1();
	}
	else
		UsrTrigPwm1Channel1();
}


void UsrCheckBat2Bat3(void)
{
	if(pil[1] > pil[2])
	{
		UsrTrigSwapPwm1Channel2();
	}
	else
		UsrTrigPwm1Channel2();
}


void UsrCheckBat3Bat4(void)
{
	if (pil[2] > pil[3])
	{
		UsrTrigSwapPwm1Channel3();
	}
	else
		UsrTrigPwm1Channel3();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  UsrInitGpio();
  UsrTim2Init();
  UsrInitPwm();

  HAL_ADCEx_Calibration_Start(&hadc1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  UsrReadAdc();
//	  UsrCheckBat1Bat2();
//	  UsrCheckBat2Bat3();
//	  UsrCheckBat3Bat4();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
