/**

仍然有一些bug,但不影响发射,

**/
#include "dart_task.h"
#include "stm32f4xx.h"
#include "user_lib.h"
#include "CAN_receive.h"
#include "tim.h"
#include "force.h"
#include "usbd_cdc_if.h"
#include "referee.h"
#include "view.h"

static void dart_init(dart_control_t *init); 
static void dart_set_mode(dart_control_t *set_mode);
static void dart_feedback_update(dart_control_t *feedback_update);
static void dart_set_control(dart_control_t *set_control);
static void dart_control_loop(dart_control_t *control_loop);

extern game_status_t dart_message4;  
dart_control_t dart_control;
int16_t Motor1_3508_Current;
int16_t Motor_Bullet_2006_Current; 		
int16_t Motor_Yaw_Current;
int16_t testCurrent3508;
int16_t testCurrent2006;
uint16_t triggerPWM	=	0;
uint16_t reloadPWM = 54;
uint16_t upPWM = 52;
uint16_t downPWM = 79;
uint16_t lockPWM = 0x53;
uint16_t unlockPWM = 0x22;                //1100   200   从大到小  为顺时针

float PULL_3508_ANGLE_START = -1.f;

float dart_2006_angle_set = -170.0;//-200                    //       46.8469658        3  -50.5     5    -52            1  61.3     3 - 59.5        5 - 61     6  -61             3 -57.8  5  -59.4      6   -57.5    2 -56.8;                       //     5  73   3   71.4;3和5号可以认为一致
																			
//-4 约为35cm  bias
fp32 Yaw_Angle = -70;//-70;//-85;               //修改Yaw轴方向,越负越往右

uint8_t force_start = 0;
float force_mv_set = 450.f;              //25m约为 450mv       5mv正好是高出一个大装甲板的距离     3mv一个小装甲板
float force_OK = 0;
uint32_t force_OK_count = 0;             //-50.6843452

uint8_t auto_start_time = 0;
uint8_t auto_start_last;

float PULL_3508_SPEED_SET ;


float dart_I_correct = 1000000000.0f;	
float dart_P_correct = 3000.8f;

fp32 ANGLE_SET_3508 = -5;
fp32 ANGLE_SET_6020 = RELOAD_6020_ANGLE0; //-2.1000699988;//中位



int ttt = 55;// 下降舵机: 55 81  ;新抓取舵机中位 1号:抓:45  松:60    
int ttt1 = 75;// 另一个:80 - 400 松:110 夹:240:tim1  :tim8 : 75 中位 ,54 夹着


void dart_angle_clear(Motor_t* motor)    //清除电机的角度值，这样我们就可以借堵转/限位开关设定初始角度值
{
	  motor->round_cnt = 0;
		motor->offset_ecd = motor->motor_measure->ecd;
		motor->angle = 0;
}

int dart_motor_check(Motor_t* motor,float target_angle,float sensitive)  //检测电机是否转到指定角度,需要设定敏感值
{
		if(fabs(motor->angle - target_angle) <sensitive)
		{
			return 1;
		}
		else
		{
			return 0;
		}
}

float dart_6020_angle_set(uint8_t shoot_time,uint8_t step)
{
	switch(shoot_time)
	{
		case 0:
			return RELOAD_6020_ANGLE0;             //case0情况必须加,否则函数末尾没有return默认返回0
		break;
		
		case 1:
			if(step == SERVO_READY || step == SERVO_PULL)
			{
				return RELOAD_6020_ANGLE60;
			}
			else
			{
				return RELOAD_6020_ANGLE120;
			}
		break;
		case 2:
			if(step == SERVO_READY || step == SERVO_PULL)
			{
				return RELOAD_6020_ANGLE180;
			}
			else
			{
				return RELOAD_6020_ANGLE240;
			}
		break;
		case 3:
			if(step == SERVO_READY || step == SERVO_PULL)
			{
				return RELOAD_6020_ANGLE300;
			}
			else
			{
				return RELOAD_6020_ANGLE360;
			}
		break;
			
	}
	
	return RELOAD_6020_ANGLE0;    //现在添加补偿情况
}

void dart_task(void const * argument){

		//延时等待电机上电
	  vTaskDelay(DART_TASK_INIT_TIME);
		//镖架初始化
		dart_init(&dart_control);
		//dwt定时器开启
	  uint32_t System_Clock = osKernelSysTick();
		while (1)
    {
			
	
				referee_auto();
			//	view_err();
			
				dart_control.Dt = DWT_GetDeltaT(&dart_control.Dwt_Count);
				dart_feedback_update(&dart_control);
				dart_set_mode(&dart_control);
				dart_set_control(&dart_control);
				dart_control_loop(&dart_control);
			
				//电流赋值
				Motor1_3508_Current =  dart_control.Dart_3508_Motor.give_current;
				Motor_Bullet_2006_Current = -dart_control.Dart_2006_Bullet_Motor.give_current;
				Motor_Yaw_Current = dart_control.Dart_Yaw_Motor.give_current;
				//发送电流
				CAN_cmd_dart(Motor1_3508_Current,Motor_Bullet_2006_Current,Motor_Yaw_Current,0);               //上弹电流负远离 正靠近   偏转2006负远离  正靠近   Yaw电机待测试
				CAN_cmd_dart6020(0,0,0,dart_control.Dart_6020_Motor.give_current);

				__HAL_TIM_SET_COMPARE(&tim_lock, channel_lock,triggerPWM);
				__HAL_TIM_SET_COMPARE(&tim_down, channel_down,reloadPWM);

//				//任务延迟
				vTaskDelay(DART_CONTROL_TIME);
		}
}


