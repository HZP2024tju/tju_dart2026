#include "adc.h"
#include "voltage_ADC.h"
#include "tim.h"
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;
uint16_t ADC_value[2]={0};
float ADC_voltage[2]={0};
void voltage_ADC()
{
HAL_ADC_Start(&hadc1);
HAL_ADC_PollForConversion(&hadc1,HAL_MAX_DELAY);
ADC_value[0]=HAL_ADC_GetValue(&hadc1);
ADC_voltage[0]=(ADC_value[0]/4095.0)*3.3;
if(ADC_voltage[0]<=3)
{
for(int i=0;i<100;i++)
{
__HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_3,i);
HAL_Delay(10);
}
for(int i=99;i>0;i--)
{
__HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_3,i);
HAL_Delay(10);
}
}
}
