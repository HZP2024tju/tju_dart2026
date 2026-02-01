#ifndef __DART_TASK_H
#define __DART_TASK_H

#include "remote_control.h"
#include "struct_typedef.h"
#include "CAN_receive.h"
#include "controller.h"
#include "arm_math.h"
#include "servo.h"
#include "view.h"

#define tim_lock htim8
#define channel_lock TIM_CHANNEL_1

#define tim_down htim8
#define channel_down TIM_CHANNEL_2

//任务初始化 空闲一段时间
#define DART_TASK_INIT_TIME 500
//任务周期

#define DART_CONTROL_TIME 1

//电机码盘值最大以及中值
#define HALF_ECD_RANGE  4096   //0x1000
#define ECD_RANGE       8191	 //0x2000 -1
#define MOTOR_ECD_TO_RAD 0.000766990394f //      2*  PI  /8192
//3508和2006电机转速转换
#define RPM_MIN_TO_RAD_S 2*PI/60.0f
#define REDUCTION_RATIO_3508 3591.0f/187.0f
#define REDUCTION_RATIO_2006 36.0f

#define REDUCTION_RATIO_6020 1
//编码器值转换为弧度的系数
#define MOTOR_ECD_TO_ANGLE    0.0007669903939f 
//遥控器右上拨杆
#define DART_MODE_CHANNEL 0
//射击发射开关通道数据
#define SHOOT_MODE_CHANNEL 1

/*********************   一些等待时间   ********************/
#define dart_servo_lock_ready  10            //等待下拉完全后自锁
#define dart_lock_ready 			 270						//等待舵机自锁
#define dart_shoot_wait 		600										//等待发射结束
#define dart_down_ready1   400								//等待舵机下降
#define dart_down_ready2   470	
#define dart_turn_ready   700								//等待6020到位
#define dart_yaw_ready   	50									//yaw堵转复位时间
#define dart_3508_reset   25									//等待3508复位

#define dart_3508_wait_slow 400               //3508慢速时间
#define dart_3508_wait_fast 1420
/*********************   一些等待时间   ********************/


//上弹3508回到初始位置的两个速度设定值
#define RESET_3508_SPEED_SET_SLOW    150.0f
#define RESET_3508_SPEED_SET    950.0f


//发射2006电机需要的位置和速度
#define BULLET_2006_RESERT_SPEED_SET	50.0f
//偏转2006电机需要的位置和速度
#define Yaw_2006_RESERT_SPEED_SET	30.0f


//装弹2006电机需要的位置和速度
#define RELOAD_2006_RESET_SPEED_SET	 -20.0f

//3508速度环pid
#define MOTOR_3508_GYRO_PID_KP 200.0f
#define MOTOR_3508_GYRO_PID_KI 0.0001f  
#define MOTOR_3508_GYRO_PID_KD 0.01f	 
#define MOTOR_3508_GYRO_PID_MAX_OUT  16383.0f
#define MOTOR_3508_GYRO_PID_MAX_IOUT 2000.0f

//3508 角度环 角度由编码器 
#define BULLET_3508_ANGLE_PID_KP        400.3f
#define BULLET_3508_ANGLE_PID_KI        0.00001f
#define BULLET_3508_ANGLE_PID_KD        0.001f
#define BULLET_3508_ANGLE_PID_MAX_OUT   600.0f
#define BULLET_3508_ANGLE_PID_MAX_IOUT  20.0f

//设定3508敏感值
#define DART_3508_SENSITIVE 0.36 

//上弹3508的角度
#define PULL_3508_ANGLE_DOWN	-17.42f
#define PULL_3508_ANGLE_UP 		0.0001f
#define PULL_3508_ANGLE_START -2.f
#define PULL_3508_P1				 	-8.74
#define PULL_3508_P2 					-7.7



//发射2006 角度环 角度由编码器 
#define BULLET_2006_ANGLE_PID_KP        14.3f
#define BULLET_2006_ANGLE_PID_KI        0.000001f
#define BULLET_2006_ANGLE_PID_KD        0.0001f
#define BULLET_2006_ANGLE_PID_MAX_OUT   50.0f
#define BULLET_2006_ANGLE_PID_MAX_IOUT  0.0f
//发射2006 速度环
#define BULLET_2006_GYRO_PID_KP        3200.0f
#define BULLET_2006_GYRO_PID_KI        0.0f
#define BULLET_2006_GYRO_PID_KD        0.0f
#define BULLET_2006_GYRO_PID_MAX_OUT   10000.0f
#define BULLET_2006_GYRO_PID_MAX_IOUT  0.0f
#define BULLET_2006_RESET_PID_MAX_OUT  2000.0f
#define BULLET_2006_RESET_PID_MAX_IOUT 0.0f

