/*
 * CalibrationStore.c
 *
 *  Created on: Oct 5, 2021
 *      Author: thier
 */

#include "CalibrationStore.h"
#include "math.h"

static void __Flash_Read_Data(uint32_t StartPageAddress, uint32_t *RxBuf,
		uint16_t numberofwords) {
	while (1) {
		*RxBuf = *(uint32_t*) StartPageAddress;
		StartPageAddress += 4;
		RxBuf++;
		if (!(numberofwords--))
			break;
	}
}

static double limit(double value, int decimalPrecision) {
	int calcScale = (int) pow(10, decimalPrecision);
	return (double) round(value * calcScale) / calcScale;
}

static double map(double sourceNumber, double fromA, double fromB, double toA,
		double toB, int decimalPrecision) {
	double deltaA = fromB - fromA;
	double deltaB = toB - toA;
	double scale = deltaB / deltaA;
	double negA = -1 * fromA;
	double offset = (negA * scale) + toA;
	double finalNumber = (sourceNumber * scale) + offset;

	return limit(finalNumber, decimalPrecision);
}

static double CalibrationStore_ProcessAxis(double value, double __min,
		double __max) {
	return map(min(max( value, __min), __max), __min, __max, 0,
	CalibrationStore_BitSize, 2);
}

static double CalibrationStore_ProcessThrottle(double value, double __min,
		double __mil, double __max) {
	double maxValue = (((__mil + 0.5 - __min) / CalibrationStore_MIL) * 100)
			+ __min;
	return map(min(max( value, __min + CalibrationStore_Margin), maxValue),
			__min + CalibrationStore_Margin, maxValue, 0,
			CalibrationStore_BitSize, 2);
}

static double CalibrationStore_ProcessCenterAxis(double value, double __min,
		double center, double __max, double deadzonePercentage) {
	double offset =
			center - __min < __max - center ? center - __min : __max - center;
	double _min = center - offset;
	double _max = center + offset;
	double _deadzone = (_max - _min) * (deadzonePercentage / 100);

	value = min(max( value, _min), _max);
	value = value > center - _deadzone && value < center + _deadzone ? center : value;

	double result = map(value, _min, _max, 0, CalibrationStore_BitSize, 2);

	return result;
}

static double CalibrationStore_Filter(double value, double *store,
		uint8_t current) {
	store[current] = value;

	int i;
	double result;
	for (i = 0; i < CalibrationStore_FilterAmount; i++) {
		result += store[i];
	}

	return result / CalibrationStore_FilterAmount;
}

void CalibrationStore_Initialize(CalibrationStore *dev,
		CalibrationStoreValues *val) {
	memset(dev, 0, sizeof(CalibrationStore));

	dev->x_axis_min = val->x_axis;
	dev->x_axis_center = dev->x_axis_min / 2;
	dev->x_axis_max = val->x_axis;

	dev->y_axis_min = val->y_axis;
	dev->y_axis_center = dev->y_axis_min / 2;
	dev->y_axis_max = val->y_axis;

	dev->rx_min = val->rx;
	dev->rx_center = dev->rx_min / 2;
	dev->rx_max = val->rx;

	dev->ry_min = val->ry;
	dev->ry_max = val->ry;

	dev->rz_min = val->rz;
	dev->rz_mil = val->rz / 2;
	dev->rz_max = val->rz;
}

void CalibrationStore_Calibrate(CalibrationStore *dev,
		CalibrationStoreValues *val) {
	dev->x_axis_min = min(dev->x_axis_min || val->x_axis, val->x_axis);
	dev->x_axis_center = val->x_axis;
	dev->x_axis_max = max(dev->x_axis_max, val->x_axis);

	dev->y_axis_min = min(dev->y_axis_min || val->y_axis, val->y_axis);
	dev->y_axis_center = val->y_axis;
	dev->y_axis_max = max(dev->y_axis_max, val->y_axis);

	dev->rx_min = min(dev->rx_min == 0 ? val->rx : dev->rx_min, val->rx);
	dev->rx_center = val->rx;
	dev->rx_max = max(dev->rx_max, val->rx);

	dev->ry_min = min(dev->ry_min == 0 ? val->ry : dev->ry_min, val->ry);
	dev->ry_max = max(dev->ry_max, val->ry);

	dev->rz_min = min(dev->rz_min == 0 ? val->rz : dev->rz_min, val->rz);
	dev->rz_mil = val->rz;
	dev->rz_max = max(dev->rz_max, val->rz);
}

