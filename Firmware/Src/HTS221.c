#include "HTS221.h"
#include "stm32f1xx_hal.h"

extern I2C_HandleTypeDef hi2c2;

void read_i2c(uint8_t register_addr, int num, uint8_t *buf) {
  HAL_I2C_Mem_Read(&hi2c2, HTS221_ADDRESS, register_addr, I2C_MEMADD_SIZE_8BIT, buf, num, 200);       
}

void hts221_init(void)
{
	uint8_t buffer[1] = {0x81};
	HAL_I2C_Mem_Write(&hi2c2, HTS221_ADDRESS, HTS221_CTRL_REG1, I2C_MEMADD_SIZE_8BIT, buffer, 1, 200);
}

float hts221_getHumidity()
{
  uint16_t H0_T0_out, H1_T0_out, H_T_out;
  uint16_t H0_rh, H1_rh;
  uint8_t buffer[2];
  uint32_t humidity;
  read_i2c(HTS221_H0_RH_X2, 1, &buffer[0]);
  read_i2c(HTS221_H1_RH_X2, 1, &buffer[1]);
  H0_rh = (buffer[0]&0xff)>>1;
  H1_rh = (buffer[1]&0xff)>>1;
  read_i2c(HTS221_H0_T0_OUT_L, 1, &buffer[0]);
  read_i2c(HTS221_H0_T0_OUT_H, 1, &buffer[1]);
  H0_T0_out = (((uint16_t)buffer[1])<<8) | (uint16_t)buffer[0];
  read_i2c(HTS221_H1_T0_OUT_L, 1, &buffer[0]);
  read_i2c(HTS221_H1_T0_OUT_H, 1, &buffer[1]);  
  H1_T0_out = (((uint16_t)buffer[1])<<8) | (uint16_t)buffer[0];
  read_i2c(HTS221_HR_OUT_L_REG, 1, &buffer[0]);
  read_i2c(HTS221_HR_OUT_H_REG, 1, &buffer[1]);  
  H_T_out = buffer[1]<<8 | buffer[0];
  double rh_dx = ((int16_t)H1_rh - (int16_t)H0_rh)*10;
  humidity = ((int16_t)H_T_out - (int16_t)H0_T0_out)*rh_dx / ((int16_t)H1_T0_out - (int16_t)H0_T0_out);
  humidity = (int16_t)(humidity + H0_rh*10);
  if(humidity > 1000){ humidity = 1000; }
  return humidity*0.1;
}


float hts221_getTemperature()
{
  int16_t T0_out, T1_out, T_out, T0_degC_x8_u16, T1_degC_x8_u16;
  int16_t T0_degC, T1_degC;
  uint8_t buffer[4];
  uint32_t tmp32;
  read_i2c(HTS221_T0_DEGC_X8, 1, &buffer[0]);
  read_i2c(HTS221_T1_DEGC_X8, 1, &buffer[1]);
  read_i2c(HTS221_T0_T1_DEGC_H2, 1, &buffer[2]);
  T0_degC_x8_u16 = (((uint16_t)(buffer[2] & 0x03)) << 8) | ((uint16_t)buffer[0]);
  T1_degC_x8_u16 = (((uint16_t)(buffer[2] & 0x0C)) << 6) | ((uint16_t)buffer[1]);
  T0_degC = T0_degC_x8_u16>>3;
  T1_degC = T1_degC_x8_u16>>3;
  read_i2c(HTS221_T0_OUT_L, 1, &buffer[0]);
  read_i2c(HTS221_T0_OUT_H, 1, &buffer[1]);
  read_i2c(HTS221_T1_OUT_L, 1, &buffer[2]);
  read_i2c(HTS221_T1_OUT_H, 1, &buffer[3]);
  T0_out = (((uint16_t)buffer[1])<<8) | (uint16_t)buffer[0];
  T1_out = (((uint16_t)buffer[3])<<8) | (uint16_t)buffer[2];
  read_i2c(HTS221_TEMP_OUT_L, 1, &buffer[0]);
  read_i2c(HTS221_TEMP_OUT_H, 1, &buffer[1]);
  T_out = (((uint16_t)buffer[1])<<8) | (uint16_t)buffer[0];
  tmp32 = ((uint32_t)(T_out - T0_out)) * ((uint32_t)(T1_degC - T0_degC)*10);
  return (tmp32 /(T1_out - T0_out) + T0_degC*10)*0.1;
}
