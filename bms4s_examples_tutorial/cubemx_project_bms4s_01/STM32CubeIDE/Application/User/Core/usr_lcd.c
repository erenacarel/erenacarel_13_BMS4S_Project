#include "usr_lcd.h"

uint8_t startMessage[64] = "";
uint8_t data[64] = "";

_io bool lcdSendCommandProc(char m_command);
_io bool lcdSendDataProc(char m_data);


bool UsrLcdInit(void)
{
    if(!lcdSendCommandProc(_USR_LCD_CLEAR_DISPLAY))
    	return false;
    if(!lcdSendCommandProc(_USR_LCD_RETURN_HOME))
    	return false;
    if(!lcdSendCommandProc(_USR_LCD_FUNCTION_SET | _USR_LCD_FUNCTION_SET_DATA_LENGTH)) // Function set -> DB5=1 and DB3=N=1  2-line display mode.      2x16 kullanılabilir
    	return false;
	if(!lcdSendCommandProc(_USR_LCD_DISPLAY_ON))   // Display ON/OFF Control DB3=1 and DB2=D=1 display is turned on  // if DB1=C=1 cursor on
		return false;
	if(!lcdSendCommandProc(0x80))  // imleci ilk satırın basinda tut.
		return false;
	return true;
}


void UsrLcdSendString(char *m_string)
{
    while(*m_string)
    {
        lcdSendDataProc(*m_string++);
    }
        
}


void UsrLcdClearDisplay(void)
{
    lcdSendCommandProc(_USR_LCD_CLEAR_DISPLAY);
}


void UsrLcdSetCursor(uint8_t f_pRow, uint8_t f_pCol)
{
    if (f_pRow == 0)
        lcdSendCommandProc(_USR_LCD_SET_DDRAM_ADDR | f_pCol);
    else if (f_pRow == 1)
        lcdSendCommandProc(_USR_LCD_SET_SUB_DDRAM_ADDR | f_pCol);
    else
        f_pRow = 0;
}


void UsrI2CScan(void)
{
    uint16_t i = 0;
    uint16_t ret = 0;

    HAL_UART_Transmit(&_USR_SERIAL_UART_CHANNEL, startMessage, sprintf((char*)startMessage, "Starting I2C Scanning...\r\n"), 100);

    for (i = 1; i < 128; i++)
    {
        ret = HAL_I2C_IsDeviceReady(&_USR_LCD_PANEL_I2C_CHANNEL, (i << 1), 2, 100);
        if (ret == HAL_OK)
        {
            HAL_UART_Transmit(&_USR_SERIAL_UART_CHANNEL, data, sprintf((char*)data, "iste adres: 0x%X\n", i), 100);
        }
    }
    HAL_UART_Transmit(&_USR_SERIAL_UART_CHANNEL, data, sprintf((char*)data, "islem bitmistir.\r\n"), 100);
}


_io bool lcdSendCommandProc(char m_command)
{
    char data_u = ' ';
    char data_l = ' ';
    uint8_t data_t[4] = {"0"};

	/*	Komut göndermek için genellikle RS = 0 ve E = 1 yapılır.
	 *  Ardından yüksek değerlikli 4 bit gönderilir.
	 *  Yüksek değerlikli 4 bit gönderildikten sonra E = 0 yapılır.
	 *  Gönderilen yüksek değerlikli 4 bit temizlenir.
	 *  Ardından E = 1 yapılır ve ilk 4 bit lcd'ye gönderilir.
	 *  Ve E = 0 yapılarak, son gönderilen düşük değerli 4 bit porttan silinir.
	 */    
    
    data_u = (m_command & 0xF0);   // cmd & 0x11110000 -> 0xabcdefgh & 0x11110000 = 0xabcd0000
    data_l = ((m_command << 4) & 0xF0); // (cmd << 4) & 0x11110000 -> 0xefgh0000 & 0x11110000 = 0xefgh0000

	data_t[0] = data_u | 0x0C;  // en = 1 & rs = 0    0xabcd0000 | 0x00001100 -> 0xabcd1100
	data_t[1] = data_u | 0x08;  // en = 0 & rs = 0    0xabcd0000 | 0x00001000 -> 0xabcd1000
	data_t[2] = data_l | 0x0C;  // en = 1 & rs = 0    0xefgh0000 | 0x00001100 -> 0xefgh1100
	data_t[3] = data_l | 0x08;  // en = 0 & rs = 0    0xefgh0000 | 0x00001000 -> 0xefgh1000

    if (HAL_OK == HAL_I2C_Master_Transmit(&_USR_LCD_PANEL_I2C_CHANNEL, (_USR_LCD_I2C_ADDRESS << 1), (uint8_t*)data_t, 4, HAL_MAX_DELAY))
        return true;
    return false;
}


_io bool lcdSendDataProc(char m_data)
{
    char data_u = ' ';
    char data_l = ' ';
    uint8_t data_t[4] = {"0"};

    // öncelikli ve düşük seviyeli olarak ikiye bölüyor
    data_u = (m_data & 0xF0); // data & 0x11110000 -> 0xabcdefgh & 0x11110000 = 0xabcd0000
    data_l = ((m_data << 4) & 0xF0); // (data << 4) & 0x11110000 -> 0xefgh0000 & 0x11110000 = 0xefgh0000

	data_t[0] = data_u | 0x0D;  // en = 1 & rs = 1   0xabcd0000 | 0x00001101 -> 0xabcd1101   // yazma yani gönderme yapıyoruz
	data_t[1] = data_u | 0x09;  // en = 0 & rs = 1   0xabcd0000 | 0x00001001 -> 0xabcd1001   // silme yapıyoruz
	data_t[2] = data_l | 0x0D;  // en = 1 & rs = 1   0xefgh0000 | 0x00001100 -> 0xefgh1101   // düşük seviyeli bit gönderme
	data_t[3] = data_l | 0x09;  // en = 0 & rs = 1   0xefgh0000 | 0x00001000 -> 0xefgh1001   // sonra silme

    if (HAL_OK == HAL_I2C_Master_Transmit(&_USR_LCD_PANEL_I2C_CHANNEL, (_USR_LCD_I2C_ADDRESS << 1), (uint8_t*)data_t, 4, HAL_MAX_DELAY))
        return true;
    return false;
}
