#include "missle.h"
#include "freertos.h"
#include "servo.h"
#include "JY901B.h"
#include "mitrx.h"

uint32_t what;

//uint8_t send[4] = {0x91,0x08};

extern UART_HandleTypeDef huart2;
	
void missle_task(void *argument)
{
	
	while(1)
	{
		
		// HAL_UART_Transmit_DMA(&huart2,send,4);

//	what++;
		servo_control();
		osDelay(50);

	}
}

