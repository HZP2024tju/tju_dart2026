
#include "pc_info_task.h"
#include "bsp_usart.h"
#include "cmsis_os.h"
#include "string.h"
#include "main.h"
#include "CAN_receive.h"
#include "usbd_cdc_if.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "bsp_dwt.h"
//与pc通信
send_msg_t pc_send_msg;
receive_msg_t pc_receive_msg;
uint8_t PC_SEND_BUF[LEN_TX_PACKET + 1];
//大相机上的C板通信
send_msg_t_big_camera pc_send_msg_6;
receive_msg_t_big_camera pc_receive_msg_6;
uint8_t BIG_CAMERA_RX_BUF_6[LEN_TX_PACKET_big_camera + 1];
uint8_t BIG_CAMERA_RX_BUF_6_temp[LEN_TX_PACKET_big_camera + 1];
uint8_t color_flag;
int for_pitch = 0;
//自瞄状态机切换，想改初始处于什么状态，仅需更改up,person,以及下面那句状态初始化
uint8_t modeSet_flag = 0;
uint8_t modeChange_flag = 0;
uint8_t modeChange_time_up = 0;
uint8_t modeChange_time_down = 0;
uint8_t modeChange_time = 0;
uint8_t up = 0;
uint8_t person = 1;

void data_solve(receive_msg_t *pc_receive_msg, uint8_t *rx_data);
static void data_solve_6(receive_msg_t_big_camera *pc_receive_msg_6, uint8_t *rx_data);
float pc_dt;
uint32_t pc_DWT_Count;


uint8_t res = 0;
float before_yaw = 0, before_pitch = 0, before_shoot_speed_avg = 15.8f;
bool_t fire_flag;
void data_solve(receive_msg_t *pc_receive_msg, uint8_t *rx_data)
{
	if(rx_data[SOF_ADDR] == FRAME_HEADER) 
	{	
		//帧尾CRC16校验
		res = verify_crc16_check_sum(rx_data, LEN_RX_PACKET);
		if(res == 1) 
		{
			//数据正确则拷贝接收数据
			memcpy(pc_receive_msg, rx_data, LEN_RX_PACKET);	
		}
	}
	//若数据无效，保持原数据不变
	if(res == 0) 
	{ 
		pc_receive_msg->rx_data.fire=0;
		pc_receive_msg->rx_data.shoot_yaw = before_yaw;
		pc_receive_msg->rx_data.shoot_pitch = before_pitch;
		pc_receive_msg->rx_data.shoot_speed_avg = before_shoot_speed_avg;
	}
	else
	{
		//记录这次接收到的角度值
		before_yaw = pc_receive_msg->rx_data.shoot_yaw;
		before_pitch = pc_receive_msg->rx_data.shoot_pitch;
		for_pitch = 1000*pc_receive_msg->rx_data.shoot_pitch;
		before_shoot_speed_avg = pc_receive_msg->rx_data.shoot_speed_avg;
	}
	fire_flag = pc_receive_msg->rx_data.fire;
	if( pc_receive_msg->rx_data.shoot_pitch > 6.28f ||  pc_receive_msg->rx_data.shoot_yaw > 6.28f)
	{
		
	}
}

//触发空闲中断时进行数据的校验与拷贝
void USART1_IRQHandler(void)
{
	if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) && __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_IDLE))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);
		uint32_t DMA_FLAGS = __HAL_DMA_GET_TC_FLAG_INDEX(huart1.hdmarx); 
		USART1_RX_STA = USART1_MAX_RECV_LEN - huart1.hdmarx->Instance->NDTR; 
		__HAL_DMA_DISABLE(huart1.hdmarx);
		data_solve(&pc_receive_msg,USART1_RX_BUF);
		__HAL_DMA_CLEAR_FLAG(huart1.hdmarx, DMA_FLAGS);	
		__HAL_DMA_SET_COUNTER(huart1.hdmarx,USART1_MAX_RECV_LEN);
		__HAL_DMA_ENABLE(huart1.hdmarx);
	}
	//串口重新开启接收中断（尝试用）
	if((huart1.Instance->CR1 & 0x20)==0)
	{
		HAL_UART_Receive_IT(&huart1,(uint8_t*)USART1_RX_BUF,USART1_MAX_RECV_LEN);
	}
	 HAL_UART_IRQHandler(&huart1);
}

uint8_t res6 = 0;
float before_pitch_big_camera = 0;
static void data_solve_6(receive_msg_t_big_camera *pc_receive_msg_6, uint8_t *rx_data)
{
	if(rx_data[SOF_ADDR] == FRAME_HEADER) 
	{	
		//帧尾CRC16校验
		res6 = verify_crc16_check_sum(rx_data, LEN_RX_PACKET_big_camera);
		if(res6 == 1) 
		{
			//数据正确则拷贝接收数据
			memcpy(pc_receive_msg_6, rx_data, LEN_RX_PACKET_big_camera);	
		}
	}
	else//解决错位情况
	{ 
		uint8_t i = 0;
		for(i = 0;i < 9;i++)
		{
			if(rx_data[i] == FRAME_HEADER)
				break;
		}
		uint8_t j = 0;
		for(j = 0;j < 9;j++)
		{
			BIG_CAMERA_RX_BUF_6_temp[j] = rx_data[i];
			i++;
			if(i >= 9)
				i = 0;			
		}
		res6 = verify_crc16_check_sum(BIG_CAMERA_RX_BUF_6_temp, LEN_RX_PACKET_big_camera);
		if(res6 == 1) 
		{
			//数据正确则拷贝接收数据
			memcpy(pc_receive_msg_6, BIG_CAMERA_RX_BUF_6_temp, LEN_RX_PACKET_big_camera);	
		}
	}
		
	//若数据无效，保持原数据不变
	if(res6 == 0) 
	{ 
		pc_receive_msg_6->rx_data.shoot_pitch = before_pitch_big_camera;
	}
	else
	{
		before_pitch_big_camera = pc_receive_msg_6->rx_data.shoot_pitch;
//		HAL_UART_Receive_DMA(&huart6,BIG_CAMERA_RX_BUF_6,9);
	}
}