void dart_init(dart_control_t *init)
{
    if (init == NULL)
    {
        return;
    }


		 //电机数据指针获取
		init->Dart_3508_Motor.motor_measure 			 = get_dart_motor_measure_point(CAN_3508_M1_ID - CAN_DART_ALL_ID);
		init->Dart_2006_Bullet_Motor.motor_measure = get_dart_motor_measure_point(CAN_2006_M2_ID - CAN_DART_ALL_ID);
		init->Dart_Yaw_Motor.motor_measure 				 = get_dart_motor_measure_point(CAN_YAW_M3_ID	 - CAN_DART_ALL_ID);
		init->Dart_6020_Motor.motor_measure 			 = get_dart_motor_measure_point(CAN_6020_M4_ID - CAN_DART_ALL_ID);//已统一id写法,只需修改枚举
    //遥控器数据指针获取
    init->Dart_Rc_Ctrl = get_remote_control_point();//大疆说要这么做的
			
		//初始化3508速度环pid

		PID_Init(&init->Dart_3508_Gyro_Pid,MOTOR_3508_GYRO_PID_MAX_OUT,MOTOR_3508_GYRO_PID_MAX_IOUT,0.0f,
					MOTOR_3508_GYRO_PID_KP,MOTOR_3508_GYRO_PID_KI,MOTOR_3508_GYRO_PID_KD,0.0f,0.0f,0.000795774715459476f,0.0f,5,0x11);
		
		PID_Init(&init->Dart_3508_Angle_Pid,BULLET_3508_ANGLE_PID_MAX_OUT,BULLET_3508_ANGLE_PID_MAX_IOUT,0.0f,
				BULLET_3508_ANGLE_PID_KP,BULLET_3508_ANGLE_PID_KI,BULLET_3508_ANGLE_PID_KD,0.0f,0.0f,0.000795774715459476f,0.0f,1,0x11);	
		
		//初始化发射2006电机pid
		
			
		PID_Init(&init->Dart_2006_Bullet_Force_ANGLE_Pid,FORCE_BULLET_2006_ANGLE_PID_MAX_OUT,FORCE_BULLET_2006_ANGLE_PID_MAX_IOUT,0.0f,
				FORCE_BULLET_2006_ANGLE_PID_KP,FORCE_BULLET_2006_ANGLE_PID_KI,FORCE_BULLET_2006_ANGLE_PID_KD,0.0f,0.0f,0.000795774715459476f,0.0f,5,0x11);
				
		PID_Init(&init->Dart_2006_Bullet_Force_GYRO_Pid,FORCE_BULLET_2006_GYRO_PID_MAX_OUT,FORCE_BULLET_2006_GYRO_PID_MAX_IOUT,0.0f,
				FORCE_BULLET_2006_GYRO_PID_KP,FORCE_BULLET_2006_GYRO_PID_KI,FORCE_BULLET_2006_GYRO_PID_KD,0.0f,0.0f,0.000795774715459476f,0.0f,5,0x11);
		

    PID_Init(&init->Dart_2006_Bullet_Angle_Pid,BULLET_2006_ANGLE_PID_MAX_OUT,BULLET_2006_ANGLE_PID_MAX_IOUT,0.0f,
				BULLET_2006_ANGLE_PID_KP,BULLET_2006_ANGLE_PID_KI,BULLET_2006_ANGLE_PID_KD,0.0f,0.0f,0.000795774715459476f,0.0f,1,0x11);	
		
    PID_Init(&init->Dart_2006_Bullet_Gyro_Pid,BULLET_2006_GYRO_PID_MAX_OUT,BULLET_2006_GYRO_PID_MAX_IOUT,0.0f,
				BULLET_2006_GYRO_PID_KP,BULLET_2006_GYRO_PID_KI,BULLET_2006_GYRO_PID_KD,0.0f,0.0f,0.000795774715459476f,0.0f,1,0x11);	

		//初始化偏转电机pid

		
    PID_Init(&init->Yaw_Motor_Angle_Pid,YAW_ANGLE_PID_MAX_OUT,BULLET_2006_ANGLE_PID_MAX_IOUT,0.0f,
				YAW_ANGLE_PID_KP,YAW_ANGLE_PID_KI,YAW_ANGLE_PID_KD,0.0f,0.0f,0.000795774715459476f,0.0f,5,0x11);	        //此处的角度最大值没改
		
    PID_Init(&init->Yaw_Motor_Gyro_Pid,YAW_GYRO_PID_MAX_OUT,BULLET_2006_GYRO_PID_MAX_IOUT,0.0f,
				YAW_GYRO_PID_KP,YAW_GYRO_PID_KI,YAW_GYRO_PID_KD,0.0f,0.0f,0.000795774715459476f,0.0f,5,0x11);	


			PID_Init(&init->Dart_2006_Yaw_Gyro_view_Pid,YAW_VIEW_GYRO_PID_MAX_OUT,YAW_VIEW_GYRO_PID_MAX_IOUT,0.0f,
				YAW_VIEW_GYRO_PID_KP,YAW_VIEW_GYRO_PID_KI,YAW_VIEW_GYRO_PID_KD,0.0f,0.0f,0.000795774715459476f,0.0f,1,0x11);		
				
		
		PID_Init(&init->Dart_2006_Yaw_Angle_view_Pid,YAW_VIEW_ANGLE_PID_MAX_OUT,YAW_VIEW_ANGLE_PID_MAX_IOUT,0.0f,
				YAW_VIEW_ANGLE_PID_KP,YAW_VIEW_ANGLE_PID_KI,YAW_VIEW_ANGLE_PID_KD,0.0f,0.0f,0.000795774715459476f,0.0f,1,0x11);		
				
		PID_Init(&init->Dart_6020_Gyro_Pid,MOTOR_6020_GYRO_PID_MAX_OUT,MOTOR_6020_GYRO_PID_MAX_IOUT,0.0f,
				MOTOR_6020_GYRO_PID_KP,MOTOR_6020_GYRO_PID_KI,MOTOR_6020_GYRO_PID_KD,0.0f,0.0f,0.000795774715459476f,0.0f,1,0x11);	

		PID_Init(&init->Dart_6020_Angle_Pid,MOTOR_6020_ANGLE_PID_MAX_OUT,MOTOR_6020_ANGLE_PID_MAX_IOUT,0.0f,
				MOTOR_6020_ANGLE_PID_KP,MOTOR_6020_ANGLE_PID_KI,MOTOR_6020_ANGLE_PID_KD,0.0f,0.0f,0.000795774715459476f,0.0f,1,0x11);

		//更新yaw电机中值
		//init->Dart_Yaw_Motor.offset_ecd = 4000;
		//更新电机数据
    dart_feedback_update(init);
		init->Dart_Yaw_Motor.offset_ecd = init->Dart_Yaw_Motor.motor_measure->ecd;
		init->Dart_2006_Bullet_Motor.offset_ecd = init->Dart_2006_Bullet_Motor.motor_measure->ecd;
		init->Dart_3508_Motor.offset_ecd = init->Dart_3508_Motor.motor_measure->ecd;
		init->Dart_6020_Motor.offset_ecd = init->Dart_6020_Motor.motor_measure->ecd;
}


