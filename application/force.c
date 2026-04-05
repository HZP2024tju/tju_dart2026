#include "stm32f4xx.h"                  // Device header
#include "force.h"
#include "freertos.h"
#include <stdlib.h>

extern UART_HandleTypeDef Force_huart;
uint8_t force_tx_data[TX_BITS] = {0x01,0x04,0x00,0x00,0x00,0x01,0x31,0xCA};
uint8_t force_ini_arr[TX_BITS] = {0x01,0x03,0x00,0x3A,0x00,0x01,0xA4,0x07 };
//uint8_t force_tx_data[TX_BITS] = {0xCA,0x31,0x01,0x00,0x00,0x00,0x04,0x01};

uint8_t force_rx_arr[20];
uint8_t force_tx_CRC_H[2];
uint8_t force_tx_CRC_L[2];

uint8_t force_rx_CRC_H[2];
uint8_t force_rx_CRC_L[2];

uint8_t force_check = 0;
uint16_t force_change = 0;
float force_elec_pressure = 14.5;

uint16_t force_tx_count = 0;
uint8_t force_rx_ok = 0;
float force_mv;

uint8_t force_tx_res[5] = {0};

int err_count = 0;
int err = 0;

//这里史山有点多,哈哈让大一的来修吧

/**
* @brief
* @param
* @param
* @param
* @param
* @note	Calculate Modbus CRC16
puchMsg: Data to calculate CRC
usMsgLen: Data length
puchCRCHi: High byte of CRC value
puchCRCLo: Low byte of CRC value
Send the low byte first
* @retval None
*/
void GetModbusCRC16(unsigned char *puchMsg, unsigned short usMsgLen,
unsigned char *puchCRCLo, unsigned char *puchCRCHi)                                   //计算CRC校验码
{
unsigned char uchCRCLo = 0xFF;unsigned char uchCRCHi = 0xFF;unsigned short uIndex = 0;
while(usMsgLen--)
{
uIndex = (unsigned char)(uchCRCHi^*puchMsg++);
uchCRCHi = (unsigned char)(uchCRCLo^auchCRCHi[uIndex]);
uchCRCLo = (unsigned char)(auchCRCLo[uIndex]);
}
*puchCRCLo = uchCRCHi;
*puchCRCHi = uchCRCLo;
}


void force_ini()
{
	
			for(int i = 0; i <TX_BITS;i++)
	{
		HAL_UART_Transmit(&Force_huart,force_ini_arr +i,1,1);		//等待时间有点蛊
	}
		GetModbusCRC16(force_tx_data,6,force_tx_CRC_H,force_tx_CRC_L);
		force_tx_data[6] = force_tx_CRC_H[0];
		force_tx_data[7] = force_tx_CRC_L[0];                                     //计算CRC校验码
}	

void get_force(void const * argument)
{
	force_ini();
	while(1)
	{			//这一段可以优化
			if(force_tx_count == 15	)           //太短了反而会无法读取
			{
				force_rx_handler(force_rx_arr);
				HAL_UARTEx_ReceiveToIdle_DMA(&Force_huart,force_rx_arr,RX_BITS*2);
				for(int i = 0; i <TX_BITS;i++)
				{
					HAL_UART_Transmit(&Force_huart,force_tx_data +i,1,1);		//等待时间有点蛊	
				}		
				force_tx_count = 0;
			}
			force_tx_count++;
			osDelay(1);
	}
}



void force_rx_handler(uint8_t * data)
{
	if(data[1] == 0x04)
	{
		GetModbusCRC16(data,RX_BITS - 2,data,force_rx_CRC_L);
	//	if((data[RX_BITS - 2] == force_rx_CRC_H[0] )&& (data[RX_BITS - 1] == force_rx_CRC_L[0]))
		{
			force_change = ((data[3] << 8) | data[4]);
			force_mv = (((float)force_change) * 500 /65535)-1;   
			force_rx_ok = 0;			//放大信号
		}
	}
}

void force_send(UART_HandleTypeDef *res_huart)
{
		memcpy(force_tx_res,&force_mv,4);
		HAL_UART_Transmit_DMA(res_huart,force_tx_res,4);
}
