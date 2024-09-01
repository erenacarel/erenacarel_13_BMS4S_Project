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
	if (((g_batteryValuesModified[0] >= 35) && (g_batteryValuesModified[0] < 45)) \
			&& ((g_batteryValuesModified[1] >= 35) && (g_batteryValuesModified[1] < 35)) \
			&& ((g_batteryValuesModified[2] >= 35) && (45> g_batteryValuesModified[3] >= 35)))
	{
		UsrReadAdc();
		HAL_GPIO_WritePin(CHARGE_GPIO_Port, CHARGE_Pin, GPIO_PIN_SET);
		UsrCheckBat0Bat1();
		UsrCheckBat1Bat2();
		UsrCheckBat2Bat3();
	}
	if ((g_batteryValuesModified[0] < 35) && (g_batteryValuesModified[1] < 35) && (g_batteryValuesModified[2] < 35) && (g_batteryValuesModified[3] < 35))
	{
		UsrReadAdc();
		HAL_GPIO_WritePin(DECHARGE_GPIO_Port, DECHARGE_Pin, GPIO_PIN_RESET);
	}


}



void UsrBatteryOnOffControl(void)
{

}






