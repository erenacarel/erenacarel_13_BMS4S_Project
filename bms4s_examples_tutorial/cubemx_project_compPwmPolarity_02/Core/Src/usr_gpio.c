/*
 * usr_gpio.c
 *
 *  Created on: Aug 6, 2024
 *      Author: eacarel
 */


#include "usr_gpio.h"

void UsrInitGpio(void)
{
//	GPIOA->CRL &= (uint32_t)0x00;
//	GPIOB->CRL &= (uint32_t)0x00;

//	GPIOA->CRH &= (uint32_t)0x0000;
	GPIOA->CRH |= (uint32_t)0x0BBB;

//	GPIOB->CRH &= (uint32_t)0x00;
	GPIOB->CRH |= (uint32_t)0xBBB00000;
}
