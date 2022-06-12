/*
 * MLX90393.h
 *
 *  Created on: Oct 4, 2021
 *      Author: thier
 */

#ifndef INC_MLX90393_H_
#define INC_MLX90393_H_

#include "stm32f4xx_hal.h"

/*
 * DEFINES
 */
#define MLX90393_I2C_ADDR		(0x10 << 1)

#define MLX90393_I2C_CMD_SB		0x10
#define MLX90393_I2C_CMD_SW		0x20
#define MLX90393_I2C_CMD_SM		0x30
#define MLX90393_I2C_CMD_RM		0x40
#define MLX90393_I2C_CMD_RR		0x50
#define MLX90393_I2C_CMD_WR		0x60
#define MLX90393_I2C_CMD_EX		0x80
#define MLX90393_I2C_CMD_HR		0xD0
#define MLX90393_I2C_CMD_HS		0xE0
#define MLX90393_I2C_CMD_RT		0xF0

#define MLX90393_REG_1			0x00
#define MLX90393_REG_2			0x01
#define MLX90393_REG_3			0x02
#define MLX90393_REG_4			0x03
#define MLX90393_REG_5			0x04
#define MLX90393_REG_6			0x05
#define MLX90393_REG_7			0x06
#define MLX90393_REG_8			0x07
#define MLX90393_REG_9			0x08
#define MLX90393_REG_10			0x09

#define MLX90393_REG_Z_SERIES	0x0
#define MLX90393_REG_GAIN_SEL	0x0
#define MLX90393_REG_HALLCONF	0x0

typedef enum mlx90393_filter {
  MLX90393_FILTER_0,
  MLX90393_FILTER_1,
  MLX90393_FILTER_2,
  MLX90393_FILTER_3,
  MLX90393_FILTER_4,
  MLX90393_FILTER_5,
  MLX90393_FILTER_6,
  MLX90393_FILTER_7,
} mlx90393_filter_t;

typedef struct {
	I2C_HandleTypeDef *i2cHandle;
	int16_t x;
	int16_t y;
	uint8_t ready;
} MLX90393;


HAL_StatusTypeDef MLX90393_Initialize( MLX90393 *dev, I2C_HandleTypeDef *i2cHandle );

void MLX90393_ProcessInterrupt( MLX90393 *dev );
HAL_StatusTypeDef MLX90393_ReadFromInterrupt( MLX90393 *dev );

HAL_StatusTypeDef MLX90393_ReadRegister( MLX90393 *dev, uint8_t reg, uint8_t *data, uint8_t length );
HAL_StatusTypeDef MLX90393_WriteRegister( MLX90393 *dev, uint8_t reg, uint8_t *data, uint8_t length );
HAL_StatusTypeDef MLX90393_SetFilter( MLX90393 *dev, uint8_t filter );
double MLX90393_Rotation( MLX90393 *dev );

#endif /* INC_MLX90393_H_ */
