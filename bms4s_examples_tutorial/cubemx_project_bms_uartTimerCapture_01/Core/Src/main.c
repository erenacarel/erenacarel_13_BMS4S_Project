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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
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

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
#define _USR_ADC_BUF_SIZE 5
#define _USR_ADC_SAMPLE_COUNT 16
#define _USR_ADC_CHANNEL_1 0
#define _USR_ADC_CHANNEL_2 1
#define _USR_ADC_CHANNEL_3 2
#define _USR_ADC_CHANNEL_4 3
#define _USR_VREF_CHANNEL 4

uint8_t g_pwmCounter;
uint32_t g_timeoutMs;
uint8_t g_u8count;
uint8_t g_u8adcSampleCounter;
bool g_adcFinishedFlg;
bool g_adcConvFlg;
uint32_t channel1 = 0;
uint32_t channel2 = 0;
uint32_t channel3 = 0;
uint32_t channel4 = 0;
uint32_t channel5 = 0;
uint32_t g_adcDmaBuffer[_USR_ADC_BUF_SIZE];
uint16_t Vref, vdda;
uint16_t readValue1, readValue2, readValue3, readValue4, readValue5;
uint16_t pil1, pil2, pil3, pil4;
static volatile uint32_t g_adcRawValueBuf[_USR_ADC_BUF_SIZE][_USR_ADC_SAMPLE_COUNT];
char buffer[20];

ADC_ChannelConfTypeDef sConfigPrivate = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */
void UsrMyDelay(uint32_t timeoutMs);
void UsrAdcGetValue(void);
void UsrAdcCallback(void);
void UsrReadAdc(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*
	TIM_CLCOK = APB_TIM_CLOCK / PRESCALAR    72000000 / 35 = 2 MHz
	FREQUENCY = TIM_CLOCK / ARR              2000000 / 100 = 20 KHz
	DUTY = (CCRx / ARR) x 100                30 / 100 x100 = 20
*/

void UsrReadAdc(void)
{
	sConfigPrivate.Rank = ADC_REGULAR_RANK_1;
	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_1;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	readValue1 = HAL_ADC_GetValue(&hadc1);
	sprintf(buffer, "adc1: %d\n", readValue1);
	HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
	memset(buffer, 0, 20);
	HAL_ADC_Stop(&hadc1);

	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_2;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	readValue2 = HAL_ADC_GetValue(&hadc1);
	sprintf(buffer, "adc2: %d\n", readValue2);
	HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
	memset(buffer, 0, 20);
	HAL_ADC_Stop(&hadc1);

	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_3;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	readValue3 = HAL_ADC_GetValue(&hadc1);
	sprintf(buffer, "adc3: %d\n", readValue3);
	HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
	memset(buffer, 0, 20);
	HAL_ADC_Stop(&hadc1);

	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_4;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	readValue4 = HAL_ADC_GetValue(&hadc1);
	sprintf(buffer, "adc4: %d\n", readValue4);
	HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
	memset(buffer, 0, 20);
	HAL_ADC_Stop(&hadc1);

	sConfigPrivate.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
	sConfigPrivate.Channel = ADC_CHANNEL_VREFINT;
	HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	readValue5 = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
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
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADCEx_Calibration_Start(&hadc1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  UsrReadAdc();
	  pil1 = (readValue1/4096.0)*1000;
	  pil2 = (readValue2/4096.0)*1000;
	  pil3 = (readValue3/4096.0)*1000;
	  pil4 = (readValue4/4096.0)*1000;
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

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 3;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 89;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 26;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 0;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 200;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7199;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

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
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void UsrMyDelay(uint32_t timeoutMs)
{
	while(timeoutMs - g_timeoutMs > 0);
	g_timeoutMs = 0;
}

//void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	// dusen kenar yakalandiginda complementary pwm baslat
//	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	// periyod doldugunda complementary pwm durdur

}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	g_pwmCounter++;

	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

}


void UsrAdcGetValue(void)
{
	g_adcFinishedFlg = false;
	if (HAL_OK == HAL_ADC_Start_DMA(&hadc1, g_adcDmaBuffer, _USR_ADC_BUF_SIZE))
	{
		while(!g_adcFinishedFlg)
		;

		for (uint8_t i = 0; i < _USR_ADC_SAMPLE_COUNT; i++)
		{
			channel1 += g_adcRawValueBuf[_USR_ADC_CHANNEL_1][i];
			channel2 += g_adcRawValueBuf[_USR_ADC_CHANNEL_2][i];
			channel3 += g_adcRawValueBuf[_USR_ADC_CHANNEL_3][i];
			channel4 += g_adcRawValueBuf[_USR_ADC_CHANNEL_4][i];
			channel5 += g_adcRawValueBuf[_USR_VREF_CHANNEL][i];
		}
		channel1 /= _USR_ADC_SAMPLE_COUNT;
		channel2 /= _USR_ADC_SAMPLE_COUNT;
		channel3 /= _USR_ADC_SAMPLE_COUNT;
		channel4 /= _USR_ADC_SAMPLE_COUNT;
		channel5 /= _USR_ADC_SAMPLE_COUNT;
	}
	else
	{
		HAL_ADC_Stop_DMA(&hadc1);
	}

}


void UsrAdcCallback(void)
{
	for (uint8_t i = 0; i < _USR_ADC_BUF_SIZE; i++)
	{
		g_adcRawValueBuf[i][g_u8adcSampleCounter] = g_adcDmaBuffer[i];
	}
	g_u8adcSampleCounter++;
	if (g_u8adcSampleCounter >= _USR_ADC_SAMPLE_COUNT)
	{
		HAL_ADC_Stop_DMA(&hadc1);
		g_u8adcSampleCounter = 0;
		g_adcFinishedFlg = true;
	}


}

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