void dart_feedback_update(dart_control_t *feedback_update)
{
    if (feedback_update == NULL)
    {
        return;
    }

		//更新3508电机数据
		feedback_update->Dart_3508_Motor.speed 				= feedback_update->Dart_3508_Motor.motor_measure->speed_rpm * RPM_MIN_TO_RAD_S;
		
		//更新发射2006电机数据
		feedback_update->Dart_2006_Bullet_Motor.speed =-feedback_update->Dart_2006_Bullet_Motor.motor_measure->speed_rpm * RPM_MIN_TO_RAD_S/REDUCTION_RATIO_2006;
		
		//更新yaw电机数据
		feedback_update->Dart_Yaw_Motor.speed 				= feedback_update->Dart_Yaw_Motor.motor_measure->speed_rpm * RPM_MIN_TO_RAD_S/REDUCTION_RATIO_2006;   

		//更新换弹6020
		feedback_update->Dart_6020_Motor.speed 				=-feedback_update->Dart_6020_Motor.motor_measure->speed_rpm * RPM_MIN_TO_RAD_S/REDUCTION_RATIO_6020;   //我知道为什么这里有负号,这是个不影响发射的bug,让大一的改哈哈
		
		//更新限位开关状态
		feedback_update->Dart_Flag.topLimitSwitch_Last 		= feedback_update->Dart_Flag.topLimitSwitch_Last;
		feedback_update->Dart_Flag.topLimitSwitch.flag  	= (HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_1) == GPIO_PIN_RESET);
		
		feedback_update->Dart_Flag.bottomLimitSwitch_Last = feedback_update->Dart_Flag.bottomLimitSwitch.flag ;
		feedback_update->Dart_Flag.bottomLimitSwitch.flag = (HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_0) == GPIO_PIN_RESET);
		
}


