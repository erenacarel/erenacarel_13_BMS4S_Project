/*
 * usr_uart.h
 *
 *  Created on: Aug 6, 2024
 *      Author: eacarel
 */

#ifndef INC_USR_UART_H_
#define INC_USR_UART_H_

#include "usr_general.h"

extern bool g_rxIntFlg;
extern uint8_t g_rxTempBuf[1];
extern uint8_t g_rxBuf[40];
extern uint8_t g_txBuf[40];
extern uint16_t g_rxIndex;
extern bool g_rxIntFlg;

#endif /* INC_USR_UART_H_ */
