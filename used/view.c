#include "view.h"
#include "stm32f4xx.h"
#include "mitrx.h"

 //逗号0x5C
uint32_t view_change;

uint16_t view_rx_len = 9;

float view_y[2];
float view_x;

float view_y_temp_last;        //对于 yaw轴 灯相对相机正向偏右为正
float view_x_temp_last;        // pitch轴 灯偏上为正
float view_y_temp;        //对于 yaw轴 灯相对相机正向偏右为正
float view_x_temp;
	
uint8_t view_set_x = 0;
uint8_t view_set_y = 0;

uint32_t view_count;
 
void view_solve(uint8_t * data)
{	
		if(data[0] == 0xA5)
		{
			view_change	 =  (data[4]<<24) | (data[3]<<16) | (data[2]<<8) | data[1];
			memcpy(&view_y_temp,&view_change,4);	
			view_change	 =  (data[8]<<24) | (data[7]<<16) | (data[6]<<8) | data[5];
			memcpy(&view_x_temp,&view_change,4);	
		}
		
		view_count++;
		
		
		if(view_x_temp_last != view_x_temp)
		{
			view_set_x = 0;
			view_count = 0;
		}
		
			if((view_x_temp_last == view_x_temp &&  view_count > 6) || view_set_x == 1)
			{
				view_x = 0;
				//view_count = 0;
				view_set_x = 1;
			}
			else if(view_set_x == 0)
			{
					view_x = view_x_temp;
			}
			
			
			if(view_y_temp_last != view_y_temp)
			{
				view_set_y = 0;
				view_count = 0;
			}
		
			if((view_y_temp_last == view_y_temp &&  view_count > 6) || view_set_y == 1 )
			{
				view_y[0] = 0;
				//view_count = 0;
				view_set_y = 1;
			}
			else if(view_set_y == 0)
			{
					view_y[0] = view_y_temp;
					
			}
		
		
			view_y_temp_last = view_y_temp;
			view_x_temp_last = view_x_temp;
}

