/*
 * Leds.h
 *
 *  Created on: Oct 5, 2021
 *      Author: thier
 */

#ifndef INC_LEDS_H_
#define INC_LEDS_H_

#include "main.h"

typedef struct {
	uint8_t current;
	int last;
	int delay;
} LEDS;

void LEDS_Initialize(LEDS *leds, uint16_t delay);
void LEDS_Set(uint8_t id, uint8_t state);
void LEDS_Run(LEDS *leds, uint8_t reverse);
void LEDS_SetDelay(LEDS *leds, uint16_t delay);


#endif /* INC_LEDS_H_ */
