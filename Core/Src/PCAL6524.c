/*
 * PCAL6524.c
 *
 *  Created on: Oct 6, 2021
 *      Author: thier
 */

#include "PCAL6524.h"

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

uint8_t PCAL6524_Initialize(PCAL6524 *dev, I2C_HandleTypeDef *i2cHandle) {
	dev->i2cHandle = i2cHandle;
	dev->inputs = 0x0;
	dev->ready = 0;

	// set registers config
	unsigned char data = 0xff;
	PCAL6524_WriteRegister(dev, PCAL6524_I2C_REG_INVERS_0, &data, 1);
	PCAL6524_WriteRegister(dev, PCAL6524_I2C_REG_INVERS_1, &data, 1);

	return 0;
}

HAL_StatusTypeDef PCAL6524_ReadFromInterrupt( PCAL6524 *dev ) {
	HAL_StatusTypeDef result = HAL_OK;

//	if (dev->ready == 1) {
		dev->ready = 0;
		unsigned char buffer0;
		unsigned char buffer1;

		// request the measurements for the x and y axis
		result = PCAL6524_ReadRegister( dev, PCAL6524_I2C_REG_INPUT_0, &buffer0, 1 );
		result = PCAL6524_ReadRegister( dev, PCAL6524_I2C_REG_INPUT_1, &buffer1, 1 );

		if (result == HAL_OK) {
			dev->inputs = (buffer1 << 8 ) | buffer0 ;
		}
//	}

	return result;
}

void PCAL6524_ProcessInterrupt( PCAL6524 *dev ) {
	dev->ready			= 1;
}

HAL_StatusTypeDef PCAL6524_ReadRegister( PCAL6524 *dev, uint8_t reg, uint8_t *data, uint8_t length ) {
	return HAL_I2C_Mem_Read( dev->i2cHandle, PCAL6524_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY);
}

HAL_StatusTypeDef PCAL6524_WriteRegister( PCAL6524 *dev, uint8_t reg, uint8_t *data, uint8_t length ) {
	return HAL_I2C_Mem_Write( dev->i2cHandle, PCAL6524_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY);
}

uint8_t PCAL6524_ReadBit( PCAL6524 *dev, uint8_t bit ) {
	return (dev->inputs & (1 << (bit-1))) != 0;
}