//触发空闲中断时进行数据的校验与拷贝
//void USART6_IRQHandler(void)
//{
//	if (__HAL_UART_GET_FLAG(&huart6, UART_FLAG_IDLE) && __HAL_UART_GET_IT_SOURCE(&huart6, UART_IT_IDLE))
//	{
//		__HAL_UART_CLEAR_IDLEFLAG(&huart6);
//		uint32_t DMA_FLAGS = __HAL_DMA_GET_TC_FLAG_INDEX(huart6.hdmarx); 
//		USART6_RX_STA = USART6_MAX_RECV_LEN - huart6.hdmarx->Instance->NDTR; 
//		__HAL_DMA_DISABLE(huart6.hdmarx);
//		data_solve_6(&pc_receive_msg_6,USART6_RX_BUF);
//		__HAL_DMA_CLEAR_FLAG(huart6.hdmarx, DMA_FLAGS);	
//		__HAL_DMA_SET_COUNTER(huart6.hdmarx,USART6_MAX_RECV_LEN);
//		__HAL_DMA_ENABLE(huart6.hdmarx);
//	}
//	//串口重新开启接收中断（尝试用）
//	if((huart6.Instance->CR1 & 0x20)==0)
//	{
//		HAL_UART_Receive_IT(&huart6,(uint8_t*)USART6_RX_BUF,USART6_MAX_RECV_LEN);
//	}
//}
//void USART6_IRQHandler(void)
//{
//    if(huart6.Instance->SR & UART_FLAG_RXNE)//接收到数据
//    {
//        __HAL_UART_CLEAR_PEFLAG(&huart6);
//    }
//    else if(USART6->SR & UART_FLAG_IDLE)
//    {
//        static uint16_t this_time_rx_len = 0;

//        __HAL_UART_CLEAR_PEFLAG(&huart6);

//        if ((hdma_usart6_rx.Instance->CR & DMA_SxCR_CT) == RESET)
//        {
//            /* Current memory buffer used is Memory 0 */
//    
//            //disable DMA
//            //失效DMA
//            __HAL_DMA_DISABLE(&hdma_usart6_rx);

//            //get receive data length, length = set_data_length - remain_length
//            //获取接收数据长度,长度 = 设定长度 - 剩余长度
//            this_time_rx_len = LEN_TX_PACKET_big_camera + 1 - hdma_usart6_rx.Instance->NDTR;

//            //reset set_data_lenght
//            //重新设定数据长度
//            hdma_usart6_rx.Instance->NDTR = SBUS_RX_BUF_NUM;

//            //set memory buffer 1
//            //设定缓冲区1
//            hdma_usart6_rx.Instance->CR |= DMA_SxCR_CT;
//            
//            //enable DMA
//            //使能DMA
//            __HAL_DMA_ENABLE(&hdma_usart6_rx);

//            if(this_time_rx_len == LEN_TX_PACKET_big_camera + 1)
//            {
//								data_solve_6(&pc_receive_msg_6,BIG_CAMERA_RX_BUF_6[0]);
//            }
//        }
//        else
//        {
//            /* Current memory buffer used is Memory 1 */
//            //disable DMA
//            //失效DMA
//            __HAL_DMA_DISABLE(&hdma_usart6_rx);

//            //get receive data length, length = set_data_length - remain_length
//            //获取接收数据长度,长度 = 设定长度 - 剩余长度
//            this_time_rx_len = LEN_TX_PACKET_big_camera + 1 - hdma_usart6_rx.Instance->NDTR;

//            //reset set_data_lenght
//            //重新设定数据长度
//            hdma_usart6_rx.Instance->NDTR = LEN_TX_PACKET_big_camera + 1;

//            //set memory buffer 0
//            //设定缓冲区0
//            DMA2_Stream1->CR &= ~(DMA_SxCR_CT);
//            
//            //enable DMA
//            //使能DMA
//            __HAL_DMA_ENABLE(&hdma_usart6_rx);

//            if(this_time_rx_len == LEN_TX_PACKET_big_camera + 1)
//            {
//                //处理遥控器数据
//								data_solve_6(&pc_receive_msg_6,BIG_CAMERA_RX_BUF_6[1]);
//            }
//        }
//    }
//}

void USART6_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart6);
	data_solve_6(&pc_receive_msg_6,BIG_CAMERA_RX_BUF_6);
	HAL_UART_Receive_IT(&huart6,BIG_CAMERA_RX_BUF_6,9);
}
//void Big_camera_init()
//{
//	usart6_init(BIG_CAMERA_RX_BUF_6[0],BIG_CAMERA_RX_BUF_6[1],LEN_TX_PACKET_big_camera + 1);
//}
