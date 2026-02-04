#include "dsp_adc.h"
#include "adc.h"
extern ADC_HandleTypeDef hadc1;
uint16_t bat_v_buf;
uint16_t adc_value;
float adc_voltage;
void my_adc_init()
{
HAL_ADC_Start_DMA(&hadc1,(uint32_t *)&bat_v_buf,1);	
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
adc_value=bat_v_buf;
adc_voltage=adc_value*3.55f*2.5f/4095.0f;
}





