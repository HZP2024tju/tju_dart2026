#ifndef REFEREE_H
#define REFEREE_H
#include "main.h"
typedef __packed struct fuckyou
{
  uint8_t SOF;
  uint16_t data_length;
  uint8_t seq;
  uint8_t CRC8;
}frame_header_struct ;


typedef enum
{
    ROBOT_STATE_CMD_ID                = 0x0201,
    POWER_HEAT_DATA_CMD_ID            = 0x0202,
    SHOOT_DATA_CMD_ID                 = 0x0207,
    BULLET_REMAINING_CMD_ID           = 0x0208,
		DART_STATE_ID											= 0X020A,
	  MULTI_COMMUNICATE_ID							= 0x0301
}referee_cmd_id_t;

typedef enum
{
		HERO_ID														= 0x0201,
		DART_ID                           = 0x0008,     //ºì·½
		INFANTRY3_ID  										= 0x0203,
		INFANTRY4_ID											= 0x0204,
		INFANTRY5_ID											= 0x0205
}data_cmd_id_t;

typedef __packed struct
{
    uint8_t  sof;
    uint16_t dataLenth;
    uint8_t  seq;
    uint8_t  crc8;
//		uint16_t CMD_ID;
} tFrameHeader;

typedef __packed struct 
{ 
  uint8_t robot_id; 
  uint8_t robot_level; 
  uint16_t current_HP;  
  uint16_t maximum_HP; 
  uint16_t shooter_barrel_cooling_value; 
  uint16_t shooter_barrel_heat_limit; 
  uint16_t chassis_power_limit;  
  uint8_t power_management_gimbal_output : 1; 
  uint8_t power_management_chassis_output : 1;  
  uint8_t power_management_shooter_output : 1; 
}robot_status_t; 


typedef __packed struct            //0x020A 
{ 
  uint8_t dart_launch_opening_status;  
  uint8_t dart_attack_target;  
  uint16_t target_change_time;  
  uint16_t latest_launch_cmd_time; 
}dart_client_cmd_t; 

typedef __packed struct 
{ 
  uint8_t dart_remaining_time; 
  uint16_t dart_info; 
}dart_info_t; 

typedef __packed struct 
{ 
  uint8_t game_type : 4; 
  uint8_t game_progress : 4; 
  uint16_t stage_remain_time; 
  uint64_t SyncTimeStamp; 
}game_status_t; 


void referee_auto(void);
void init_referee_struct_data(void);
void referee_data_solve(uint8_t *frame);
 extern robot_status_t dart_message;
#endif
