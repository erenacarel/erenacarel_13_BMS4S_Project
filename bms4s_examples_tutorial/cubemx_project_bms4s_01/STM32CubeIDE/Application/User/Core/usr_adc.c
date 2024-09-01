#include "usr_adc.h"

bool m_adcFinishedFlag;
bool m_adcStartGetValueFlag;
bool m_adcInitialFlag;
uint8_t m_adcSampleCounter;

uint16_t m_adcDmaValues[_USR_ADC_CHANNEL_COUNT];
_iov uint32_t m_adcRawValueBuf[_USR_ADC_CHANNEL_COUNT][_USR_ADC_SAMPLE_COUNT];

bool UsrAdcInitial(S_ADC_PARAMETERS *f_pParameter)
{
    *f_pParameter = g_sAdcParameters;
    m_adcInitialFlag = true;
    // flags

    HAL_ADC_DeInit(g_sAdcParameters.pAdc);
    _USR_ADC1_INIT_FUNC();

    ADC1->CR2 &= (uint32_t)0xfffffffe;  // first adc disable
    ADC1->CR2 |= (uint32_t)0x00000004;  // second adc calibration enable
    while(ADC1->CR2 & (uint32_t)0x00000004)   // third wait adc calibration
    ;
    ADC1->CR2 |= (uint32_t)0x00000001;  // the end adc enable 

    // HAL_ADCEx_Calibration_Start(&_USR_ADC_SELECT_CHANNEL_1);
    return true;
}



bool UsrAdcGetValues(S_ADC_PARAMETERS *f_pParameter, S_ADC_RAW_PARAMETERS *f_pData)
{
    if (f_pData != NULL && f_pParameter != NULL)
    {
        *f_pParameter = g_sAdcParameters;
        *f_pData = g_sAdcRawParameters;
    }
    
    m_adcFinishedFlag = false;
    if (HAL_ADC_Start_DMA(g_sAdcParameters.pAdc, (uint32_t*)m_adcDmaValues, _USR_ADC_CHANNEL_COUNT) == HAL_OK)
    {
        while (!m_adcFinishedFlag)
        ;

        uint32_t fourthCellNegValue = 0;
        uint32_t thirdCellNegValue = 0;
        uint32_t secondCellNegValue = 0;
        uint32_t firstCellNegValue = 0;
        uint32_t currentSenseValue = 0;
        uint32_t vrefVoltage = 0;

        if (!m_adcStartGetValueFlag)
        {
            return false;
        }
        else
        {
            for (uint8_t i = 0; i < _USR_ADC_SAMPLE_COUNT; i++)
            {
                fourthCellNegValue += m_adcRawValueBuf[_USR_ADC_FOURTH_CELL_NEG_CHANNEL][i];
                thirdCellNegValue += m_adcRawValueBuf[_USR_ADC_THIRD_CELL_NEG_CHANNEL][i];
                secondCellNegValue += m_adcRawValueBuf[_USR_ADC_SECOND_CELL_NEG_CHANNEL][i];
                firstCellNegValue += m_adcRawValueBuf[_USR_ADC_FIRST_CELL_NEG_CHANNEL][i];
            }

            fourthCellNegValue /= _USR_ADC_SAMPLE_COUNT;
            thirdCellNegValue /= _USR_ADC_SAMPLE_COUNT;
            secondCellNegValue /= _USR_ADC_SAMPLE_COUNT;
            firstCellNegValue /= _USR_ADC_SAMPLE_COUNT;

            return true;
        }
    }
    else
    {
        HAL_ADC_Stop_DMA(g_sAdcParameters.pAdc);
        return false;
    }

}


void UsrAdcPeripheral(EAdcControl f_eControl)
{
    if (f_eControl == disable)
    {
        HAL_ADC_DeInit(&_USR_ADC_SELECT_CHANNEL_1);
    }
    else
    {
        HAL_ADC_DeInit(&_USR_ADC_SELECT_CHANNEL_1);
        _USR_ADC1_INIT_FUNC();
        UsrAdcInitial(&g_sAdcParameters);
    }

}


void UsrAdcCallback(void)
{
    // Take the values from Dma
    for (uint8_t i = 0; i < _USR_ADC_CHANNEL_COUNT; i++)
    {
        m_adcRawValueBuf[i][m_adcSampleCounter] = m_adcDmaValues[i];
    }
    // Increase the sample count
    m_adcSampleCounter++;
    // check adc sampling that is enough
    if (m_adcSampleCounter >= _USR_ADC_SAMPLE_COUNT)
    {
        // Close the Dma interrupt reading
        HAL_ADC_Stop_DMA(&_USR_ADC_SELECT_CHANNEL_1);
        // Clear sampling count
        m_adcSampleCounter = 0;
        // show the system that the reading finished
        m_adcFinishedFlag = true;     
    }
}


