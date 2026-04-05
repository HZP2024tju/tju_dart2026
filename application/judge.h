#ifndef JUDGE_H
#define JUDGE_H

void get_judge(void const * argument);
void judge_view_send();

void judge_set_shoot(uint8_t shoot_time);

void judge_shoot_angle(uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t shoot_time);

void judge_choose_bias(uint8_t lable);
void judge_set_by_usart(int index,float data);

typedef __packed struct 
{
	uint8_t head;
	uint8_t crc8;
	int shoot_time;
	uint16_t crc16;
	uint8_t tail;
	
}view_SEND;





uint8_t get_crc8_check_sum(uint8_t* pchMessage, uint16_t dwLength, uint8_t ucCRC8);
uint8_t verify_crc8_check_sum(uint8_t* pchMessage, uint16_t dwLength);
void append_crc8_check_sum(uint8_t* pchMessage, uint16_t dwLength);
uint16_t get_crc16_check_sum(uint8_t* pchMessage, uint32_t dwLength, uint16_t wCRC);
//CRC16
uint8_t verify_crc16_check_sum(uint8_t* pchMessage, uint32_t dwLength);

void append_crc16_check_sum(uint8_t* pchMessage, uint32_t dwLength);



#endif

