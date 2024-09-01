#ifndef __USR_GENERAL_H
#define __USR_GENERAL_H

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
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"


#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "stm32f103xb.h"


#include "usr_system.h"
#include "usr_battery_calc.h"
#include "usr_adc.h"
#include "usr_lcd.h"




#endif
