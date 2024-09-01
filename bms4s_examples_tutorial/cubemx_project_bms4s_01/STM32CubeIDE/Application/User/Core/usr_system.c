#include "usr_system.h"

_io void AllPeripheralProc(void);


S_ADC_PARAMETERS g_sAdcParameters;
S_ADC_RAW_PARAMETERS g_sAdcRawParameters;


void UsrSystemInitial(void)
{
    // UsrAdcInitial(enable);
    // UsrAdcInitial(&g_sAdcParameters);
    UsrI2CScan();
    if (UsrLcdInit())
    {
        UsrLcdSetCursor(0, 0);
        UsrLcdSendString("Eren Acarel");
        // UsrLcdClearDisplay();

    }

}


void UsrSystemGeneral(void)
{
    
}



void AllPeripheralProc(void)
{
    g_sAdcParameters.pAdc = &_USR_ADC_SELECT_CHANNEL_1;
    g_sAdcRawParameters.rawFourthCellNegativeValue = 0;
    g_sAdcRawParameters.rawThirdCellNegativeValue = 0;
    g_sAdcRawParameters.rawSecondCellNegativeValue = 0;
    g_sAdcRawParameters.rawFirstCellNegativeValue = 0;
    g_sAdcRawParameters.rawCurrentSenseValue = 0;
    g_sAdcRawParameters.rawVrefValue = 0;

}
