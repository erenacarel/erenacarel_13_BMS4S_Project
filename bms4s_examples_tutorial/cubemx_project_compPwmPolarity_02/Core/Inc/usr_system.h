/*
 * usr_system.h
 *
 *  Created on: Aug 6, 2024
 *      Author: eacarel
 */

#ifndef INC_USR_SYSTEM_H_
#define INC_USR_SYSTEM_H_

#include "usr_general.h"

#define Delay(x) HAL_Delay(x)
#define ABS_DIFF(x, y) ((x) > (y) ? ((x) - (y)) : ((y) - (x)))

void UsrSystemInitial(void);
void UsrSystemGeneral(void);
void UsrBatteryOnOffControl(void);

extern uint16_t autoReload;
extern uint16_t prescaler;
extern char m_batteryBufferUpLine[16];
extern char m_batteryBufferDownLine[16];
extern uint32_t g_timerCount;
extern uint16_t g_dutyCycle;

#endif /* INC_USR_SYSTEM_H_ */
