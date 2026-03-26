#include "stm32f4xx.h"                  // Device header
#include "force.h"
#include "freertos.h"

extern UART_HandleTypeDef huart1;
uint8_t force_tx_data[TX_BITS] = {0x01,0x04,0x00,0x00,0x00,0x01,0x31,0xCA};
uint8_t force_ini_arr[9] = {0x01,0x03,0x00,0x3A,0x00,0x01,0xA4,0x07 };
//uint8_t force_tx_data[TX_BITS] = {0xCA,0x31,0x01,0x00,0x00,0x00,0x04,0x01};

uint8_t force_rx_arr[30];

uint8_t force_tx_CRC_H[2];
uint8_t force_tx_CRC_L[2];

uint8_t force_rx_CRC_H[2];
uint8_t force_rx_CRC_L[2];

uint8_t force_check = 0;
uint16_t force_change = 0;
float force_elec_pressure = 14.5;

uint16_t force_tx_count = 0;
uint8_t force_rx_ok = 1;
float force_mv;


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
		//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
		//	HAL_UARTEx_ReceiveToIdle_DMA(&huart1,force_rx_arr,40);   //时序要求很紧

			for(int i = 0; i <TX_BITS;i++)
	{
		HAL_UART_Transmit(&huart1,force_ini_arr +i,1,1);		//等待时间有点蛊
	}
	
		//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_SET);       
	  
		GetModbusCRC16(force_tx_data,6,force_tx_CRC_H,force_tx_CRC_L);
		force_tx_data[6] = force_tx_CRC_H[0];
		force_tx_data[7] = force_tx_CRC_L[0];                                     //计算CRC校验码
}	

void get_force(void const * argument)
{
	
	
	while(1)
	{	
		force_rx_handler();

		if(force_rx_ok == 1 || force_tx_count == 8	)           //太短了反而会无法读取
	{

		HAL_UARTEx_ReceiveToIdle_DMA(&huart1,force_rx_arr,40);   //时序要求很紧

		for(int i = 0; i <8;i++)
	{
		HAL_UART_Transmit(&huart1,force_tx_data +i,1,HAL_MAX_DELAY);		//等待时间有点蛊
	}

		  force_rx_ok = 0;
      force_tx_count = 0;

}


force_tx_count++;

err_count++;
if(err_count == 1000)
{
	err=0;
	err_count = 0;
}

		osDelay(1);
	}
}



void force_rx_handler()
{

	GetModbusCRC16(force_rx_arr,RX_BITS - 2,force_rx_CRC_H,force_rx_CRC_L);
		if((force_rx_arr[RX_BITS - 2] == force_rx_CRC_H[0] )&& (force_rx_arr[RX_BITS - 1] == force_rx_CRC_L[0]))
		{
		force_check = 1;
		force_change = ((force_rx_arr[3] << 8) | force_rx_arr[4]);
		force_mv = force_change; 

    //             																											  //放大信号
		}
		else 
		{
			force_check = 0;
			err++;
		}

}