//yaw 角度环 角度由编码器 
#define YAW_ANGLE_PID_KP        2.f
#define YAW_ANGLE_PID_KI        0.000f
#define YAW_ANGLE_PID_KD        0.f // 0,7
#define YAW_ANGLE_PID_MAX_OUT   10000 //2.0f
#define YAW_ANGLE_PID_MAX_IOUT  0.0f
//yaw 速度环
#define YAW_GYRO_PID_KP        4000.0f
#define YAW_GYRO_PID_KI        0.00000f
#define YAW_GYRO_PID_KD        0.01f
#define YAW_GYRO_PID_MAX_OUT   16000.0f
#define YAW_GYRO_PID_MAX_IOUT  3000.0f

#define DART_YAW_SENSITIVE 1


#define YAW_VIEW_ANGLE_PID_KP        0.6f
#define YAW_VIEW_ANGLE_PID_KI        0.000f
#define YAW_VIEW_ANGLE_PID_KD        0.001f // 0,7
#define YAW_VIEW_ANGLE_PID_MAX_OUT   4000 //2.0f
#define YAW_VIEW_ANGLE_PID_MAX_IOUT  0.0f
//yaw 速度环
#define YAW_VIEW_GYRO_PID_KP        4000.0f
#define YAW_VIEW_GYRO_PID_KI        0.00000f
#define YAW_VIEW_GYRO_PID_KD        0.00001f
#define YAW_VIEW_GYRO_PID_MAX_OUT   16000.0f
#define YAW_VIEW_GYRO_PID_MAX_IOUT  3000.0f

//发射2006力反馈
#define FORCE_BULLET_2006_GYRO_PID_KP        3200.0f
#define FORCE_BULLET_2006_GYRO_PID_KI        0.000f
#define FORCE_BULLET_2006_GYRO_PID_KD        0.04000f
#define FORCE_BULLET_2006_GYRO_PID_MAX_OUT   10000.0f
#define FORCE_BULLET_2006_GYRO_PID_MAX_IOUT  7000.0f

#define FORCE_BULLET_2006_ANGLE_PID_KP        0.60003
#define FORCE_BULLET_2006_ANGLE_PID_KI        1.009999976f
#define FORCE_BULLET_2006_ANGLE_PID_KD        0.000109999986f
#define FORCE_BULLET_2006_ANGLE_PID_MAX_OUT   13000.0f
#define FORCE_BULLET_2006_ANGLE_PID_MAX_IOUT  7000.0f


//换弹6020

#define MOTOR_6020_GYRO_PID_KP        300.0f
#define MOTOR_6020_GYRO_PID_KI        0.000f
#define MOTOR_6020_GYRO_PID_KD        0.0200f
#define MOTOR_6020_GYRO_PID_MAX_OUT   7000.0f
#define MOTOR_6020_GYRO_PID_MAX_IOUT  7000.0f

#define MOTOR_6020_ANGLE_PID_KP        320.6f
#define MOTOR_6020_ANGLE_PID_KI        0.08f
#define MOTOR_6020_ANGLE_PID_KD        0.4f
#define MOTOR_6020_ANGLE_PID_MAX_OUT   14.5f
#define MOTOR_6020_ANGLE_PID_MAX_IOUT  20.0f

//换弹6020的各个角度 , 以第一个发射角度为基准
#define RELOAD_6020_ANGLE0   -1.03919753 
#define RELOAD_6020_ANGLE60  -2.087
#define RELOAD_6020_ANGLE120 -3.134197533333333
#define RELOAD_6020_ANGLE180 0
#define RELOAD_6020_ANGLE240 0
#define RELOAD_6020_ANGLE300 0
#define RELOAD_6020_ANGLE360 0
//60 180 300 是换弹角度,需要标定
//设定6020敏感度(与设定角度差多少时可以开始下一次行动)
#define DART_6020_SENSITIVE 0.2


//测试模式下定义为0，比赛模式下定义为1
//测试模式：拨一下左上上弹,拨一下左下扣动扳机发射，完成发射流程
//比赛模式：拨一下左上上弹,之后全自动完成发射流程

//自动换弹模式:只需要左上一次即可连发四次
#define AUTO_RELOAD_TEST 	1
#define FORCE_CONTROL 		0
#define VIEW 							0
#define REFEREE_START 		0
#define Reload_mode_test  0
#define SHOOT_TIME_SET    1	//设定第一发结束后还要打多少次
//一个流程全部的标志位
typedef enum
{
    SHOOT_STOP = 0,
		SHOOT_START,//3508，2006开始复位
    SHOOT_READY_3508_AND_2006,//就位模式，3508下拉，2006就位
    SHOOT_FINISH_PULL,//3508发射前复位
		SHOOT_UPING,
		SHOOT_RELOADING,
		SHOOT_DOWN_BULLET,
		SHOOT_RESET,
		SHOOT_RELOAD_FINISH,
    SHOOT_READY_BULLET,
    SHOOT_BULLET,
    SHOOT_BULLET_DONE,
} shoot_mode_e;

