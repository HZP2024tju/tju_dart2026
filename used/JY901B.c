#include "JY901B.h"
#include "i2c.h"
#include "mitrx.h"

#define JY901L_I2C_ADDR 0x50
#define I2C_angle_ADDR 0x3d
#define I2C_Angular_v_ADDR 0x37
#define I2C_LEN 6

JY901_data JY_data;

JY901_Kalman_struct kf_x;
JY901_Kalman_struct kf_y;
JY901_Kalman_struct kf_z;

float Kalman_mitrx_X[2 * 2]; 
float Kalman_mitrx_Y[2 * 2]; 
float Kalman_mitrx_Z[2 * 2]; 


HAL_StatusTypeDef JY901L_ReadAngles() 
{
   uint8_t buffer[6]; 
   HAL_StatusTypeDef state= HAL_I2C_Mem_Read(&hi2c1,JY901L_I2C_ADDR<<1,I2C_angle_ADDR,I2C_MEMADD_SIZE_8BIT,buffer,I2C_LEN,50);
	//state= HAL_I2C_Mem_Read(&hi2c1,JY901L_I2C_ADDR<<1,I2C_start_ADDR,I2C_MEMADD_SIZE_8BIT,buffer,I2C_LEN,50)
	if (state!=HAL_OK) return state;
    int16_t x_raw = (buffer[1] << 8) | buffer[0];
    int16_t y_raw = (buffer[3] << 8) | buffer[2];
    int16_t z_raw = (buffer[5] << 8) | buffer[4];
    
   
    JY_data.ANGLE_x = (float)(x_raw / 32768.0f) * 180.0f;
    JY_data.ANGLE_y = (float)(y_raw / 32768.0f )* 180.0f;
    JY_data.ANGLE_z= (float)(z_raw / 32768.0f )* 180.0f;
    
    return HAL_OK;
}
HAL_StatusTypeDef JY901L_ReadAngular_v() 
{
     uint8_t buffer[6];
   HAL_StatusTypeDef state= HAL_I2C_Mem_Read(&hi2c1,JY901L_I2C_ADDR<<1,I2C_Angular_v_ADDR,I2C_MEMADD_SIZE_8BIT,buffer,I2C_LEN,50);
	//state= HAL_I2C_Mem_Read(&hi2c1,JY901L_I2C_ADDR<<1,I2C_start_ADDR,I2C_MEMADD_SIZE_8BIT,buffer,I2C_LEN,50)
	if (state!=HAL_OK) return state;
    int16_t x_raw = (buffer[1] << 8) | buffer[0];
    int16_t y_raw = (buffer[3] << 8) | buffer[2];
    int16_t z_raw = (buffer[5] << 8) | buffer[4];
    
   
    JY_data.gyro_x = (float)(x_raw / 32768.0f )* 2000.0f;
    JY_data.gyro_y = (float)(y_raw / 32768.0f )* 2000.0f;
    JY_data.gyro_z= (float)(z_raw / 32768.0f) * 2000.0f;
    
    return HAL_OK;
}

void JY901_Kalman_init(JY901_Kalman_struct * kf,float *kf_mitrx,float *kf_H)
{
		kf->K = kf_mitrx;
		kf->H = kf_H;
} 


void JY901_GetData(float * data_x)
{
	JY901L_ReadAngles();
	JY901L_ReadAngular_v();
	
	data_x[0] = JY_data.gyro_x;
	data_x[1] = 0;//JY_data.gyro_y;
	data_x[2] = 0;//JY_data.gyro_z;
	data_x[3] = JY_data.ANGLE_x;
	
	
}

