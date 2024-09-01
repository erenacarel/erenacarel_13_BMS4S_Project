#ifndef __USR_ADC_H
#define __USR_ADC_H

#include "usr_general.h"
 
#define _USR_ADC_RESOLUTION (float)4095.0
#define _USR_ADC_CHANNEL_COUNT (uint8_t)7
#define _USR_ADC_SAMPLE_COUNT (uint8_t)16

#define _USR_ADC_FOURTH_CELL_NEG_CHANNEL 0    // adc channel 1
#define _USR_ADC_THIRD_CELL_NEG_CHANNEL 1    // adc channel 2
#define _USR_ADC_SECOND_CELL_NEG_CHANNEL 2    // adc channel 3
#define _USR_ADC_FIRST_CELL_NEG_CHANNEL 3    // adc_channel 4
#define _USR_ADC_CURRENT_SENSE_CHANNEL 4    // adc_channel 5
#define _USR_ADC_TEMP_SENSOR_CHANNEL 5 
#define _USR_VREF_ADC_CHANNEL 6 // adc_channel 11

#define _USR_ADC_SELECT_CHANNEL_1 hadc1
// #define _USR_ADC_SELECT_CHANNEL_2 hadc2

#define _USR_ADC1_INIT_FUNC() MX_ADC1_Init()
// #define _USR_ADC2_INIT_FUNC MX_ADC2_Init


typedef struct S_ADC_RAW_PARAMETERS_TAG
{
    uint32_t rawFourthCellNegativeValue;
    uint32_t rawThirdCellNegativeValue;
    uint32_t rawSecondCellNegativeValue;
    uint32_t rawFirstCellNegativeValue;
    uint32_t rawCurrentSenseValue;
    uint32_t rawVrefValue;
}S_ADC_RAW_PARAMETERS;

typedef struct S_ADC_PARAMETERS_TAG
{
    ADC_HandleTypeDef *pAdc;
}S_ADC_PARAMETERS;

extern S_ADC_RAW_PARAMETERS g_sAdcRawParameters;
extern S_ADC_PARAMETERS g_sAdcParameters;

typedef enum 
{
    disable,
    enable
}EAdcControl;



bool UsrAdcInitial(S_ADC_PARAMETERS *f_pParameter);
bool UsrAdcGetValues(S_ADC_PARAMETERS *f_pParameter, S_ADC_RAW_PARAMETERS *f_pData);
void UsrAdcPeripheral(EAdcControl f_eControl);
void UsrAdcCallback(void);


#endif // !__USR_ADC_H
