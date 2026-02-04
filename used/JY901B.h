#include "stm32f4xx.h"
#ifndef __JY901B_H
#define __JY901B_H

typedef struct kf
{
	float angle;
	float gyro	;
	float *H;
	float *K;
}JY901_Kalman_struct;

typedef struct damn
{
	
float ANGLE_x;
float ANGLE_y;	
float ANGLE_z;
	
float gyro_x;
float gyro_y;	
float gyro_z;
}JY901_data;


HAL_StatusTypeDef JY901L_ReadAngles(void);
HAL_StatusTypeDef JY901L_ReadAngular_v(void);
void JY901_GetData(float * data_x);
void JY901_Kalman_init(JY901_Kalman_struct * kf,float *kf_mitrx,float *kf_H);

#endif