void shoot_flag_renew(dart_control_t *shoot_flag_renew)      //在这个函数中设置标志位
{
		if(shoot_flag_renew->Dart_Mode == DART_ZERO_FORCE)
		{
				//判断3508是否复位完成
				shoot_flag_renew->Dart_Flag.Reset_3508.flag  = 0;                                                   //这个变量 1是在上面 2是在下面 0是准备进行动作
				shoot_flag_renew->Dart_Flag.Reset_3508.count = 0;
				//判断发射2006是否复位完成
				shoot_flag_renew->Dart_Flag.Reset_Bullet_2006.flag  = 0;                                             //0表示未复位 1表示已复位
				shoot_flag_renew->Dart_Flag.Reset_Bullet_2006.count = 0;	
				//判断偏转2006是否复位完成
				shoot_flag_renew->Dart_Flag.Reset_Yaw_2006.flag  = 0;
				shoot_flag_renew->Dart_Flag.Reset_Yaw_2006.count = 0;	
				//判断是否成功换弹

				//判断是否完成发射
				shoot_flag_renew->Dart_Flag.Bullet_Done.flag  = 0;
				shoot_flag_renew->Dart_Flag.Bullet_Done.count = 0;

				//限位开关延迟判断
				shoot_flag_renew->Dart_Flag.topLimitSwitch_Last = 0;
				shoot_flag_renew->Dart_Flag.topLimitSwitch.flag  = 0;
				shoot_flag_renew->Dart_Flag.topLimitSwitch.count = 0;
				shoot_flag_renew->Dart_Flag.bottomLimitSwitch_Last = 0;
				shoot_flag_renew->Dart_Flag.bottomLimitSwitch.flag  = 0;
				shoot_flag_renew->Dart_Flag.bottomLimitSwitch.count = 0;
				//延迟清零
				shoot_flag_renew->Dart_Flag.waitingLock.flag = 0;
				shoot_flag_renew->Dart_Flag.waitingLock.count = 0;
				
				//复位清零
				shoot_flag_renew->Dart_Flag.Reset_all_2006_ready.flag = 0;
				shoot_flag_renew->Dart_Flag.Reset_3508_Angle = 0;
				//发射次数清0
				reloadPWM = upPWM;
				shoot_flag_renew->Shoot_Time = 0;
				auto_start = 0;
				force_start = 0;
				
				shoot_flag_renew->dart_reload_step = SERVO_READY;//换弹标志位重置
				shoot_flag_renew->Dart_servo_wait_down = 0;     //等待舵机下降计时
				shoot_flag_renew->Dart_motor_wait_turn = 0;			//等待电机就绪
				PULL_3508_SPEED_SET = RESET_3508_SPEED_SET_SLOW;//设置3508复进速度
				ANGLE_SET_6020 = RELOAD_6020_ANGLE0;						//设定6020角度为初始角度
		}
		else
		{			

						
         if(shoot_flag_renew->Shoot_Mode == SHOOT_RELOADING)
				 {
					
						shoot_flag_renew->Dart_Flag.waitingLock.count = 0;    //开始换弹时自动解锁扳机
				 }

						//判断发弹2006是否完成复位
						if(fabs(shoot_flag_renew->Dart_2006_Bullet_Motor.speed)<0.3f && shoot_flag_renew->Dart_Flag.Reset_Bullet_2006.flag == 0)  //用堵转判定复位 或许有一天会改为电流判定,让大一的写
						{
								shoot_flag_renew->Dart_Flag.Reset_Bullet_2006.count ++;
								if(shoot_flag_renew->Dart_Flag.Reset_Bullet_2006.count > dart_Bullet_ready/DART_CONTROL_TIME)                                          
								{
										shoot_flag_renew->Dart_Flag.Reset_Bullet_2006.flag  = 1;
										shoot_flag_renew->Dart_Flag.Reset_Bullet_2006.count = 0;
										dart_angle_clear(&shoot_flag_renew->Dart_2006_Bullet_Motor);
								}
						}
						else
						{
										shoot_flag_renew->Dart_Flag.Reset_Bullet_2006.count = 0;
						}
												
						//判断偏转2006是否复位
  			
							if(fabs(shoot_flag_renew->Dart_Yaw_Motor.speed)<0.3f) 
							{
										shoot_flag_renew->Dart_Flag.Reset_Yaw_2006.count ++;
										if(shoot_flag_renew->Dart_Flag.Reset_Yaw_2006.count > dart_yaw_ready/DART_CONTROL_TIME)  //设定堵转时间
										{
										shoot_flag_renew->Dart_Flag.Reset_Yaw_2006.flag  = 1;
										shoot_flag_renew->Dart_Flag.Reset_Yaw_2006.count = 0;
										dart_angle_clear(&shoot_flag_renew->Dart_Yaw_Motor);	
										}
				
							}
							else
							{
										
										shoot_flag_renew->Dart_Flag.Reset_Yaw_2006.count = 0;
							}
						
							if(dart_motor_check(&shoot_flag_renew->Dart_Yaw_Motor,Yaw_Angle,DART_YAW_SENSITIVE))//判断yaw2006是否到位置
							{
										shoot_flag_renew->Dart_Flag.Reset_all_2006_ready.flag = 1;
							}
						
				
					
				
		}
	}

