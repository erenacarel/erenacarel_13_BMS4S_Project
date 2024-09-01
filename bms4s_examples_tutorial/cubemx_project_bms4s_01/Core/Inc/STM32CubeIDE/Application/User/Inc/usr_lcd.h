#ifndef __USR_LCD_H
#define __USR_LCD_H

#include "usr_general.h"

#define _USR_SERIAL_UART_CHANNEL huart1
#define _USR_LCD_PANEL_I2C_CHANNEL hi2c1

/* Command */
#define _USR_LCD_CLEAR_DISPLAY 0X01
#define _USR_LCD_RETURN_HOME 0X02
#define _USR_ENTRY_MODE_SET 0x04
#define _USR_DISPLAY_CONTROL 0x08
#define _USR_CURSOR_SHIFT 0x10
#define _USR_LCD_FUNCTION_SET 0x20
#define _USR_LCD_FUNCTION_SET_2_LINE_DISPLAY 0x28

#define _USR_LCD_SET_CGRAM_ADDR 0x40
#define _USR_LCD_SET_DDRAM_ADDR 0x80 

// flags for display entry mode
#define _USR_LCD_ENTRY_RIGHT 0x00
#define _USR_LCD_ENTRY_LEFT 0x02
#define _USR_LCD_ENTRY_SHIFT_INCREMENT 0x01
#define _USR_LCD_ENTRY_SHIFT_DECREMENT 0x00

// flags for display on/off control
#define _USR_LCD_DISPLAY_ON 0x04
#define _USR_LCD_DISPLAY_OFF 0x00
#define _USR_LCD_CURSOR_ON 0x02
#define _USR_LCD_CURSOR_OFF 0x00
#define _USR_LCD_BLINK_ON 0x01
#define _USR_LCD_BLINK_OFF 0x00

// flags for display/cursor shift
#define _USR_LCD_DISPLAY_MOVE 0x08
#define _USR_LCD_CURSOR_MOVE 0x00
#define _USR_LCD_MOVE_RIGHT 0x04
#define _USR_LCD_MOVE_LEFT 0x00

// flags for function set
#define _USR_LCD_8BITMODE 0x10
#define _USR_LCD_4BITMODE 0x00
#define _USR_LCD_2LINE 0x08
#define _USR_LCD_1LINE 0x00
#define _USR_LCD_5x10DOTS 0x04
#define _USR_LCD_5x8DOTS 0x00

// flags for backlight control
#define _USR_LCD_BACKLIGHT 0x08
#define _USR_LCD_NOBACKLIGHT 0x00

/* Enable Bit */
#define _USR_ENABLE_BIT 0x04

/* Read Write Bit */
#define _USR_READ_WRITWE_BIT 0x00

/* Register Select Bit */
#define _USR_REGISTER_SELECT_BIT 0x01

/* Device I2C Address */
#define _USR_LCD_I2C_ADDRESS 0x27


bool UsrLcdInit(void);
void UsrI2CScan(void);
void UsrLcdSendString(char *m_string);
void UsrLcdClearDisplay(void);

#endif // !
