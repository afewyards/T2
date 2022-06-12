/*
 * CalibrationStore.h
 *
 *  Created on: Oct 5, 2021
 *      Author: thier
 */

#ifndef INC_CALIBRATIONSTORE_H_
#define INC_CALIBRATIONSTORE_H_

#define CalibrationStore_Sector 		5
#define CalibrationStore_BaseAddress 	0x08020000
#define CalibrationStore_BitSize		0xffff
#define CalibrationStore_MIL			75
#define CalibrationStore_Margin			0.5
#define CalibrationStore_FilterAmount	7


#ifndef max
#define max(a,b)            			(((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            			(((a) < (b)) ? (a) : (b))
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>


typedef struct {
	uint32_t x_axis_min;
	uint32_t x_axis_center;
	uint32_t x_axis_max;

	uint32_t y_axis_min;
	uint32_t y_axis_center;
	uint32_t y_axis_max;

	uint32_t rx_min;
	uint32_t rx_center;
	uint32_t rx_max;

	uint32_t ry_min;
	uint32_t ry_max;

	double rz_min;
	double rz_mil;
	double rz_max;
} CalibrationStore;

typedef struct {
	double x_axis;
	double y_axis;
	double rx;
	double ry;
	double rz;

	uint8_t cutoff;
	uint8_t currentF;

	double x_axis_f[CalibrationStore_FilterAmount];
	double y_axis_f[CalibrationStore_FilterAmount];
	double rx_f[CalibrationStore_FilterAmount];
	double ry_f[CalibrationStore_FilterAmount];
	double rz_f[CalibrationStore_FilterAmount];
} CalibrationStoreValues;

void CalibrationStore_Initialize(CalibrationStore *dev, CalibrationStoreValues *val);
void CalibrationStore_Calibrate(CalibrationStore *dev,
		CalibrationStoreValues *val);
void CalibrationStore_CursorCenter(CalibrationStore *dev,
		CalibrationStoreValues *val);
void CalibrationStore_Cursor(CalibrationStore *dev,
		CalibrationStoreValues *val);

void CalibrationStore_Process(CalibrationStore *dev,
		CalibrationStoreValues *val);
uint32_t CalibrationStore_Store(CalibrationStore *dev);
uint32_t CalibrationStore_Retrieve(CalibrationStore *dev);

#endif /* INC_CALIBRATIONSTORE_H_ */