void shoot_set_mode(dart_control_t *shoot_set_mode)           //只在这个函数中设置Shoot_Mode(当前发射状态)
{

    if (shoot_set_mode == NULL)
    {
        return;
    }
		
    static int8_t last_s = RC_SW_UP;
		
		switch(shoot_set_mode->Dart_Mode)          //镖架准备状态设置
		{
			case DART_ZERO_FORCE:
				shoot_set_mode->Shoot_Mode = SHOOT_STOP;
				return ;
			break;
			
			case DART_OUTPOST:
				shoot_set_mode->Dart_2006_Bullet_Motor.angle_set = dart_2006_angle_set;		
			  shoot_set_mode->Dart_Yaw_Motor.angle_set = Yaw_Angle;										//其实这样的写法是不好的,但是由于后面会被pid覆盖掉,所以不影响
			break;	
		}
		
		switch(shoot_set_mode->Shoot_Mode)
		{
			case SHOOT_STOP:
				if(shoot_set_mode->Dart_Flag.Reset_all_2006_ready.flag == 1)
				{
					shoot_set_mode->Shoot_Mode = SHOOT_UPING;             //复位
				}
			break;
			case SHOOT_UPING:
				if(shoot_set_mode->Dart_Flag.Reset_3508_Angle == 1)
				{
					shoot_set_mode->Shoot_Mode = SHOOT_READY_3508_AND_2006;    //如果已经复位过了就不再尝试复位
				}
				else if(shoot_set_mode->Dart_Flag.topLimitSwitch.flag == 1)
				{
					shoot_set_mode->Dart_Flag.topLimitSwitch.count ++;
					
					if(shoot_set_mode->Dart_Flag.topLimitSwitch.count > dart_3508_reset / DART_CONTROL_TIME)
					{
								shoot_set_mode->Shoot_Mode = SHOOT_READY_3508_AND_2006;              //3508准备完成
								shoot_set_mode->Dart_Flag.topLimitSwitch.flag = 0;
								shoot_set_mode->Dart_Flag.topLimitSwitch.count = 0;
								dart_angle_clear(&shoot_set_mode->Dart_3508_Motor);
								shoot_set_mode->Dart_Flag.Reset_3508_Angle = 1;
					}
				}
				
			break;
			
			case SHOOT_RELOADING:

				if(shoot_set_mode->Dart_Flag.bottomLimitSwitch.flag == 1)
					{
									shoot_set_mode->Dart_Flag.bottomLimitSwitch.count ++;
							if(shoot_set_mode->Dart_Flag.bottomLimitSwitch.count > dart_servo_lock_ready/DART_CONTROL_TIME)
							{
									triggerPWM = lockPWM;                                                        //锁定
							}								
								
							if(shoot_set_mode->Dart_Flag.bottomLimitSwitch.count > dart_lock_ready /DART_CONTROL_TIME)
							{
											shoot_set_mode->Shoot_Mode = SHOOT_FINISH_PULL;
											shoot_set_mode->Dart_Flag.bottomLimitSwitch.flag = 0;
											shoot_set_mode->Dart_Flag.bottomLimitSwitch.count = 0;
							}						
					}		

			break;
			
			case SHOOT_FINISH_PULL:
					if(dart_motor_check(&shoot_set_mode->Dart_3508_Motor,shoot_set_mode->Dart_3508_Motor.angle_set,DART_3508_SENSITIVE))//判断3508是否复位完成    这是发弹前复位
						{																														
																																	
								shoot_set_mode->Dart_Flag.Reset_3508.count ++;                                 //其实是可以不计时的,但是我们需要考虑受到干扰的情况
								if(shoot_set_mode->Dart_Flag.Reset_3508.count > dart_3508_reset/DART_CONTROL_TIME)
								{
										shoot_set_mode->Shoot_Mode = SHOOT_READY_BULLET;
										shoot_set_mode->Dart_Flag.Reset_3508.flag  = 1;
										shoot_set_mode->Dart_Flag.Reset_3508.count = 0;
								}										
						}
						else
						{
								shoot_set_mode->Dart_Flag.Reset_3508.count = 0;
						}	

						break;
			
			case SHOOT_READY_BULLET:
#if AUTO_RELOAD_TEST
			
#else
							if(switch_is_down(shoot_set_mode->Dart_Rc_Ctrl->rc.s[SHOOT_MODE_CHANNEL]) && !switch_is_down(last_s))
							{	
								//遥控器从左中拨到左下扣动扳机发射
									shoot_set_mode->Shoot_Mode = SHOOT_BULLET;				
							}	
#endif
							break;
			
			case SHOOT_BULLET:
							shoot_set_mode->Dart_Flag.Bullet_Done.flag++;
						if(shoot_set_mode->Dart_Flag.Bullet_Done.flag > dart_shoot_wait/DART_CONTROL_TIME)	//对于发射完成的计时   神秘发射计时变量 可以不让osDelay阻塞
						{
							shoot_set_mode->Shoot_Mode = SHOOT_STOP;	
							shoot_set_mode->Dart_Flag.Bullet_Done.flag = 0;
							shoot_set_mode->Shoot_Time += 1;
						}		
						break;
			
		}

				
#if AUTO_RELOAD_TEST

	dart_auto_reload(shoot_set_mode,last_s);
	judge_set_shoot(shoot_set_mode->Shoot_Time);//哼哈二将
#else 			

				if(switch_is_up(shoot_set_mode->Dart_Rc_Ctrl->rc.s[SHOOT_MODE_CHANNEL]) && 
					      !switch_is_up(last_s) 
						&& shoot_set_mode->Shoot_Mode == SHOOT_READY_3508_AND_2006)
			  {                                   
						shoot_set_mode->Shoot_Mode = SHOOT_RELOADING;   
						
				}
							

#endif			
		last_s = shoot_set_mode->Dart_Rc_Ctrl->rc.s[SHOOT_MODE_CHANNEL];//这行代码的位置不能随便移动,必须在末尾
}


