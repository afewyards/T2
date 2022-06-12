/*
 * ADS122C04.h
 *
 *  Created on: Oct 4, 2021
 *      Author: thier
 */

#ifndef INC_ADS122C04_H_
#define INC_ADS122C04_H_

#include "stm32f4xx_hal.h"

#define ADS122C04_I2C_ADDR_0		(0x40) << 1
#define ADS122C04_I2C_ADDR_1		(0x41) << 1

#define ADS122C04_RESET_CMD          0x06     //0000 011x      Reset
#define ADS122C04_START_CMD          0x08     //0000 100x      Start/Sync
#define ADS122C04_POWERDOWN_CMD      0x02     //0000 001x      PowerDown
#define ADS122C04_RDATA_CMD          0x10     //0001 xxxx      RDATA
#define ADS122C04_RREG_CMD           0x20     //0010 rrxx      Read REG rr= register address 00 to 11
#define ADS122C04_WREG_CMD           0x40     //0100 rrxx      Write REG rr= register address 00 to 11

#define ADS122C04_WRITE_CMD(reg)     (ADS122C04_WREG_CMD | (reg << 2))    //Shift is 2-bit in ADS122C04
#define ADS122C04_READ_CMD(reg)      (ADS122C04_RREG_CMD | (reg << 2))    //Shift is 2-bit in ADS122C04

// ADS122C04 Table 16 in Datasheet
#define ADS122C04_CONFIG_0_REG      0 // Configuration Register 0
#define ADS122C04_CONFIG_1_REG      1 // Configuration Register 1
#define ADS122C04_CONFIG_2_REG      2 // Configuration Register 2
#define ADS122C04_CONFIG_3_REG      3 // Configuration Register 3

#define ADS122C04_MUX_AIN0		0x8
#define ADS122C04_MUX_AIN1		0x9
#define ADS122C04_MUX_AIN2		0xA
#define ADS122C04_MUX_AIN3		0xB

#define ADS122C04_MUX_AIN0P1N   0x3
#define ADS122C04_MUX_AIN2P3N   0x7

#define ADS122C04_GAIN			0x0 // 1 gain
#define ADS122C04_PGA_BYPASS	0x0 // enabled

typedef struct {
	I2C_HandleTypeDef *i2cHandle;
	float ain[4];
	uint8_t ready;
	uint8_t current;
	uint8_t interrupt;
	uint8_t addr;
	uint8_t gain;
	uint8_t mux[4];
	uint8_t muxLenght;
} ADS122C04;

uint8_t ADS122C04_Initialize( ADS122C04 *dev, I2C_HandleTypeDef *i2cHandle, uint8_t addr );
void ADS122C04_ProcessInterrupt( ADS122C04 *dev );
void ADS122C04_SetGain( ADS122C04 *dev, uint8_t gain );
HAL_StatusTypeDef ADS122C04_ReadAll( ADS122C04 *dev );

HAL_StatusTypeDef ADS122C04_ReadRegister( ADS122C04 *dev, uint8_t reg, uint8_t *data, uint8_t length );
HAL_StatusTypeDef ADS122C04_WriteRegister( ADS122C04 *dev, uint8_t reg, uint8_t *data, uint8_t length );

#endif /* INC_ADS122C04_H_ */
