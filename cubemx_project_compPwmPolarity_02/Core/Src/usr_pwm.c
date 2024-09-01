/*
 * usr_pwm.c
 *
 *  Created on: Aug 6, 2024
 *      Author: eacarel
 */

#include "usr_pwm.h"

/*
	TIM_CLCOK = APB_TIM_CLOCK / PRESCALAR    72000000 / 72 = 1 MHz
	FREQUENCY = TIM_CLOCK / ARR              1000000 / 1000 = 1 KHz
	DUTY = (CCRx / ARR) x 100                200 / 1000 x100 = 20
*/

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
//	TIM1->BDTR |= (uint32_t)0x5000;  // AOE and BKE enable

	// Dead time register. reference manual sayfa 361
	/*
	 * t_dtg = t_dts => 1 / 72MHz = 13,9nS ayarlamaya çalıştım.
	 */
	TIM1->BDTR |= (uint32_t)0x19;

	TIM1->EGR |= (uint32_t)0x01; // update generation
	TIM1->CR1 |= (uint32_t)0x80; // auto-reload preload enable

    // Counter enable
	TIM1->CR1 |= (uint32_t)0x01;  // Counter enable
}


void UsrTrigPwm1Channel1(void)
{
	uint16_t tempValue = (autoReload * g_dutyCycle) / 100;
	TIM1->CCR1 = tempValue; // Duty cycle
	TIM1->CCER &= ~(uint32_t)0xA;  // polarity high
	TIM1->CCER |= (uint32_t)0x01;  // TIM1_CH1 output enable and TIM1_CH1N complementary output enable
	Delay(1/2); // delay 1 ms, compliment comes behind
	TIM1->CCER |= (uint32_t)0x04; // 0x04

//	UsrLcdClearDisplay();
//	UsrLcdSetCursor(0, 0);
	char *a = {"%"};
	sprintf(g_sendStringUpLine, "CH1: %c%d DUTY\n", *a, g_dutyCycle);
	HAL_UART_Transmit(&huart1, (uint8_t*)g_sendStringUpLine, 16, 100);
//	UsrLcdSendString(g_sendStringUpLine);
}


void UsrTrigSwapPwm1Channel1(void)
{
	uint16_t tempValue = (autoReload * g_dutyCycle) / 100;
	TIM1->CCER &= ~(uint32_t)0x5;

	TIM1->CCR1 = tempValue; // Duty cycle
	TIM1->CCER |= (uint32_t)0x04;
	Delay(1/2);  // delay 1 ms, compliment comes behind
	TIM1->CCER |= (uint32_t)0x01; // TIM1_CH1 output enable and TIM1_CH1N complementary output enable
	TIM1->CCER |= (uint32_t)0xA;  // polarity low

//	UsrLcdClearDisplay();
//	UsrLcdSetCursor(0, 0);
	char *a = {"%"};
	sprintf(g_sendStringUpLine, "CH1: %c%d DUTY\n", *a, g_dutyCycle);
	HAL_UART_Transmit(&huart1, (uint8_t*)g_sendStringUpLine, 16, 100);
//	UsrLcdSendString(g_sendStringUpLine);

//	UsrLcdSetCursor(1,0);
	sprintf(g_sendStringDownLine, "COMPLIMENT\n");
	HAL_UART_Transmit(&huart1, (uint8_t*)g_sendStringDownLine, 16, 100);
//	UsrLcdSendString(g_sendStringDownLine);
}


void UsrTrigPwm1Channel2(void)
{
	uint16_t tempValue = (autoReload * g_dutyCycle) / 100;
	TIM1->CCR2 = tempValue; // Duty cycle
	TIM1->CCER &= ~(uint32_t)0xA0;  // polarity high
	TIM1->CCER |= (uint32_t)0x10;  // TIM1_CH2 output enable and TIM1_CH2N complementary output enable
	Delay(1/2);  // delay 1 ms, compliment comes behind
	TIM1->CCER |= (uint32_t)0x40;

//	UsrLcdClearDisplay();
//	UsrLcdSetCursor(0, 0);
	char *a = {"%"};
	sprintf(g_sendStringUpLine, "CH2: %c%d DUTY\n", *a, g_dutyCycle);
	HAL_UART_Transmit(&huart1, (uint8_t*)g_sendStringUpLine, 16, 100);
//	UsrLcdSendString(g_sendStringUpLine);
}