void dart_set_mode(dart_control_t *set_mode)
{
    if (set_mode == NULL)
    {
        return;
    }
		
		set_mode->Dart_Mode_Last = set_mode->Dart_Mode;
		
		
		//开关控制 飞镖状态
		//右下是无力模式
    if (switch_is_down(set_mode->Dart_Rc_Ctrl->rc.s[DART_MODE_CHANNEL]))
    {       
				set_mode->Dart_Mode= DART_ZERO_FORCE;
    }
		//右中是前哨站模式
    else if (switch_is_mid(set_mode->Dart_Rc_Ctrl->rc.s[DART_MODE_CHANNEL]))
    {
			set_mode->Dart_Mode = DART_OUTPOST;
			
    }
		//右上是基地模式
    else if (switch_is_up(set_mode->Dart_Rc_Ctrl->rc.s[DART_MODE_CHANNEL]))
    {
			
    }
		
		//发射标志位判断
		shoot_flag_renew(set_mode);
		
		//发射模式判断
		shoot_set_mode(set_mode);
}



void dart_set_control(dart_control_t *set_control)                //规定只在此处设置电机速度和角度
{
    if (set_control == NULL)
    { 
        return;
    }
		
		if(set_control->Dart_Mode == DART_ZERO_FORCE)             //这里没有必要,但是为了安全性考虑还是留着
		{
				force_start = 0;
				//发射3508
				set_control->Dart_3508_Motor.speed_set = 0.0f;
				//发射2006
				set_control->Dart_2006_Bullet_Motor.angle_set = 0.0f;
				set_control->Dart_2006_Bullet_Motor.speed_set = 0.0f;
		    set_control->Dart_2006_Bullet_Motor.give_current = 0.0f;

				//偏转2006

				set_control->Dart_Yaw_Motor.speed_set = 0.0f;
		    set_control->Dart_Yaw_Motor.give_current = 0.0f;
		}
		else
		{

			
			switch(set_control->Shoot_Mode)
      {	
					
				case SHOOT_STOP  :
						force_start = 0;

						if(set_control->Dart_Flag.Reset_Bullet_2006.flag == 0)  //没有初始化时,设置两个电机的速度
						{
								set_control->Dart_2006_Bullet_Motor.speed_set = BULLET_2006_RESERT_SPEED_SET;	
						}
						if(set_control->Dart_Flag.Reset_Yaw_2006.flag == 0)      
						{
							set_control->Dart_Yaw_Motor.speed_set = Yaw_2006_RESERT_SPEED_SET;   
						}
						
							set_control->Dart_3508_Motor.give_current = 0.0f;
							triggerPWM = unlockPWM;	
							reloadPWM = upPWM;
				break;
				case SHOOT_READY_3508_AND_2006:
						ANGLE_SET_3508 = PULL_3508_ANGLE_START;
				break;
				case SHOOT_UPING:
					set_control->Dart_3508_Motor.speed_set =  100;
					break;
				
				case SHOOT_RELOADING:
				{
#if AUTO_RELOAD_TEST
					if(set_control->Shoot_Time == 0)
					{
							ANGLE_SET_3508 = PULL_3508_ANGLE_DOWN;
					}
					else if(auto_start == 1)
					{
						if(set_control->dart_reload_step == SERVO_READY)               //优美的石山
						{
								ANGLE_SET_3508 = PULL_3508_P1;
								if(dart_motor_check(&set_control->Dart_3508_Motor,ANGLE_SET_3508,DART_3508_SENSITIVE))
								{
										ANGLE_SET_6020 = dart_6020_angle_set(set_control->Shoot_Time,set_control->dart_reload_step);  //3508到位则启动6020,同时开启计时
										set_control->Dart_motor_wait_turn++;                    //计时变量,实现非堵塞计时
									if(set_control->Dart_motor_wait_turn > dart_turn_ready/DART_CONTROL_TIME)
									{
											reloadPWM = downPWM;
											set_control->Dart_servo_wait_down++;
											if(set_control->Dart_servo_wait_down > dart_down_ready1/DART_CONTROL_TIME)						//等待舵机
											{
												set_control->dart_reload_step = SERVO_UP;
												set_control->Dart_servo_wait_down = 0;
												set_control->Dart_motor_wait_turn = 0;
											}
												
									}
									
								}
								
						}
						else if(set_control->dart_reload_step == SERVO_UP)
						{
								
								ANGLE_SET_3508 = PULL_3508_P2;
								if(dart_motor_check(&set_control->Dart_3508_Motor,ANGLE_SET_3508,DART_3508_SENSITIVE))
								{
										reloadPWM = upPWM;                                                                    
										set_control->Dart_servo_wait_down++;
										if(set_control->Dart_servo_wait_down > dart_down_ready2/DART_CONTROL_TIME)							//等待舵机
										{
												ANGLE_SET_3508 = PULL_3508_ANGLE_DOWN;                 //开始下拉
												set_control->Dart_servo_wait_down = 0;									
												set_control->dart_reload_step = SERVO_PULL;   //不再重入此函数
										}
									
								}
						}//if	
					}//case
#else
					ANGLE_SET_3508 = PULL_3508_ANGLE_DOWN;      //认为已经换完弹了 单发模式
					
#endif					
					
					break;
				}

				case SHOOT_FINISH_PULL :                                          //上弹完成,准备复位3508
					{
						

						ANGLE_SET_6020 = dart_6020_angle_set(set_control->Shoot_Time,set_control->dart_reload_step);
						set_control->dart_reload_step = SERVO_UPING;  //复位
					
						triggerPWM = lockPWM;  
						set_control->Dart_Flag.waitingLock.count++;
          
						ANGLE_SET_3508 = PULL_3508_ANGLE_UP;
					if(set_control->Dart_Flag.waitingLock.count <= dart_3508_wait_slow/DART_CONTROL_TIME)
						{
							set_control->Dart_3508_Angle_Pid.MaxOut = BULLET_3508_ANGLE_PID_MAX_OUT/5;
						}
						
						else if(set_control->Dart_Flag.waitingLock.count >= dart_3508_wait_slow/DART_CONTROL_TIME && set_control->Dart_Flag.waitingLock.count <= dart_3508_wait_fast/DART_CONTROL_TIME)
						{
							set_control->Dart_3508_Angle_Pid.MaxOut = BULLET_3508_ANGLE_PID_MAX_OUT;
						}

             break;
					}
									
				case SHOOT_READY_BULLET :
						{						
					  	
							break;
						}
				case SHOOT_BULLET:
					{
#if FORCE_CONTROL
						set_control->Dart_2006_Bullet_Force_ANGLE_Pid.Iout = 0;
#endif
						triggerPWM = unlockPWM; 
						set_control->Dart_3508_Motor.speed_set = 0.0f;  //保险
						set_control->dart_reload_step = SERVO_READY;
            break;					
					}
				}
			set_control->Dart_6020_Motor.angle_set =  ANGLE_SET_6020;
			set_control->Dart_3508_Motor.angle_set =  ANGLE_SET_3508;
		}
}

