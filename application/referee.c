#include "referee.h"
#include "string.h"
#include "stdio.h"
#include "CAN_receive.h"
#include "dart_task.h"


frame_header_struct referee_receive_header;

extern uint8_t auto_start;
extern uint8_t auto_start_last;



//dart_client_cmd_t dart_message;

robot_status_t dart_message1;                 //查看机器人id
dart_client_cmd_t dart_message2;              //读取闸门
dart_info_t dart_message3;                    //看发射剩余时间
game_status_t dart_message4;                  //读取比赛阶段


uint8_t dart_door = 0;
uint8_t dart_door_last = 0;

uint8_t referee_door_open = 0;

uint8_t referee_game_start = 0;

//与裁判系统通信初始化
void init_referee_struct_data(void)
{
    memset(&dart_message1, 0, sizeof(robot_status_t));
    memset(&dart_message2, 0, sizeof(dart_client_cmd_t));
    memset(&dart_message3, 0, sizeof(dart_info_t));
	  memset(&dart_message4, 0, sizeof(game_status_t));
}
//裁判系统数据解包


void referee_auto()
{
	
#if REFEREE_START
	dart_door = dart_message2.dart_launch_opening_status;
	referee_game_start = dart_message4.game_progress;


	if(referee_game_start == 4 )
	{
		#if FourMode
		if(((dart_door_last == 2) && (dart_door  == 0))    || (dart_message3.dart_remaining_time == 18))  
		{
		auto_start = 1;
		}
		#endif
		
		if(((dart_door_last == 2) && (dart_door  == 0))    || (dart_message3.dart_remaining_time == 18))  
		{
		auto_start = 1;
		}
		
		if((dart_door_last == 2) && (dart_door  == 0))
		{
		referee_door_open = 1;
		}
	}
	
#else
		if(((dart_door_last == 1) && (dart_door  == 2))    || (dart_message3.dart_remaining_time == 18))  
	{
		auto_start = 1;
	}
	
#endif
	if(   ((dart_message3.dart_info & 0x0002) == 0x0002 )|| ((dart_message3.dart_info & 0x0008) == 0x0008 ))
	{
		
	}
	dart_door_last = dart_door;
}


void referee_data_solve(uint8_t *frame)
{
    uint16_t cmd_id = 0;
    uint8_t index = 0;
		uint16_t data_cmd_id = 0;
    memcpy(&referee_receive_header, frame, sizeof(frame_header_struct));
    index += sizeof(frame_header_struct);
    memcpy(&cmd_id, frame + index, sizeof(uint16_t));
    index += sizeof(uint16_t);

    switch (cmd_id)
    {
			
			case 0x0201:
			{
			 memcpy(&dart_message1, frame + index, sizeof(robot_status_t));
				break;
			}

			case 0x020A:
			{
				
			 memcpy(&dart_message2, frame + index, sizeof(dart_client_cmd_t));
				break;
				
			}
			
			case 0x0105:
			{
			memcpy(&dart_message3, frame + index, sizeof(dart_info_t));
				break;
			}	
			case 0x0001:			
			{
			memcpy(&dart_message4, frame + index, sizeof(game_status_t));
				break;
			}	
				
		}
}
