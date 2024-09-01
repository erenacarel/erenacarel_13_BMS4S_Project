/*
 * usr_pwm.h
 *
 *  Created on: Aug 6, 2024
 *      Author: eacarel
 */

#ifndef INC_USR_PWM_H_
#define INC_USR_PWM_H_

#include "usr_general.h"

#define _USR_PWM1_DUTY (uint16_t)10
#define _USR_PWM2_DUTY (uint16_t)10
#define _USR_PWM3_DUTY (uint16_t)10

void UsrTrigPwm1Channel1(void);
void UsrTrigSwapPwm1Channel1(void);
void UsrTrigPwm1Channel2(void);
void UsrTrigSwapPwm1Channel2(void);
void UsrTrigPwm1Channel3(void);
void UsrTrigSwapPwm1Channel3(void);
void UsrInitPwm(void);
void UsrAllChannelLowLevel(void);

#endif /* INC_USR_PWM_H_ */
