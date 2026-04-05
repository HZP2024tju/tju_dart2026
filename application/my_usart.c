#include "my_usart.h"
#include "stm32f4xx.h"
#include "freertos.h"
#include "raspberrypi.h"
#include "force.h"

extern UART_HandleTypeDef huart1;  //Ê÷Ý®ÅÉÓÃµÄ´®¿Ú
extern UART_HandleTypeDef huart6;

extern float force;
uint8_t usart_rx_data_rasp[20];


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart == &huart1)
	{
		rasp_unpack(usart_rx_data_rasp);

		HAL_UARTEx_ReceiveToIdle_DMA(&huart1,usart_rx_data_rasp,20);
	}
	if(huart == &huart6)
	{
		
	}
	
}


void usart_task(void const * argument)
{
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1,usart_rx_data_rasp,20);

	while(1)
	{
		force_send(&huart1);
		osDelay(100);
		
	}
}