void CalibrationStore_Cursor(CalibrationStore *dev,
		CalibrationStoreValues *val) {
	dev->x_axis_min = min(dev->x_axis_min || val->x_axis, val->x_axis);
	dev->x_axis_center = val->x_axis;
	dev->x_axis_max = max(dev->x_axis_max, val->x_axis);

	dev->y_axis_min = min(dev->y_axis_min || val->y_axis, val->y_axis);
	dev->y_axis_center = val->y_axis;
	dev->y_axis_max = max(dev->y_axis_max, val->y_axis);
}

void CalibrationStore_CursorCenter(CalibrationStore *dev,
		CalibrationStoreValues *val) {
	dev->x_axis_center = val->x_axis;
	dev->y_axis_center = val->y_axis;
}

void CalibrationStore_Process(CalibrationStore *dev,
		CalibrationStoreValues *val) {

	val->currentF =
			val->currentF < CalibrationStore_FilterAmount - 1 ?
					val->currentF + 1 : 0;

	val->x_axis = (uint16_t)CalibrationStore_ProcessCenterAxis(val->x_axis,
			dev->x_axis_min, dev->x_axis_center, dev->x_axis_max, 2.5) - 32768;
//	val->x_axis = CalibrationStore_Filter(val->x_axis, val->x_axis_f,
//			val->currentF) - 32768;

	val->y_axis = (uint16_t)CalibrationStore_ProcessCenterAxis(val->y_axis,
			dev->y_axis_min, dev->y_axis_center, dev->y_axis_max, 2.5) - 32768;
//	val->y_axis = CalibrationStore_Filter(val->y_axis, &val->y_axis_f,
//			val->currentF) - 32768;

	val->rx = CalibrationStore_ProcessCenterAxis(val->rx, dev->rx_min,
			dev->rx_center, dev->rx_max, 3) - 32768;

	val->ry = CalibrationStore_ProcessAxis(val->ry, dev->ry_min, dev->ry_max)
			- 32768;

	val->cutoff = val->rz < (dev->rz_min - 5) ? 0x1 : 0x0;

	val->rz = CalibrationStore_ProcessThrottle(limit(val->rz, 1), dev->rz_min,
			dev->rz_mil, dev->rz_max);
	val->rz = CalibrationStore_Filter(val->rz, val->rx_f, val->currentF);
	val->rz = min(max( val->rz, 0), CalibrationStore_BitSize) - 32768;

}

uint32_t CalibrationStore_Store(CalibrationStore *dev) {

	uint32_t baseAddress = CalibrationStore_BaseAddress;
	int numofwords = (sizeof(CalibrationStore) / 4)
			+ ((sizeof(CalibrationStore) % 4) != 0);
	int sofar = 0;

	uint32_t data[numofwords];
	memcpy(data, dev, sizeof(CalibrationStore));

	HAL_FLASH_Unlock();

	/* Erase the user Flash area*/
	FLASH_Erase_Sector(CalibrationStore_Sector, VOLTAGE_RANGE_3);
	FLASH_WaitForLastOperation(100);

	HAL_Delay(500);

	/* Program the user Flash area word by word*/
	while (sofar < numofwords) {
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, baseAddress, data[sofar])
				== HAL_OK) {
			baseAddress += 4; // use StartPageAddress += 2 for half word and 8 for double word
			sofar++;
		} else {
			/* Error occurred while writing data in Flash memory*/
			return HAL_FLASH_GetError();
		}
		FLASH_WaitForLastOperation(10);
	}

	HAL_FLASH_Lock();

	return 0;
}

uint32_t CalibrationStore_Retrieve(CalibrationStore *dev) {
	int numofwords = (sizeof(CalibrationStore) / 4)
			+ ((sizeof(CalibrationStore) % 4) != 0);

	char *RxData[sizeof(CalibrationStore)];
	__Flash_Read_Data( CalibrationStore_BaseAddress, RxData, numofwords);

	memcpy(dev, RxData, sizeof(CalibrationStore));
}
