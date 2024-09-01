/*
 * usr_uart.c
 *
 *  Created on: Aug 6, 2024
 *      Author: eacarel
 */


#include "usr_uart.h"


uint8_t g_rxTempBuf[1];
uint8_t g_rxBuf[40];
uint8_t g_txBuf[40];
uint16_t g_rxIndex;
bool g_rxIntFlg;


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		g_rxBuf[g_rxIndex++] = g_rxTempBuf[0];
		if(g_rxIndex >= 40)
			g_rxIndex = 0;
		g_rxIntFlg = true;
		HAL_UART_Receive_IT(&huart1, (uint8_t*)g_rxTempBuf, 1);
	}
}
