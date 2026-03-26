/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       can_receive.c/h
  * @brief      there is CAN interrupt function  to receive motor data,
  *             and CAN send function to send motor current to control motor.
  *             这里是CAN中断接收函数，接收电机数据,CAN发送函数发送电机电流控制电机.
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. done
  *  V1.1.0     Nov-11-2019     RM              1. support hal lib
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */

#include "CAN_receive.h"
#include "dart_task.h"
#include "cmsis_os.h"
#include "main.h"


extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
//motor data read
#define get_motor_measure(ptr, data)                                    \
    {                                                                   \
        (ptr)->last_ecd = (ptr)->ecd;                                   \
        (ptr)->ecd = (uint16_t)((data)[0] << 8 | (data)[1]);            \
        (ptr)->speed_rpm = (uint16_t)((data)[2] << 8 | (data)[3]);      \
        (ptr)->given_current = (uint16_t)((data)[4] << 8 | (data)[5]);  \
        (ptr)->temperate = (data)[6];                                   \
    }
/*
motor data,  0:chassis motor1 3508;1:chassis motor3 3508;2:chassis motor3 3508;3:chassis motor4 3508;
4:yaw gimbal motor 6020;5:pitch gimbal motor 6020;6:trigger motor 2006;
电机数据, 0:底盘电机1 3508电机,  1:底盘电机2 3508电机,2:底盘电机3 3508电机,3:底盘电机4 3508电机;
4:yaw云台电机 6020电机; 5:pitch云台电机 6020电机; 6:拨弹电机 2006电机*/
static motor_measure_t motor_chassis[13];

static CAN_TxHeaderTypeDef  gimbal_tx_message;
static uint8_t              gimbal_can_send_data[8];
static CAN_TxHeaderTypeDef  chassis_tx_message;
static uint8_t              chassis_can_send_data[8];

/**
  * @brief          hal CAN fifo call back, receive motor data
  * @param[in]      hcan, the point to CAN handle
  * @retval         none
  */
/**
  * @brief          hal库CAN回调函数,接收电机数据
  * @param[in]      hcan:CAN句柄指针
  * @retval         none
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);

	if (hcan == &DART_CAN)	
	{
    switch (rx_header.StdId)
    {
        case CAN_3508_M1_ID:
        case CAN_2006_M2_ID:
        case CAN_YAW_M3_ID:
				case CAN_6020_M4_ID:
        {
            static uint8_t i = 0;
            //get motor id
            i = rx_header.StdId - CAN_DART_ALL_ID;   //stdid - 0x201
            get_motor_measure(&motor_chassis[i], rx_data);           //此处取得数据
            if(rx_header.StdId == CAN_2006_M2_ID)
            {
                if (motor_chassis[CAN_2006_M2_ID - CAN_DART_ALL_ID].ecd - motor_chassis[CAN_2006_M2_ID - CAN_DART_ALL_ID].last_ecd > HALF_ECD_RANGE)
								{
										dart_control.Dart_2006_Bullet_Motor.round_cnt --;//反转
								}
								else if (motor_chassis[CAN_2006_M2_ID - CAN_DART_ALL_ID].ecd - motor_chassis[CAN_2006_M2_ID - CAN_DART_ALL_ID].last_ecd < -HALF_ECD_RANGE)//此时ecd超过ECD_RANGE，从0开始增加
								{
										dart_control.Dart_2006_Bullet_Motor.round_cnt ++;//正转
								}
								
								//计算输出轴角度 
								dart_control.Dart_2006_Bullet_Motor.angle = -dart_control.Dart_2006_Bullet_Motor.round_cnt*2*PI/REDUCTION_RATIO_2006 - (motor_chassis[CAN_2006_M2_ID - CAN_DART_ALL_ID].ecd - dart_control.Dart_2006_Bullet_Motor.offset_ecd) * MOTOR_ECD_TO_ANGLE / REDUCTION_RATIO_2006;
                
						}
						
            if(rx_header.StdId == CAN_YAW_M3_ID)
						{
							
		             if (motor_chassis[CAN_YAW_M3_ID - CAN_DART_ALL_ID].ecd - motor_chassis[CAN_YAW_M3_ID - CAN_DART_ALL_ID].last_ecd > HALF_ECD_RANGE)
								{
										dart_control.Dart_Yaw_Motor.round_cnt --;//反转
								}
								else if (motor_chassis[CAN_YAW_M3_ID - CAN_DART_ALL_ID].ecd - motor_chassis[CAN_YAW_M3_ID - CAN_DART_ALL_ID].last_ecd < -HALF_ECD_RANGE)//此时ecd超过ECD_RANGE，从0开始增加
								{
										dart_control.Dart_Yaw_Motor.round_cnt ++;//正转
								}
								
								dart_control.Dart_Yaw_Motor.angle = dart_control.Dart_Yaw_Motor.round_cnt*2*PI/REDUCTION_RATIO_2006 + (motor_chassis[CAN_YAW_M3_ID - CAN_DART_ALL_ID].ecd - dart_control.Dart_Yaw_Motor.offset_ecd) * MOTOR_ECD_TO_ANGLE / REDUCTION_RATIO_2006;

						}
						
						if(rx_header.StdId == CAN_3508_M1_ID)
            {
                if (motor_chassis[CAN_3508_M1_ID - CAN_DART_ALL_ID].ecd - motor_chassis[CAN_3508_M1_ID - CAN_DART_ALL_ID].last_ecd > HALF_ECD_RANGE )
								{
										dart_control.Dart_3508_Motor.round_cnt --;//反转
								}
								else if (motor_chassis[CAN_3508_M1_ID - CAN_DART_ALL_ID].ecd - motor_chassis[CAN_3508_M1_ID - CAN_DART_ALL_ID].last_ecd < -HALF_ECD_RANGE )//此时ecd超过ECD_RANGE，从0开始增加
								{
										dart_control.Dart_3508_Motor.round_cnt ++;//正转
								}
								
								//计算输出轴角度 
								dart_control.Dart_3508_Motor.angle = (dart_control.Dart_3508_Motor.round_cnt*2*PI/REDUCTION_RATIO_3508 + ((motor_chassis[CAN_3508_M1_ID - CAN_DART_ALL_ID].ecd - dart_control.Dart_3508_Motor.offset_ecd) * MOTOR_ECD_TO_ANGLE / REDUCTION_RATIO_3508)) * 10000;  //10000是因为值太小了放大一下
						}
						
						if(rx_header.StdId == CAN_6020_M4_ID)
            {
                if (motor_chassis[CAN_6020_M4_ID - CAN_DART_ALL_ID].ecd - motor_chassis[CAN_6020_M4_ID - CAN_DART_ALL_ID].last_ecd > HALF_ECD_RANGE - 2)
								{
										dart_control.Dart_6020_Motor.round_cnt --;//反转
								}
								else if (motor_chassis[CAN_6020_M4_ID - CAN_DART_ALL_ID].ecd - motor_chassis[CAN_6020_M4_ID - CAN_DART_ALL_ID].last_ecd < -HALF_ECD_RANGE + 2)//此时ecd超过ECD_RANGE，从0开始增加
								{
										dart_control.Dart_6020_Motor.round_cnt ++;//正转
								}
								
								//计算输出轴角度 
								dart_control.Dart_6020_Motor.angle =  -dart_control.Dart_6020_Motor.round_cnt*2*PI/REDUCTION_RATIO_6020 -  ((motor_chassis[CAN_6020_M4_ID - CAN_DART_ALL_ID].ecd ) * MOTOR_ECD_TO_ANGLE / REDUCTION_RATIO_6020) ;
								//对于6020,我们只使用单圈定位功能,利用绝对值编码器
																	
						}
            break;
        }
				


        default:
        {
            break;
        }
    }
	}
}




/**
  * @brief          发送电机控制电流(0x201,0x202,0x203,0x204)
  * @param[in]      motor1: (0x201) 3508电机控制电流, 范围 [-16384,16384]
  * @param[in]      motor2: (0x202) 2006电机控制电流, 范围 [-16384,16384]
  * @param[in]      motor3: (0x203) 2006电机控制电流, 范围 [-16384,16384]
  * @param[in]      motor4: (0x204) 电机控制电流, 范围 [-16384,16384]
  * @retval         none
  */