void UsrTrigSwapPwm1Channel2(void)
{
	uint16_t tempValue = (autoReload * g_dutyCycle) / 100;
	TIM1->CCER &= ~(uint32_t)0x50;

	TIM1->CCR2 = tempValue; // Duty cycle
	TIM1->CCER |= (uint32_t)0x40;  // TIM1_CH2 output enable and TIM1_CH2N complementary output enable
	Delay(1/2);  // delay 1 ms, compliment comes behind
	TIM1->CCER |= (uint32_t)0x10;
	TIM1->CCER |= (uint32_t)0xA0;  // polarity low

//	UsrLcdClearDisplay();
//	UsrLcdSetCursor(0, 0);
	char *a = {"%"};
	sprintf(g_sendStringUpLine, "CH2: %c%d DUTY\n", *a, g_dutyCycle);
	HAL_UART_Transmit(&huart1, (uint8_t*)g_sendStringUpLine, 16, 100);
//	UsrLcdSendString(g_sendStringUpLine);

//	UsrLcdSetCursor(1,0);
	sprintf(g_sendStringDownLine, "COMPLIMENT\n");
	HAL_UART_Transmit(&huart1, (uint8_t*)g_sendStringDownLine, 16, 100);
//	UsrLcdSendString(g_sendStringDownLine);
}


void UsrTrigPwm1Channel3(void)
{
	uint16_t tempValue = (autoReload * g_dutyCycle) / 100;

	TIM1->CCR3 = tempValue; // Duty cycle
	TIM1->CCER &= ~(uint32_t)0xA00;  // polarity high
	TIM1->CCER |= (uint32_t)0x100;  // TIM1_CH3 output enable and TIM1_CH3N complementary output enable
	Delay(1/2);  // delay 1 ms, compliment comes behind
	TIM1->CCER |= (uint32_t)0x400;

//	UsrLcdClearDisplay();
//	UsrLcdSetCursor(0, 0);
	char *a = {"%"};
	sprintf(g_sendStringUpLine, "CH3: %c%d DUTY\n", *a, g_dutyCycle);
	HAL_UART_Transmit(&huart1, (uint8_t*)g_sendStringUpLine, 16, 100);
//	UsrLcdSendString(g_sendStringUpLine);
}


void UsrTrigSwapPwm1Channel3(void)
{
	uint16_t tempValue = (autoReload * g_dutyCycle) / 100;
	TIM1->CCER &= ~(uint32_t)0x500;

	TIM1->CCR3 = tempValue; // Duty cycle
	TIM1->CCER |= (uint32_t)0x400;  // TIM1_CH3 output enable and TIM1_CH3N complementary output enable
	Delay(1/2);  // delay 1 ms, compliment comes behind
	TIM1->CCER |= (uint32_t)0x100;
	TIM1->CCER |= (uint32_t)0xA00;  // polarity low

//	UsrLcdClearDisplay();
//	UsrLcdSetCursor(0, 0);
	char *a = {"%"};
	sprintf(g_sendStringUpLine, "CH3: %c%d DUTY\n", *a, g_dutyCycle);
	HAL_UART_Transmit(&huart1, (uint8_t*)g_sendStringUpLine, 16, 100);
//	UsrLcdSendString(g_sendStringUpLine);

//	UsrLcdSetCursor(1,0);
	sprintf(g_sendStringDownLine, "COMPLIMENT\n");
	HAL_UART_Transmit(&huart1, (uint8_t*)g_sendStringDownLine, 16, 100);
//	UsrLcdSendString(g_sendStringDownLine);
}


void UsrAllChannelLowLevel(void)
{
	uint16_t tempValue = (autoReload * 0) / 100;
	TIM1->CCR1 = tempValue; // Duty cycle
	TIM1->CCR2 = tempValue;
	TIM1->CCR3 = tempValue;

	TIM1->CCER &= ~(uint32_t)0x555;

//	UsrLcdClearDisplay();
//	UsrLcdSetCursor(0, 0);
	char *a = {"%"};
	sprintf(g_sendStringUpLine, "ALL CH: %c%d DUTY\n", *a, 0);
	HAL_UART_Transmit(&huart1, (uint8_t*)g_sendStringUpLine, 16, 100);
//	UsrLcdSendString(g_sendStringUpLine);
}







