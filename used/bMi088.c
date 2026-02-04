#include "stm32f1xx.h"
#include "bMi088.h"
#include "IIC.h"
//   HAL_SPI_Receive_DMA(
		//HAL_SPI_Transmit_DMA(
		//HAL_I2C_Master_Transmit(&hi2c1,(0x50<<1),&reg_addr,1,HAL_MAX_DELAY);
		//HAL_I2C_Master_Receive(&hi2c1,(0x50<<1)|1,i2c_rx_data,12,HAL_MAX_DELAY);
		//加速度 0x18 0x19
		//角速度 0x68 0x69
		//写0 读1
		
uint8_t bmi_arr[10];
		
void Bmi088_init()
{
	
	osDelay(10);
	HAL_GPIO_WritePin(BMI_CS_GPIO,BMI088_ACC_CS_PIN,GPIO_PIN_RESET);
	osDelay(2);
	HAL_GPIO_WritePin(BMI_CS_GPIO,BMI088_ACC_CS_PIN,GPIO_PIN_SET); //加速度计特有的需要拉低后拉高
	
	uint8_t temp_data;
	temp_data = 0x00;
  Bmi0088_read_reg(bmi_arr,0x00 | 0x80,1,1);
	temp_data = 0x04;
	
	
	
	
}

void Bmi088_get_data(BMI088_DATA* B_Data)
{
	uint8_t temp_data[10];
	
	
	
}

void Bmi0088_read_reg(uint8_t *pdata,uint8_t reg,uint8_t Size,uint8_t select)
{
	
	Bmi088_write(&reg,1,select);
	Bmi088_read(pdata,Size,select);
	Bmi088_read(pdata,Size,select);

}
		
void Bmi088_read(uint8_t *pdata,uint8_t Size,uint8_t select)   //ACC为1,GYRO为2
{
	if(select == 1)
	{
		HAL_GPIO_WritePin(BMI_CS_GPIO,BMI088_ACC_CS_PIN,GPIO_PIN_RESET);
		HAL_SPI_Receive_DMA(&hspi1,pdata,Size);
		osDelay(1);
		HAL_GPIO_WritePin(BMI_CS_GPIO,BMI088_ACC_CS_PIN,GPIO_PIN_SET);

	}
	else	if(select == 2)
	{
		HAL_GPIO_WritePin(BMI_CS_GPIO,BMI088_GYRO_CS_PIN,GPIO_PIN_RESET);
		HAL_SPI_Receive_DMA(&hspi1,pdata,Size);
		osDelay(1);
		HAL_GPIO_WritePin(BMI_CS_GPIO,BMI088_GYRO_CS_PIN,GPIO_PIN_SET);

	}
};		

void Bmi088_write(uint8_t *pdata,uint8_t Size,uint8_t select)   //ACC为1,GYRO为2
{
	if(select == 1)
	{
		HAL_GPIO_WritePin(BMI_CS_GPIO,BMI088_ACC_CS_PIN,GPIO_PIN_RESET);
		HAL_SPI_Transmit_DMA(&hspi1,pdata,Size);
		osDelay(1);
		HAL_GPIO_WritePin(BMI_CS_GPIO,BMI088_ACC_CS_PIN,GPIO_PIN_SET);

	}
	else	if(select == 2)
	{
		HAL_GPIO_WritePin(BMI_CS_GPIO,BMI088_GYRO_CS_PIN,GPIO_PIN_RESET);
		HAL_SPI_Transmit_DMA(&hspi1,pdata,Size);
		osDelay(1);
		HAL_GPIO_WritePin(BMI_CS_GPIO,BMI088_GYRO_CS_PIN,GPIO_PIN_SET);

	}
};



