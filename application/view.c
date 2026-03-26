#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "view.h"
#include "dart_task.h"

//哈哈这里有一点石山,让大一的来改

uint8_t view_what2_count = 0;
float what2 = 0;
float view_temp = 0;
float view_temp_last = 0;
uint32_t what = 0;

uint8_t view_ok = 0;
extern uint8_t referee_game_start;

uint8_t view_reflash = 0;
uint8_t data_layout[12];

extern dart_control_t dart_control;
float view_bias = 0.0;  //此变量将会在judge.c中重新赋值

void view_err()//此函数已弃用,这是自瞄出问题时的补救方案
{
		if(uwTick % 100 == 0)  //自动刷新
	{
		if(view_temp == view_temp_last)
		{
			view_ok = 0;
			view_temp = 0;
		}
	view_temp = view_temp_last;
	}
	
#if REFEREE_START
	if(referee_game_start != 4)  //只有在比赛时才开
	{
		return;
	}
#endif

}

void view_data_solve(uint8_t * data)        //解包,接入虚拟串口
{
		what = (data[5]<<24) + (data[4] << 16) + (data[3] << 8 )+ data[2];         //诶我靠我懒得改名字了,总而言之就是拼数据然后转化
	if(data[0] == 0xA5 )
		{
			memcpy(&what2,&what,sizeof(uint8_t) * 4);
			if(what2 <2 * 0.47f && what2 > -2 * 0.52f)
			{
				view_temp = (what2 + view_bias) * 4500.0f;  //这是在dart_task.c中的pid要用到的神秘妙妙数字 令视觉传来的信号放大 这是一拖十
				view_ok = 1;		
			}
			else
			{
				view_temp = 0;
				view_ok = 0;
			}
		}
}

