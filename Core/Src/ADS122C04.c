/*
 * ADS122C04.c
 *
 *  Created on: Oct 4, 2021
 *      Author: thier
 */

#include "ADS122C04.h"


int a;
int b;

static uint8_t ADS122C04_GetDRDY(ADS122C04 *dev) {
	uint8_t rxData[1];

	ADS122C04_ReadRegister(dev, ADS122C04_READ_CMD(ADS122C04_CONFIG_2_REG),
			rxData, 1);
	uint8_t flag = ((rxData[0] >> 7) & 0x01);

	return flag;
}

static HAL_StatusTypeDef ADS122C04_StartCurrent(ADS122C04 *dev) {
	// do new reading
	HAL_StatusTypeDef result;

	uint8_t configData[1] = { dev->mux[dev->current] << 4 | dev->gain << 1
			| ADS122C04_PGA_BYPASS };
	result = ADS122C04_WriteRegister(dev,
			ADS122C04_WRITE_CMD(ADS122C04_CONFIG_0_REG), configData, 1);

	if (result == HAL_OK) {
		uint8_t startData = 0;
		result = ADS122C04_WriteRegister(dev, ADS122C04_START_CMD, &startData,
				0);
	}

	return result;
}

static HAL_StatusTypeDef ADS122C04_ReadCurrent(ADS122C04 *dev) {
	HAL_StatusTypeDef result;

	if (dev->ready) {
		result = ADS122C04_StartCurrent(dev);

		if (result == HAL_OK) {
			dev->ready = 0;
		}
	}

	uint8_t flag = ADS122C04_GetDRDY(dev);
	if (flag) {
		uint8_t data[3];

		result = ADS122C04_ReadRegister(dev, ADS122C04_RDATA_CMD, data, 3);

		if (result == HAL_OK) {
			uint32_t r = ((uint32_t) data[2]) | ((uint32_t) data[1] << 8)
					| ((uint32_t) data[0] << 16);

			dev->ready = 1;
			dev->interrupt = 0;

			if (dev->addr == ADS122C04_I2C_ADDR_1) {
				r = (r << 8) - 4294967295 / 2;
				r = r >> 8;
				a = r;
			}

			dev->ain[dev->current] = r;
			dev->current = dev->current + 1;
		}
	}

	return result;
}

uint8_t ADS122C04_Initialize(ADS122C04 *dev, I2C_HandleTypeDef *i2cHandle,
		uint8_t addr) {
	dev->i2cHandle = i2cHandle;

	dev->ain[0] = 0.0f;
	dev->ain[1] = 0.0f;
	dev->ain[2] = 0.0f;
	dev->ain[3] = 0.0f;

	dev->ready = 1;
	dev->current = 0;
	dev->addr = addr;

	if (addr == ADS122C04_I2C_ADDR_0) {
		dev->gain = ADS122C04_GAIN;
		dev->mux[0] = ADS122C04_MUX_AIN0;
		dev->mux[1] = ADS122C04_MUX_AIN1;
		dev->mux[2] = ADS122C04_MUX_AIN2;
		dev->mux[3] = ADS122C04_MUX_AIN3;
		dev->muxLenght = 4;
	} else {
		dev->gain = 0x7;
		dev->mux[0] = ADS122C04_MUX_AIN0P1N;
		dev->mux[1] = ADS122C04_MUX_AIN2P3N;
		dev->muxLenght = 2;
	}

	// 330 sps, normal, single, refp refn, no temp
	uint8_t configData[1] = { 0x82 };
	ADS122C04_WriteRegister(dev, ADS122C04_WRITE_CMD(ADS122C04_CONFIG_1_REG),
			configData, 1);
	return 0;
}

void ADS122C04_ProcessInterrupt(ADS122C04 *dev) {
	dev->interrupt = 1;
}

void ADS122C04_SetGain(ADS122C04 *dev, uint8_t gain) {
	dev->gain = gain;
}

HAL_StatusTypeDef ADS122C04_ReadAll(ADS122C04 *dev) {
	HAL_StatusTypeDef result = 0;

	while (dev->current < dev->muxLenght) {
		result = ADS122C04_ReadCurrent(dev);
	}

	dev->ready = 1;
	dev->current = 0;

	return result;
}

HAL_StatusTypeDef ADS122C04_ReadRegister(ADS122C04 *dev, uint8_t reg,
		uint8_t *data, uint8_t length) {
	return HAL_I2C_Mem_Read(dev->i2cHandle, dev->addr, reg,
	I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY);
}

HAL_StatusTypeDef ADS122C04_WriteRegister(ADS122C04 *dev, uint8_t reg,
		uint8_t *data, uint8_t length) {
	return HAL_I2C_Mem_Write(dev->i2cHandle, dev->addr, reg,
	I2C_MEMADD_SIZE_8BIT, data, length,
	HAL_MAX_DELAY);
}