void CAN_cmd_dart(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4)
{
    uint32_t send_mail_box;
    chassis_tx_message.StdId = CAN_DART_ALL_ID ;
    chassis_tx_message.IDE = CAN_ID_STD;
    chassis_tx_message.RTR = CAN_RTR_DATA;
    chassis_tx_message.DLC = 0x08;
    chassis_can_send_data[0] = motor1 >> 8;
    chassis_can_send_data[1] = motor1;
    chassis_can_send_data[2] = motor2 >> 8;
    chassis_can_send_data[3] = motor2;
    chassis_can_send_data[4] = motor3 >> 8;
    chassis_can_send_data[5] = motor3;
    chassis_can_send_data[6] = motor4 >> 8;
    chassis_can_send_data[7] = motor4;

    HAL_CAN_AddTxMessage(&hcan1, &chassis_tx_message, chassis_can_send_data, &send_mail_box);
}

void CAN_cmd_dart6020(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4)
{
    uint32_t send_mail_box;
    chassis_tx_message.StdId = CAN_DART6020_ALL_ID;                                           //chassis变量这样写真的没问题吗
    chassis_tx_message.IDE = CAN_ID_STD;
    chassis_tx_message.RTR = CAN_RTR_DATA;
    chassis_tx_message.DLC = 0x08;
    chassis_can_send_data[0] = motor1 >> 8;
    chassis_can_send_data[1] = motor1;
    chassis_can_send_data[2] = motor2 >> 8;
    chassis_can_send_data[3] = motor2;
    chassis_can_send_data[4] = motor3 >> 8;
    chassis_can_send_data[5] = motor3;
    chassis_can_send_data[6] = motor4 >> 8;
    chassis_can_send_data[7] = motor4;

    HAL_CAN_AddTxMessage(&hcan1, &chassis_tx_message, chassis_can_send_data, &send_mail_box);   
}
/**
  * @brief          return the yaw 6020 motor data point
  * @param[in]      none
  * @retval         motor data point
  */
/**
  * @brief          返回yaw 6020电机数据指针
  * @param[in]      none
  * @retval         电机数据指针
  */
const motor_measure_t *get_yaw_gimbal_motor_measure_point(void)
{
    return &motor_chassis[4];
}

/**
  * @brief          return the chassis 3508 motor data point
  * @param[in]      i: motor number,range [0,3]
  * @retval         motor data point
  */
/**
  * @brief          返回底盘电机 3508电机数据指针
  * @param[in]      i: 电机编号,范围[0,3]
  * @retval         电机数据指针
  */
const motor_measure_t *get_dart_motor_measure_point(uint8_t i)
{
    return &motor_chassis[i];
}