void dart_control_loop(dart_control_t *control_loop)
{
    if (control_loop == NULL)
    {
        return;
    }
		
		if(control_loop->Dart_Mode == DART_ZERO_FORCE)
		{
				//3508电机
				control_loop->Dart_3508_Motor.give_current = 0;
				//发弹2006
				control_loop->Dart_2006_Bullet_Motor.give_current = 0;
				//偏转2006
				control_loop->Dart_Yaw_Motor.give_current = 0;
			
				control_loop->Dart_6020_Motor.give_current = 0;


		}
		else
		{		
			 control_loop->Dart_6020_Motor.speed_set = PID_Calculate(&control_loop->Dart_6020_Angle_Pid,
																		control_loop->Dart_6020_Motor.angle,control_loop->Dart_6020_Motor.angle_set);
					
				control_loop->Dart_6020_Motor.give_current = -PID_Calculate(&control_loop->Dart_6020_Gyro_Pid,
																		control_loop->Dart_6020_Motor.speed,control_loop->Dart_6020_Motor.speed_set);		
				//3508电机复位完，就无力，防止电机堵转过久导致电机过热
				
				
			 
				if( control_loop->Shoot_Mode == SHOOT_UPING)//3508复位
				{
						control_loop->Dart_3508_Motor.give_current = int16_constrain((int16_t)PID_Calculate(&control_loop->Dart_3508_Gyro_Pid,
						control_loop->Dart_3508_Motor.speed,control_loop->Dart_3508_Motor.speed_set),-10000,10000);						                      //输出限幅 以此改变上弹的力 16384
				}
				else if(control_loop->Dart_Flag.Reset_all_2006_ready.flag == 1)      //复位完成才能动
				{
						
					  control_loop->Dart_3508_Motor.speed_set = int16_constrain((int16_t)PID_Calculate(&control_loop->Dart_3508_Angle_Pid,
																		control_loop->Dart_3508_Motor.angle,control_loop->Dart_3508_Motor.angle_set),-600,600);		
					
						control_loop->Dart_3508_Motor.give_current = (int16_t)PID_Calculate(&control_loop->Dart_3508_Gyro_Pid,
																		control_loop->Dart_3508_Motor.speed,control_loop->Dart_3508_Motor.speed_set);		
											
				}
				else//保险
				{
						control_loop->Dart_3508_Motor.give_current = 0;					
				}
			

				//发弹2006
				if(control_loop->Dart_Flag.Reset_Bullet_2006.flag == 1)
				{
#if FORCE_CONTROL
						if(force_start == 1 && force_OK == 0)
						{
							control_loop->Dart_2006_Bullet_Motor.speed_set =int16_constrain((int16_t) PID_Calculate(&control_loop->Dart_2006_Bullet_Force_ANGLE_Pid,force_mv,force_mv_set),-50,50);   //过后可以尝试串级pid或者ADRC

							control_loop->Dart_2006_Bullet_Motor.give_current = PID_Calculate(&control_loop->Dart_2006_Bullet_Force_GYRO_Pid,control_loop->Dart_2006_Bullet_Motor.speed
																								,control_loop->Dart_2006_Bullet_Motor.speed_set);   //过后可以尝试串级pid或者ADRC

						}
						else
#endif
						{
					  control_loop->Dart_2006_Bullet_Motor.speed_set = int16_constrain((int16_t)PID_Calculate(&control_loop->Dart_2006_Bullet_Angle_Pid,
																		control_loop->Dart_2006_Bullet_Motor.angle,control_loop->Dart_2006_Bullet_Motor.angle_set),-100,100);		
					
						control_loop->Dart_2006_Bullet_Motor.give_current = (int16_t)PID_Calculate(&control_loop->Dart_2006_Bullet_Gyro_Pid,
																		control_loop->Dart_2006_Bullet_Motor.speed,control_loop->Dart_2006_Bullet_Motor.speed_set);		
						}
				}
				else      //2006复位			
				{
						control_loop->Dart_2006_Bullet_Motor.give_current = int16_constrain((int16_t)PID_Calculate(&control_loop->Dart_2006_Bullet_Gyro_Pid,
																		control_loop->Dart_2006_Bullet_Motor.speed,control_loop->Dart_2006_Bullet_Motor.speed_set),-4000,4000);					
				}														

				
				
			if( view_ok == 0	|| (VIEW_NX == 0) || (control_loop->Dart_Flag.Reset_Yaw_2006.flag == 0)
#if REFEREE_START
					|| (referee_game_start != 4)
#endif
	   	)      //不要随便动这里的代码
				{
				//偏转2006
						if(control_loop->Dart_Flag.Reset_Yaw_2006.flag == 1)
						{
						control_loop->Dart_Yaw_Motor.speed_set = PID_Calculate(&control_loop->Yaw_Motor_Angle_Pid,
																		control_loop->Dart_Yaw_Motor.angle,control_loop->Dart_Yaw_Motor.angle_set);
						control_loop->Dart_Yaw_Motor.give_current = PID_Calculate(&control_loop->Yaw_Motor_Gyro_Pid,
																		control_loop->Dart_Yaw_Motor.speed,control_loop->Dart_Yaw_Motor.speed_set);						
						}
						else if(control_loop->Dart_Flag.Reset_Yaw_2006.flag == 0)
						{
						control_loop->Dart_Yaw_Motor.give_current = PID_Calculate(&control_loop->Yaw_Motor_Gyro_Pid,
																		control_loop->Dart_Yaw_Motor.speed,control_loop->Dart_Yaw_Motor.speed_set);					        //速度PID    首先回正
						}
	
				}				
				else if(control_loop->Dart_Flag.Reset_Yaw_2006.flag == 1)//启动自瞄
				{													
					   control_loop->Dart_Yaw_Motor.speed_set =PID_Calculate(&control_loop->Dart_2006_Yaw_Angle_view_Pid,view_temp,0); 
								control_loop->Dart_Yaw_Motor.give_current = PID_Calculate(&control_loop->Dart_2006_Yaw_Gyro_view_Pid,
																		control_loop->Dart_Yaw_Motor.speed,control_loop->Dart_Yaw_Motor.speed_set);	

				}									

		}		

}



