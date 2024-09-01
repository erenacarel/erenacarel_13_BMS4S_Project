/*
 * usr_adc.h
 *
 *  Created on: Aug 6, 2024
 *      Author: eacarel
 */

#ifndef INC_USR_ADC_H_
#define INC_USR_ADC_H_

#include "usr_general.h"

#define _USR_ADC_BUF_FILTER_SIZE (uint8_t)4

extern uint16_t g_batteryValues[4];
extern uint8_t g_balansComplete;
extern uint16_t g_batteryValuesModified[4];

void UsrReadAdc(void);
void UsrCheckBat0Bat1(void);
void UsrCheckBat1Bat2(void);
void UsrCheckBat2Bat3(void);


#endif /* INC_USR_ADC_H_ */
