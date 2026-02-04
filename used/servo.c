#include "servo.h"
#include "view.h"
#include "mitrx.h"
#include "controller.h"
#include "stm32f4xx.h"
#include "JY901B.h"
#include "dsp_adc.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;


extern float view_y[2];
extern float view_x;

PID_t servo_roll_angle_pid;
PID_t servo_roll_gyro_pid;

#define servo_angle_kp  200.0f
#define servo_angle_kd  0.f
#define servo_angle_ki  0.001f

float servo_gyro_kp = 0;
float servo_gyro_kd = 0;
float servo_gyro_ki = 0;

float servo_Hinf_sensitive = 23;
float servo_pitch_sensitive= 80;
float servo_yaw_sensitive= 50;

int PWM_TEST1 = 64;
int PWM_TEST2= 85;
int PWM_TEST3 = 75;
int PWM_TEST4 = 57;

float servo_Hinf_mitrx[4 * 4 + 3] = {     
  -64.5616, -102.2691,  -76.8498  ,-29.4843
  -64.5617 , 102.2689 ,  -76.8497  ,-29.4843
  -64.5318 , 102.2690  , 76.8730  ,-29.4707
  -64.5318 ,-102.2691   ,76.8729 , -29.4707};
float servo_Yaw_K[2 * 2 + 1] ={
												
 -373.5027  , -1.1730,
 -301.9527   ,-0.6372
																		};

float servo_Hinf_x[4 * 1 + 1];

float servo_Hinf_u[4 * 1 + 1];
								
float servo_Yaw_x[2 * 1 + 1];

float servo_Yaw_u[2 * 1 + 1];				

int servo_contorl_u[5];																		
															
float servo_pitch = 0;																		
float servo_yaw = 0;

float servo_limit_num = 23;

void servo_limit()
{
		for(uint8_t i = 0;i < 4 ; i++)
	{
		
		servo_Hinf_u[i] = servo_Hinf_u[i] * servo_Hinf_sensitive / 10000.0f;
		if(servo_Hinf_u[i] > servo_limit_num)
		{
			servo_Hinf_u[i] = servo_limit_num;
		}
		else if(servo_Hinf_u[i] < -servo_limit_num)
		{
			servo_Hinf_u[i] = -servo_limit_num;
		}
	}

		servo_pitch = servo_pitch * servo_pitch_sensitive / 10000.0f;
		if(servo_pitch > servo_limit_num)
		{
			servo_pitch = servo_limit_num;
		}
		else if(servo_pitch < -servo_limit_num)
		{
			servo_pitch = -servo_limit_num;
		}
		
	
		
		
		servo_yaw = servo_yaw * servo_yaw_sensitive / 10000.0f;
		if(servo_yaw > servo_limit_num)
		{
			servo_yaw = servo_limit_num;
		}
		else if(servo_yaw < -servo_limit_num)
		{
			servo_yaw = -servo_limit_num;
		}
		
}


void servo_ini()
{
	HAL_TIM_Base_Start_IT(&htim2);
	//HAL_TIM_Base_Start_IT(&htim3);

	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
	
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_4);


	my_adc_init();
	//PID_Init(&servo_roll_angle_pid,30.f,7.f,0.f,servo_angle_kp,servo_angle_ki,servo_angle_kd,0.f,0.f,0.000795774715459476f,0.f,5,0x11);      //角速度限幅
	
	  PID_Init(&servo_roll_angle_pid,10000.f,100.f,0.f,servo_angle_kp,servo_angle_ki,servo_angle_kd,0.f,0.f,0.000795774715459476f,0.0f,5,0x11);
		 
			PID_Init(&servo_roll_gyro_pid,10000.0,10000.0,0.0f,
					servo_gyro_kp,servo_gyro_kp,servo_gyro_kp,0.0f,0.0f,0.000795774715459476f,0.0f,5,0x11);
		
  //PID_Init(&servo_roll_gyro_pid,100,50,0,servo_gyro_kp,servo_gyro_ki,servo_gyro_kd,0,0,000795774715459476f,0,5,0x11);    //舵机角度限幅
//


	
}

void servo_count()										//分别对应右上  右下  左上  左下
{

	servo_contorl_u[0] =  -servo_Hinf_u[3] +servo_yaw + servo_pitch;                           
	servo_contorl_u[1] = servo_Hinf_u[1] -servo_yaw+ servo_pitch;                             
	servo_contorl_u[2] = servo_Hinf_u[2] +servo_yaw - servo_pitch;                         
	servo_contorl_u[3] = -servo_Hinf_u[0] -servo_yaw - servo_pitch;  
	
	for(uint8_t i = 0;i < 4 ; i++)
	{ 
		
		if(servo_contorl_u[i] > servo_limit_num )
		{
			servo_contorl_u[i] = servo_limit_num;
		}
		else if(servo_contorl_u[i] < -servo_limit_num)
		{
			servo_contorl_u[i] = -servo_limit_num;
		}
		
	}
	servo_contorl_u[0] += PWM_TEST1 ;          //右下   - 64 -               
	servo_contorl_u[1] += PWM_TEST2 ;          //右上    		85
	servo_contorl_u[2] += PWM_TEST3 ;          // 左下   	75
	servo_contorl_u[3] += PWM_TEST4 ;					//左上		25-	48
	
	
}

void servo_control()
{
	
	JY901_GetData(servo_Hinf_x);
	
	mitrx_x(servo_Hinf_mitrx,servo_Hinf_x,servo_Hinf_u,4,4,1);  //计算控制向量

	
	view_y[1] = servo_Hinf_x[2]; //yaw: 角度 角速度	
	mitrx_x(servo_Yaw_K,view_y,servo_Yaw_u,2,2,1);
	servo_yaw = servo_Yaw_u[0];                              
	
	servo_pitch = PID_Calculate(&servo_roll_angle_pid,view_x,0.f);       //在灯偏上和灯偏右时servo_yaw ,servo_pitch必须是正的
	servo_roll_angle_pid.Output = 0;

	servo_limit();
	servo_count();
	

		__HAL_TIM_SetCompare(&htim3  ,TIM_CHANNEL_1,servo_contorl_u[0]); //119 79  39  

		__HAL_TIM_SetCompare(&htim3  ,TIM_CHANNEL_2,servo_contorl_u[1]); // 106 66 26 

		__HAL_TIM_SetCompare(&htim3  ,TIM_CHANNEL_3,servo_contorl_u[2]); //121 81 41
		__HAL_TIM_SetCompare(&htim3  ,TIM_CHANNEL_4,servo_contorl_u[3]); //113 73 33 



	
}

