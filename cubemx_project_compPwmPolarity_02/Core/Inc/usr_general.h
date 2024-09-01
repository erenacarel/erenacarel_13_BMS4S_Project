/*
 * usr_general.h
 *
 *  Created on: Aug 6, 2024
 *      Author: eacarel
 */

#ifndef INC_USR_GENERAL_H_
#define INC_USR_GENERAL_H_

#define _io static
#define _iov static volatile

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "stm32f103xb.h"

#include "usr_lcd.h"
#include "usr_adc.h"
#include "usr_gpio.h"
#include "usr_pwm.h"
#include "usr_uart.h"
#include "usr_tim.h"
#include "usr_system.h"

#endif /* INC_USR_GENERAL_H_ */
