/*
 * PCAL6524.h
 *
 *  Created on: Oct 4, 2021
 *      Author: thier
 */

#ifndef INC_PCAL6524_H_
#define INC_PCAL6524_H_

#include "stm32f4xx_hal.h"

#define PCAL6524_I2C_ADDR			(0x22 << 1)
#define PCAL6524_I2C_REG_INPUT_0	0x00
#define PCAL6524_I2C_REG_INPUT_1	0x01
#define PCAL6524_I2C_REG_INPUT_2	0x02

#define PCAL6524_I2C_REG_INVERS_0	0x08
#define PCAL6524_I2C_REG_INVERS_1	0x09
#define PCAL6524_I2C_REG_INVERS_2	0x0A

typedef struct {
	I2C_HandleTypeDef *i2cHandle;
	uint32_t inputs;
	uint8_t ready;
} PCAL6524;

uint8_t PCAL6524_Initialize( PCAL6524 *dev, I2C_HandleTypeDef *i2cHandle );

HAL_StatusTypeDef PCAL6524_ReadFromInterrupt( PCAL6524 *dev );
void PCAL6524_ProcessInterrupt( PCAL6524 *dev );

HAL_StatusTypeDef PCAL6524_ReadRegister( PCAL6524 *dev, uint8_t reg, uint8_t *data, uint8_t length );
HAL_StatusTypeDef PCAL6524_WriteRegister( PCAL6524 *dev, uint8_t reg, uint8_t *data, uint8_t length );

uint8_t PCAL6524_ReadBit( PCAL6524 *dev, uint8_t bit ) ;

#endif /* INC_PCAL6524_H_ */
