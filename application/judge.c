#include "freertos.h"
#include "judge.h"
#include "usbd_cdc.h"
#include "dart_task.h"

extern UART_HandleTypeDef huart6;
extern float view_bias;
extern float force_mv_set;
extern dart_control_t dart_control;
extern float dart_2006_angle_set;

uint8_t view_send[11];
uint8_t view_sned_crc16[8];
uint8_t test[5] = {0x01,0x02,0x03,0x04,0x05};

uint8_t judge_rx[10];

uint8_t shoot_time_last = 0;

float judge_arr[5];

float judge_bias1 = -159.1 ; //150       // Yaw -98                 //[1]     = [4]   |       [1]   +  1.5   =   [3]     |   [1]  +  2   =  [5]     |  [1]  - 0.5     =[6] (?)         ;

float judge_bias2 = -159.1;//            //Yaw -97    //0.4约为半个装甲板

float judge_bias3 = -150.1; //-83,9   

float judge_bias4 = -74.44;

float judge_bias5 = -75.4;    //

float judge_bias6 = -74.44 ;  //6 4 5距离相似

 


float judge_view_bias1 = -0.031  ;      //越小越左  0.0015 约为四分之一个装甲板           //[1]     = [4]   |       [1]   +  1.5   =   [3]     |   [1]  +  2   =  [5]     |  [1]  - 0.5     =[6] (?)         ;

float judge_view_bias2 = -0.0404;    //0x0229     0.0001为半个

float judge_view_bias3 =  -0.085;//-0.0170    

float judge_view_bias4 = -0.0349;

float judge_view_bias5 = -0.0416;    //0.0189

float judge_view_bias6 = -0.0407; //0.0257

float judge_mv_bias1 = 366.4  ;                 //[1]     = [4]   |       [1]   +  1.5   =   [3]     |   [1]  +  2   =  [5]     |  [1]  - 0.5     =[6] (?)         ;
float judge_mv_bias3 = 366.4;    
float judge_mv_bias4 = 366.4;
float judge_mv_bias5 = 366.4;    //
float judge_mv_bias6 = 366.4 ;

view_SEND View_send;

uint8_t judge_lable = 0;



/***** 在这里设定发射顺序 *******/
void judge_set_shoot(uint8_t shoot_time)
{
		judge_shoot_angle(1,2,3,4,shoot_time);            
}
/***** 在这里设定发射顺序 *******/




void judge_choose_bias(uint8_t lable)
{
	switch(lable)
	{
		case 1:
			dart_2006_angle_set = judge_bias1;
			view_bias = judge_view_bias1;
		break;
		case 2:
			dart_2006_angle_set = judge_bias2;
			view_bias = judge_view_bias2;
		break;
		case 3:
			dart_2006_angle_set = judge_bias3;
			view_bias = judge_view_bias3;
		break;
		case 4:
			dart_2006_angle_set = judge_bias4;
			view_bias = judge_view_bias4;
		break;
		case 5:
			dart_2006_angle_set = judge_bias5;
			view_bias = judge_view_bias5;
		break;
		case 6:
			dart_2006_angle_set = judge_bias6;
			view_bias = judge_view_bias6;
		break;
	}
}

void judge_shoot_angle(uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t shoot_time)
{
	switch(shoot_time)
	{
		case 0:
			judge_choose_bias(a);
		break;
		case 1:
			judge_choose_bias(b);
		break;
		case 2:
			judge_choose_bias(c);
		break;
		case 3:
			judge_choose_bias(d);
		break;
		
	}
}



void get_judge(void const * argument)
{
	//judge_init();
	while(1)
	{
		judge_view_send();
		CDC_Transmit_FS(view_send, 9);
		osDelay(100);

	}
}



void judge_view_send()
{
	View_send.head = 0xA5;
	View_send.shoot_time = judge_lable ; //向视觉发送本次击打使用哪个编号的飞镖(记得告诉他把偏移删了)
	View_send.tail = '\n';
	
	memcpy(view_send,&View_send,9);
	append_crc16_check_sum(view_send,8);
	
}


//CRC8
uint8_t get_crc8_check_sum(uint8_t* pchMessage, uint16_t dwLength, uint8_t ucCRC8)
{
	uint8_t ucIndex;
	while (dwLength--)
	{
		ucIndex = ucCRC8 ^ (*pchMessage++);
		ucCRC8  = CRC8_TAB[ucIndex];
	}
	return (ucCRC8);
}
//CRC8
uint8_t verify_crc8_check_sum(uint8_t* pchMessage, uint16_t dwLength)
{
	uint8_t ucExpected = 0;
	if ((pchMessage == 0) || (dwLength <= 2))
		return 0;
	ucExpected = get_crc8_check_sum(pchMessage, dwLength - 1, CRC8_initt);
		return (ucExpected == pchMessage[dwLength - 1]);
}
//CRC
void append_crc8_check_sum(uint8_t* pchMessage, uint16_t dwLength)
{
	uint8_t ucCRC = 0;
	if ((pchMessage == 0) || (dwLength <= 2))
		return;
	ucCRC = get_crc8_check_sum((uint8_t*)pchMessage, dwLength - 1, CRC8_initt);
	pchMessage[dwLength - 1] = ucCRC;
}
//
uint16_t get_crc16_check_sum(uint8_t* pchMessage, uint32_t dwLength, uint16_t wCRC)
{
	uint8_t chData;
	if (pchMessage == NULL)
	{
		return 0xffff;
	}
	while (dwLength--)
	{
		chData = *pchMessage++;
		(wCRC) = ((uint16_t)(wCRC) >> 8) ^ wCRC_Table[((uint16_t)(wCRC) ^ (uint16_t)(chData)) & 0x00ff];
	}
  return wCRC;
}
//CRC16
uint8_t verify_crc16_check_sum(uint8_t* pchMessage, uint32_t dwLength)
{
	uint16_t wExpected = 0;
	if ((pchMessage == NULL) || (dwLength <= 2))
	{
		return 0;
	}
	wExpected = get_crc16_check_sum(pchMessage, dwLength - 2, CRC16_INIT);
  // printf("expected = %x, we have %x\n", wExpected, *(uint16_t*)(&pchMessage[dwLength - 2]));

	return ((wExpected & 0xff) == pchMessage[dwLength - 2] && ((wExpected >> 8) & 0xff) == pchMessage[dwLength - 1]);
}

//CRC
void append_crc16_check_sum(uint8_t* pchMessage, uint32_t dwLength)
{
	uint16_t wCRC = 0;
	if ((pchMessage == NULL) || (dwLength <= 2))
	{
		return;
	}
	wCRC = get_crc16_check_sum((uint8_t*)pchMessage, dwLength - 2, CRC16_INIT);
	pchMessage[dwLength - 2] = (uint8_t)(wCRC & 0x00ff);
	pchMessage[dwLength - 1] = (uint8_t)((wCRC >> 8) & 0x00ff);
}



 // extern CRC16_INIT
