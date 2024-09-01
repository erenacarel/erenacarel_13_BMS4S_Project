/*
 * usr_system.c
 *
 *  Created on: Aug 6, 2024
 *      Author: eacarel
 */

#include "usr_system.h"

uint16_t autoReload = 60;
uint16_t prescaler = 12;
uint32_t g_timerCount = 0;
uint16_t g_dutyCycle = _USR_PWM1_DUTY;

char m_batteryBufferUpLine[16];
char m_batteryBufferDownLine[16];

void UsrSystemInitial(void)
{
	uint8_t tryCount = 0;

	HAL_UART_Receive_IT(&huart1, (uint8_t*)g_rxTempBuf, 1);

	UsrInitGpio();
	UsrTim2Init();
	UsrInitPwm();

	HAL_ADCEx_Calibration_Start(&hadc1);

	UsrI2CScan();
	if (UsrLcdInit())
	{
	  UsrLcdClearDisplay();
	  UsrLcdSetCursor(0, 0);
	  sprintf(g_sendStringUpLine, "ASSAN AR-GE");
	  UsrLcdSendString(g_sendStringUpLine);
	  HAL_Delay(5000);
	}
	UsrAllChannelLowLevel();

	HAL_UART_Transmit(&huart1, (uint8_t*)g_txBuf, sprintf((char*)g_txBuf, "While Loop was started\n"), 100);
	UsrLcdClearDisplay();
	HAL_GPIO_WritePin(CHARGE_GPIO_Port, CHARGE_Pin, GPIO_PIN_SET);  // Q9 mosfeti

	// adc degerleri hemen ölcülmüyor bunun icin biraz surec gecmelidir. onun icin bu mekanizma kullanildi.
start_label:;
	UsrReadAdc();
	if (tryCount++ >= 30)
		goto end_label;
	else
		goto start_label;
end_label:;
}


void UsrSystemGeneral(void)
{
	UsrReadAdc();
	if (((g_batteryValuesModified[0] >= 2500) && (g_batteryValuesModified[0] < 4700)) \
			&& ((g_batteryValuesModified[1] >= 2500) && (g_batteryValuesModified[1] < 4700)) \
			&& ((g_batteryValuesModified[2] >= 2500) && (g_batteryValuesModified[2] < 4700)) \
			&& ((g_batteryValuesModified[3] >= 2500) && (g_batteryValuesModified[3] < 4700)))
	{

//		HAL_GPIO_WritePin(CHARGE_GPIO_Port, CHARGE_Pin, GPIO_PIN_SET);
//		UsrCheckBat0Bat1();
//		UsrCheckBat1Bat2();
		UsrCheckBat2Bat3();
	}
	else if ((g_batteryValuesModified[0] < 2000) || (g_batteryValuesModified[1] < 2000) \
			|| (g_batteryValuesModified[2] < 2000) || (g_batteryValuesModified[3] < 2000))
	{

		HAL_GPIO_WritePin(DECHARGE_GPIO_Port, DECHARGE_Pin, GPIO_PIN_RESET); // GPIO_PIN_SET Q8 mosfeti
	}
	else
	{
//		HAL_GPIO_WritePin(CHARGE_GPIO_Port, CHARGE_Pin, GPIO_PIN_RESET);
//		HAL_GPIO_WritePin(DECHARGE_GPIO_Port, DECHARGE_Pin, GPIO_PIN_RESET);
	}



/*
 * Bu alt satır komple test icindir
 * Bu test icin UsrSystemGeneral() fonksiyonunu yoruma almanız gerekir.
 * Termitte programından 1,2,3 vb. yolladığınızda pwm kanalları ve complimentlei acilacaktir.
 */

//
//  if(g_rxIntFlg)
//  {
//	  if (g_rxTempBuf[0] == '1')
//	  {
//		  UsrTrigPwm1Channel1();
//	  }
//	  else if (g_rxTempBuf[0] == '2')
//	  {
//		  UsrTrigPwm1Channel2();
//	  }
//	  else if (g_rxTempBuf[0] == '3')
//	  {
//		  UsrTrigPwm1Channel3();
//	  }
//	  else if (g_rxTempBuf[0] == '4')
//	  {
//		  UsrTrigSwapPwm1Channel1();
//	  }
//	  else if (g_rxTempBuf[0] == '5')
//	  {
//		  UsrTrigSwapPwm1Channel2();
//	  }
//	  else if (g_rxTempBuf[0] == '6')
//	  {
//		  UsrTrigSwapPwm1Channel3();
//	  }
//	  g_rxIntFlg = false;
//  }



}






