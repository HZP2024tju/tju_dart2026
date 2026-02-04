#ifndef __BMI088_H__
#define __BMI088_H__

#include "stm32f1xx_hal.h"

#define BMI_CS_GPIO GPIOA
#define BMI088_ACC_CS_PIN GPIO_PIN_3
#define BMI088_GYRO_CS_PIN GPIO_PIN_4

typedef struct BMI088_Data
{
	int ACC_x;
	int ACC_y;
	int ACC_z;
	int GRY_x;
	int GRY_y;
	int GRY_z;
	
	
}BMI088_DATA;

void Bmi088_init();

void Bmi088_get_data(BMI088_DATA* B_Data);
void Bmi088_read(uint8_t *pdata,uint8_t Size,uint8_t select);   //ACCΪ1,GYROΪ2
void Bmi088_write(uint8_t *pdata,uint8_t Size,uint8_t select) ;  //ACC为1,GYRO为2
void Bmi0088_read_reg(uint8_t *pdata,uint8_t reg,uint8_t Size,uint8_t select);



extern SPI_HandleTypeDef hspi1;

#endif