#if AUTO_RELOAD_TEST
void dart_auto_reload(dart_control_t * dart_reload,int last_switch)                     //此处为连发逻辑
{
		if(switch_is_up(dart_reload->Dart_Rc_Ctrl->rc.s[SHOOT_MODE_CHANNEL]) && 
					  !switch_is_up(last_switch) 
				    && auto_start == 0)
		{                                                           					//将上弹改为自动发弹模式
				 auto_start = 1;                                       
		}
		

		
		if(auto_start == 1)
		{
#if REFEREE_START

					if(referee_game_start != 4)
					{
						auto_start = 0;
						return;
					}
#endif
					if((dart_reload->Shoot_Time < SHOOT_TIME_SET + 1) && (dart_reload->Shoot_Mode == SHOOT_STOP || dart_reload->Shoot_Mode == SHOOT_READY_3508_AND_2006))                    
					{
						dart_reload->Shoot_Mode = SHOOT_RELOADING; //决定开始下拉
					}
					else if(dart_reload->Shoot_Time == SHOOT_TIME_SET + 1)
					{
						dart_reload->Shoot_Time = 0;
						auto_start = 0;
						ANGLE_SET_6020 = RELOAD_6020_ANGLE0;
						dart_reload->Shoot_Mode = SHOOT_STOP;
						return;
					}

											
					if(dart_reload->Shoot_Mode == SHOOT_READY_BULLET 
#if REFEREE_START	
					&& referee_door_open == 1

#endif
						)
					            //如果完成换弹或者为第一次发射就直接发射
					{					
							dart_reload->Shoot_Mode = SHOOT_BULLET;
							referee_door_open = 0;						           //发射结束后置开门标志位为零
					}
					
							
		}
							auto_start_last = auto_start;

}

#endif
