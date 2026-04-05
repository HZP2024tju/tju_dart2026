#include "raspberrypi.h"
#include "stdlib.h"
#include "judge.h"
#include "referee.h"

extern float  PULL_3508_ANGLE_START;
extern float  ANGLE_SET_6020;
extern float	RELOAD_6020_ANGLE60 ;
extern float  RELOAD_6020_ANGLE180;
extern float  RELOAD_6020_ANGLE300 ;
extern uint8_t auto_start;
extern uint8_t referee_game_start;


float rasp_data[17] = {0};
int rasp_cmd = 0;
float rasp_dart = 0; //这里我偷懒上位机传过来的是浮点数,可以优化
uint8_t rasp_indebug = 0;

void rasp_unpack(uint8_t* data )  //树莓派两个节点和单片机共用一个通信协议
{
	if(data[0] != 0xC6)
	{
		return;
	}
	
	int index = data[1] & 0xF;
	
	if(data[1]>>4 == 0xB)
	{
		memcpy(&rasp_data[index],&data[2],4);
		judge_set_by_usart(index,rasp_data[index]);
		
	}
	else if(data[1]>>4 == 0xA)
	{
		rasp_cmd = index|0xA0;
		memcpy(&rasp_dart,&data[2],4);	
	}
		
	

}

void rasp_control(dart_control_t *dart_control)
{
	//裁判系统规定开始比赛时禁用调参,开始发射后禁用调参
	if(
#if REFEREE_START	
	uint8_t referee_game_start !=4 || 
#endif
	dart_control->Shoot_Mode != SHOOT_READY_3508_AND_2006 || auto_start == 1)
	{
		PULL_3508_ANGLE_START = PULL_3508_ANGLE_START_SET;   //还原
		
		return;
	}
	
	
	if(rasp_cmd == 0xA2)//debug 6020
	{
		dart_control->Shoot_Mode = SHOOT_READY_3508_AND_2006;
		PULL_3508_ANGLE_START = -9;
		
		switch((int)rasp_dart)//这对吗
		{
			case 1:
				ANGLE_SET_6020 = RELOAD_6020_ANGLE60;
			break;
			case 2:
				ANGLE_SET_6020 = RELOAD_6020_ANGLE180;
			break;
			case 3:
				ANGLE_SET_6020 = RELOAD_6020_ANGLE300;
			break;
		}
		
		rasp_indebug = 1;
	}
	else
	{
		ANGLE_SET_6020 = RELOAD_6020_ANGLE0;
		PULL_3508_ANGLE_START = PULL_3508_ANGLE_START_SET;
	}
	
}