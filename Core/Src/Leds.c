
#include "Leds.h"

void LEDS_Initialize(LEDS *leds, uint16_t delay) {
	leds->current = 0;
	leds->delay = delay;
	leds->last = HAL_GetTick();
}

void LEDS_Set(uint8_t id, uint8_t state) {
	switch (id) {
	case 0:
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, state);
		break;
	case 1:
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, state);
		break;
	case 2:
		HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, state);
		break;
	case 3:
		HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, state);
		break;
	}
}

void LEDS_Run(LEDS *leds, uint8_t reverse) {
	uint32_t time = HAL_GetTick();

	if (time > leds->last + leds->delay) {
		leds->last = time;

		LEDS_Set(leds->current, 1);
		reverse ? leds->current == 0 ? leds->current = 3 : leds->current--
		: leds->current == 3 ? leds->current = 0 : leds->current++;
		LEDS_Set(leds->current, 0);
	}
}

void LEDS_SetDelay(LEDS *leds, uint16_t delay) {
	leds->delay = delay;
}
