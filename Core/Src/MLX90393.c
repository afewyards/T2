/*
 * MLX90393.c
 *
 *  Created on: Oct 4, 2021
 *      Author: thier
 */

#include "MLX90393.h"
#include <stdlib.h>
#include <math.h>

#define pi acos(-1.0)

HAL_StatusTypeDef MLX90393_Initialize(MLX90393 *dev,
		I2C_HandleTypeDef *i2cHandle) {
	HAL_StatusTypeDef result = HAL_OK;

	dev->i2cHandle = i2cHandle;
	dev->x = 0;
	dev->y = 0;
	dev->ready = 0;

	uint8_t configData[2] = { MLX90393_REG_Z_SERIES
			<< 7| MLX90393_REG_GAIN_SEL << 4 | MLX90393_REG_HALLCONF, 0xE0 };
	uint8_t data = 0x0;

	result = MLX90393_SetFilter(dev, MLX90393_FILTER_6);
	// set registers config
	result = MLX90393_WriteRegister(dev, MLX90393_REG_1, configData, 2);

	// start burst mode
	result = MLX90393_WriteRegister(dev, MLX90393_I2C_CMD_SB | 0x6, &data, 0);

	return result;
}

HAL_StatusTypeDef MLX90393_ReadFromInterrupt(MLX90393 *dev) {
	HAL_StatusTypeDef result = HAL_OK;

	uint8_t data[5];

	// request the measurements for the x and y axis

//	if (dev->ready == 1) {
		result = MLX90393_ReadRegister(dev, MLX90393_I2C_CMD_RM | 0x6, data, 5);

		if (result == HAL_OK) {
			dev->x = ((uint16_t) data[2]) | ((uint16_t) data[1] << 8);
			dev->y = ((uint16_t) data[4]) | ((uint16_t) data[3] << 8);
		}
//		dev->ready = 0;
//	}

	return result;
}

void MLX90393_ProcessInterrupt(MLX90393 *dev) {
	dev->ready = 1;
}

HAL_StatusTypeDef MLX90393_ReadRegister(MLX90393 *dev, uint8_t reg,
		uint8_t *data, uint8_t length) {
	return HAL_I2C_Mem_Read(dev->i2cHandle, MLX90393_I2C_ADDR, reg,
	I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MLX90393_WriteRegister(MLX90393 *dev, uint8_t reg,
		uint8_t *data, uint8_t length) {
	return HAL_I2C_Mem_Write(dev->i2cHandle, MLX90393_I2C_ADDR, reg,
	I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MLX90393_SetFilter(MLX90393 *dev, mlx90393_filter_t filter) {
	uint8_t data;
	MLX90393_ReadRegister(dev, MLX90393_REG_3, &data, 1);

	data &= ~0x1C;
	data |= filter << 2;

	return MLX90393_WriteRegister(dev, MLX90393_REG_3, &data, 1);
}

double MLX90393_Rotation(MLX90393 *dev) {
	double radians = 0.0;

	double x = abs(dev->x);
	double y = abs(dev->y);

	if (dev->x > 0 && dev->y > 0) {
		radians = atan(y / x) * (180 / pi);
	} else if (dev->x < 0 && dev->y > 0) {
		radians = 180 - (atan(y / x) * (180 / pi));
	} else if (dev->x < 0 && dev->y < 0) {
		radians = (atan(y / x) * (180 / pi)) + 180;
	} else if (dev->x > 0 && dev->y < 0) {
		radians = 360 - (atan(y / x) * (180 / pi));
	}

	return radians;
}

