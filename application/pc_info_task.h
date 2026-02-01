#ifndef __PC_INFO_TASK_H
#define __PC_INFO_TASK_H
#include "stm32f4xx.h"
#include "struct_typedef.h"
#define SOF_ADDR 	 0			//帧头多项式字节偏移量

#define FRAME_HEADER 		0xA5//帧头多项式
#define LEN_FRAME_HEADER 	2	//帧头长度
#define	LEN_TX_DATA 		15	//发送数据段长度
#define	LEN_RX_DATA 		14	//接收数据段长度
#define	LEN_FRAME_TAILER 	2	//帧尾CRC16
#define	LEN_TX_PACKET		19	//发送包整包长度
#define	LEN_RX_PACKET	  18	//接收包整包长度

#define	LEN_TX_DATA_big_camera 		4	//大相机C板发送数据段长度
#define	LEN_RX_DATA_big_camera 		4	//大相机C板接收数据段长度
#define	LEN_TX_PACKET_big_camera		8	//大相机C板发送包整包长度
#define	LEN_RX_PACKET_big_camera	  8	//大相机C板接收包整包长度
//帧头结构体
typedef __packed struct
{
	uint8_t  	sof;			// 帧头多项式
	uint8_t  	crc8;			// CRC8校验码
} frame_header_t;
//帧尾结构体
typedef __packed struct 
{
	uint16_t crc16;				// CRC16校验码
} frame_tailer_t;
//发送数据结构体
typedef __packed struct 
{
	float curr_yaw;         	//当前云台yaw角度
	float curr_pitch;       	//当前云台pitch角
	uint8_t state;          	//当前状态，自瞄-大符-小符
	uint8_t IsAutoAim;
	uint8_t enemy_color;    	//敌方颜色
	float shoot_speed;
}tx_data_t;
//接受数据结构体
typedef __packed struct 
{
	uint8_t fire;            	//发射指令 
	float shoot_yaw;       		//最终偏转角
	float shoot_pitch;	
	float shoot_speed_avg;
	uint8_t food;							//狗粮
}rx_data_t;
//发送包结构体
typedef __packed struct 
{
	frame_header_t 		frame_header;	
	tx_data_t	  		tx_data;	
	frame_tailer_t 		frame_tailer;	
} send_msg_t;
//接受包结构体
typedef __packed struct 
{
	frame_header_t	 	frame_header;	
	rx_data_t	  		rx_data;	
	frame_tailer_t 		frame_tailer;	
} receive_msg_t;


//发送数据结构体
typedef __packed struct 
{
	float curr_pitch;       	//当前云台pitch角
}tx_data_t_big_camera;
//接受数据结构体
typedef __packed struct 
{
	float shoot_pitch;	
}rx_data_t_big_camera;
//发送包结构体
typedef __packed struct 
{
	frame_header_t 		frame_header;	
	tx_data_t_big_camera	  tx_data;	
	frame_tailer_t 		frame_tailer;	
} send_msg_t_big_camera;
//接受包结构体
typedef __packed struct 
{
	frame_header_t	 	frame_header;	
	rx_data_t_big_camera	 rx_data;	
	frame_tailer_t 		frame_tailer;	
} receive_msg_t_big_camera;

extern float before_pitch_big_camera;


extern send_msg_t_big_camera pc_send_msg_6;
extern receive_msg_t_big_camera pc_receive_msg_6;

extern float before_yaw;
extern float before_pitch;
extern float before_shoot_speed_avg;
extern bool_t fire_flag;
extern float pc_shoot_delay;
extern void data_solve(receive_msg_t *pc_receive_msg, uint8_t *rx_data);

extern send_msg_t pc_send_msg;
extern receive_msg_t pc_receive_msg;
extern void pc_info_task(void const *argu);
extern uint8_t BIG_CAMERA_RX_BUF_6[LEN_TX_PACKET_big_camera + 1];
extern void Big_camera_init(void);
extern void data_solve_6(receive_msg_t_big_camera *pc_receive_msg_6, uint8_t *rx_data);
#endif
