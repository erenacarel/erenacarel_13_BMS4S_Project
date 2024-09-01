/*
 * usr_system.h
 *
 *  Created on: Jul 18, 2024
 *      Author: eacarel
 */

#ifndef INC_USR_SYSTEM_H_
#define INC_USR_SYSTEM_H_

#include "usr_general.h"

#define USR_ADC_BUFFER_SIZE 7


void UsrSystemInitial(void);
void UsrSystemGeneral(void);
void UsrSetPwm(uint16_t pulseValue);
uint32_t UsrVrefVoltage(void);
uint32_t UsrVrefConversion(uint32_t kanal, uint32_t vrefint);
void UsrCellCalculate(void);

#endif /* INC_USR_SYSTEM_H_ */
