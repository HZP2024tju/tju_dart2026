#include "servo.h"


#define servo_turn TIM_CHANNEL_1
#define servo_down TIM_CHANNEL_2
#define servo_grab TIM_CHANNEL_3
#define servo_push TIM_CHANNEL_4

#define servo_tim htim1

extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim8;
extern fp32 ANGLE_SET_3508;
extern fp32 ANGLE_SET_3508_use;

uint8_t reload_start1 = 0;
uint8_t reload_start1_last = 0;
uint8_t reload_start2 = 0; 
uint8_t reload_start2_last = 0;
uint8_t auto_start = 0;

void servo_ini()
{
	HAL_TIM_Base_Start(&htim1);
	HAL_TIM_Base_Start(&htim8);

	HAL_TIM_PWM_Start(&servo_tim, servo_turn);
	HAL_TIM_PWM_Start(&servo_tim, servo_down);
	HAL_TIM_PWM_Start(&servo_tim, servo_grab);
	HAL_TIM_PWM_Start(&servo_tim, servo_push);
	HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_3);

		return ;
}
 
void servo_reload(void *argument)
{
	
	while(1)
	{

		osDelay(100);
	}
		
	
}