typedef enum
{
    SERVO_READY = 100,
		SERVO_UP  	= 101,
		SERVO_PULL  = 102,
		SERVO_UPING = 103,
		SERVO_STAY  = 104,
} servo_mode_e;

//遥控器不同模式
typedef enum
{
  DART_ZERO_FORCE = 0,    
  DART_OUTPOST, 
  DART_BASE,  
} dart_behaviour_e;

//角度环PID结构体
typedef struct
{
    fp32 kp;
    fp32 ki;
    fp32 kd;
    fp32 set;
    fp32 get;
    fp32 err;
    fp32 max_out;
    fp32 max_iout;
    fp32 Pout;
    fp32 Iout;
    fp32 Dout;
    fp32 out;
} gimbal_PID_t;

typedef struct
{
  const motor_measure_t *motor_measure;
  fp32 angle;
  fp32 angle_set;
  fp32 speed;
  fp32 speed_set;
	uint16_t offset_ecd;
	int  round_cnt;
  int16_t give_current;
}Motor_t;




typedef struct
{
  uint16_t count;
	uint16_t  flag;
}Flag_t;

typedef struct
{
	  Flag_t Reset_Yaw_2006;
		Flag_t Reset_3508; //3508置顶
		Flag_t Reset_Bullet_2006; //
		Flag_t Reload_Done; 
		Flag_t Bullet_Done; //3508下拉完成
		Flag_t Reset_all_2006_ready;
		Flag_t waitingLock;
		Flag_t topLimitSwitch; //
		uint8_t topLimitSwitch_Last;
		Flag_t bottomLimitSwitch; //
		uint8_t bottomLimitSwitch_Last;
		uint8_t force_ok;
}Dart_Flag_t;

//云台结构体
typedef struct
{
	  dart_behaviour_e Dart_Mode_Last;
    dart_behaviour_e Dart_Mode;
		shoot_mode_e 		 Shoot_Mode;
    const RC_ctrl_t  *Dart_Rc_Ctrl;
	
    Motor_t   	 Dart_Yaw_Motor;  //yaw电机
		Motor_t			 Dart_3508_Motor;
		Motor_t    	 Dart_2006_Bullet_Motor;
		Motor_t      Dart_6020_Motor;
	
		PID_t 		 			 Yaw_Motor_Angle_Pid;
		PID_t 					 Yaw_Motor_Gyro_Pid;  
	
		PID_t 					 Dart_3508_Gyro_Pid;	
		PID_t 					 Dart_3508_Angle_Pid;	

		PID_t 					 Dart_2006_Bullet_Angle_Pid;  	
		PID_t 					 Dart_2006_Bullet_Gyro_Pid;  
	
		PID_t            Dart_2006_Bullet_Force_ANGLE_Pid;
	  PID_t            Dart_2006_Bullet_Force_GYRO_Pid;
	
	  PID_t            Dart_2006_Yaw_Angle_view_Pid;
	  PID_t            Dart_2006_Yaw_Gyro_view_Pid;
	
		PID_t            Dart_6020_Angle_Pid;
	  PID_t            Dart_6020_Gyro_Pid;
		Dart_Flag_t 		 Dart_Flag;
	
		uint8_t          Shoot_Time;
		
		float            Dt;
		uint32_t         Dwt_Count;
		
		uint32_t       	 Dart_servo_wait_down;
		uint32_t       	 Dart_motor_wait_turn;

		uint8_t 				 dart_reload_step;
		float            force;
}dart_control_t;

extern void dart_task(void const * argument);
extern dart_control_t dart_control;

void dart_set_control(dart_control_t *set_control);
void dart_control_loop(dart_control_t *control_loop);

extern int16_t testCurrent3508;
extern int16_t testCurrent2006;
extern fp32 dart_2006_angle_set;
extern float view_temp;
extern float force_mv;
extern uint8_t auto_start;
extern uint8_t view_ok;
extern uint8_t referee_game_start;

extern uint8_t view_reflash;


extern uint8_t referee_door_open;
void dart_auto_reload(dart_control_t * dart_reload,int last_switch) ;
void dart_angle_clear(Motor_t* motor); 
int dart_motor_check(Motor_t* motor,float target_angle,float sensitive);  //检测电机是否转到指定角度,需要设定敏感值


#endif
