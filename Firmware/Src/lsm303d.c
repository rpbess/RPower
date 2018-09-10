#include "stm32f1xx_hal.h"
#include "lsm303d.h"

extern I2C_HandleTypeDef hi2c2;

LsmDataTypeDef lsm_read_acc(void)
{
	uint8_t data[6];
	LsmDataTypeDef result;
	HAL_I2C_Mem_Read(&hi2c2, LSM_ADDRESS, 0xA8, I2C_MEMADD_SIZE_8BIT, data, 6, 200); 
	result.X = ((int16_t)(data[1] << 8 | data[0])) / 16.382;
	result.Y = ((int16_t)(data[3] << 8 | data[2])) / 16.382;
	result.Z = ((int16_t)(data[5] << 8 | data[4])) / 16.382;
	return result;
}

LsmDataTypeDef lsm_read_mag(void)
{
	uint8_t data[6];
	LsmDataTypeDef result;
	HAL_I2C_Mem_Read(&hi2c2, LSM_ADDRESS, 0x88, I2C_MEMADD_SIZE_8BIT, data, 6, 200); 
	result.X = ((data[1] << 8 | data[0])) / 16.382;
	result.Y = ((data[3] << 8 | data[2])) / 16.382;
	result.Z = ((data[5] << 8 | data[4])) / 16.382;
	return result;
}

char lsm_present()
{
	uint8_t buffer[2];
	HAL_I2C_Mem_Read(&hi2c2, LSM_ADDRESS, 0xF, I2C_MEMADD_SIZE_8BIT, buffer, 2, 200); 
	if(buffer[0] == 0x49)return 1;
	return 0;
}

void lsm_init()
{	
	if(lsm_present()){
		uint8_t buffer[1] = {0x00};
		HAL_I2C_Mem_Write(&hi2c2, LSM_ADDRESS, 0x1F, I2C_MEMADD_SIZE_8BIT, buffer, 1, 200);
		HAL_Delay(50);
		
		buffer[0] = 0x4F;
		HAL_I2C_Mem_Write(&hi2c2, LSM_ADDRESS, 0x20, I2C_MEMADD_SIZE_8BIT, buffer, 1, 200);
		HAL_Delay(50);
		
		buffer[0] = 0xC0;
		HAL_I2C_Mem_Write(&hi2c2, LSM_ADDRESS, 0x21, I2C_MEMADD_SIZE_8BIT, buffer, 1, 200);
		HAL_Delay(50);
		
		buffer[0] = 0x00;
		HAL_I2C_Mem_Write(&hi2c2, LSM_ADDRESS, 0x23, I2C_MEMADD_SIZE_8BIT, buffer, 1, 200);
		HAL_Delay(50);
		
		buffer[0] = 0xF0;
		HAL_I2C_Mem_Write(&hi2c2, LSM_ADDRESS, 0x24, I2C_MEMADD_SIZE_8BIT, buffer, 1, 200);
		HAL_Delay(50);
		
		buffer[0] = 0x80;
		HAL_I2C_Mem_Write(&hi2c2, LSM_ADDRESS, 0x26, I2C_MEMADD_SIZE_8BIT, buffer, 1, 200);
	}
}


