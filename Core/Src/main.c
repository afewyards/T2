/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MLX90393.h"
#include "ADS122C04.h"
#include "PCAL6524.h"
#include "CalibrationStore.h"
#include "Leds.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
	int16_t x;
	int16_t y;
	int16_t rx;
	int16_t ry;
	int16_t rz;
	uint16_t buttons;
	uint8_t cutoff;
} UsbData;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

/* USER CODE BEGIN PV */
CalibrationStore calibration;
CalibrationStoreValues values;
MLX90393 hall;
ADS122C04 adc;
ADS122C04 cursor;
PCAL6524 buttons;
LEDS ledData;
UsbData usbD;

uint32_t calibrationLimit;
uint32_t tick;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_USB_DEVICE_DeInit();
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C3_Init(void);
/* USER CODE BEGIN PFP */
static void JumpToBootloader();
static void HAL_NVIC_Enable();
static void SYNC();
static void AXIS_Assign(CalibrationStoreValues *_dev, ADS122C04 *_adc,
		ADS122C04 *_cursor, MLX90393 *_hall);
int8_t CUSTOM_HID_OutEvent_FS(uint8_t *state);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();
	MX_USB_DEVICE_Init();
	MX_I2C3_Init();
	/* USER CODE BEGIN 2 */

	PCAL6524_Initialize(&buttons, &hi2c1);
	PCAL6524_ReadFromInterrupt(&buttons);

	if (PCAL6524_ReadBit(&buttons, 5)) {
		__enable_irq();

		MX_USB_DEVICE_DeInit();

		HAL_RCC_DeInit();
		HAL_DeInit();

		SysTick->CTRL = SysTick->LOAD = SysTick->VAL = 0;
		__HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

		const uint32_t p = (*((uint32_t*) 0x1FFF0000));
		__set_MSP(p);

		void (*SysMemBootJump)(void);
		SysMemBootJump = (void (*)(void)) (*((uint32_t*) 0x1FFF0004));
		SysMemBootJump();

		while (1)
			;
	}

	HAL_NVIC_Enable();

	LEDS_Initialize(&ledData, 50);
	MLX90393_Initialize(&hall, &hi2c3);
	ADS122C04_Initialize(&adc, &hi2c1, ADS122C04_I2C_ADDR_0);
	ADS122C04_Initialize(&cursor, &hi2c1, ADS122C04_I2C_ADDR_1);

	// check for active buttons, if button for calibrate set timeout
	if (PCAL6524_ReadBit(&buttons, 1)) {
		calibrationLimit = HAL_GetTick() + 20000;
		tick = HAL_GetTick();

		while ((tick = HAL_GetTick()) < calibrationLimit) {
			MLX90393_ReadFromInterrupt(&hall);
			ADS122C04_ReadAll(&adc);
			ADS122C04_ReadAll(&cursor);

			AXIS_Assign(&values, &adc, &cursor, &hall);
			CalibrationStore_Calibrate(&calibration, &values);

			double percentage = (double) tick / 15000;

			LEDS_SetDelay(&ledData, 500 - percentage * 500);
			LEDS_Run(&ledData, 1);
		}

		LEDS_Set(0, 0);
		LEDS_Set(1, 0);
		LEDS_Set(2, 0);
		LEDS_Set(3, 0);

		LEDS_SetDelay(&ledData, 50);

		CalibrationStore_Store(&calibration);
	} else if (PCAL6524_ReadBit(&buttons, 2)) {
		CalibrationStore_Retrieve(&calibration);
		calibrationLimit = HAL_GetTick() + 10000;
		tick = HAL_GetTick();

		calibration.x_axis_center = 0;
		calibration.x_axis_min = 0;
		calibration.x_axis_max = 0;
		calibration.y_axis_center = 0;
		calibration.y_axis_min = 0;
		calibration.y_axis_max = 0;

		while ((tick = HAL_GetTick()) < calibrationLimit) {
			ADS122C04_ReadAll(&cursor);
			AXIS_Assign(&values, &adc, &cursor, &hall);
			CalibrationStore_Cursor(&calibration, &values);
			double percentage = (double) tick / 15000;

			LEDS_SetDelay(&ledData, 500 - percentage * 500);
			LEDS_Run(&ledData, 1);
		}

		LEDS_Set(0, 0);
		LEDS_Set(1, 0);
		LEDS_Set(2, 0);
		LEDS_Set(3, 0);
		LEDS_SetDelay(&ledData, 50);
		CalibrationStore_Store(&calibration);

	} else {
		CalibrationStore_Retrieve(&calibration);
		ADS122C04_ReadAll(&cursor);
		AXIS_Assign(&values, &adc, &cursor, &hall);
		CalibrationStore_CursorCenter(&calibration, &values);
	}

	LEDS_Set(0, 1);
	LEDS_Set(1, 1);
	LEDS_Set(2, 1);
	LEDS_Set(3, 1);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		SYNC();

		CalibrationStore_Process(&calibration, &values);

		usbD.x = (int16_t) values.x_axis;
		usbD.y = (int16_t) values.y_axis;

		usbD.rx = (int16_t) values.rx;
		usbD.ry = (int16_t) values.ry;
		usbD.rz = (int16_t) values.rz;

		usbD.cutoff = values.cutoff;
		usbD.buttons = buttons.inputs;

		CUSTOM_HID_OutEvent_FS((uint8_t*) &usbD);
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 400;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 8;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
		Error_Handler();
	}
	/** Enables the Clock Security System
	 */
	HAL_RCC_EnableCSS();
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 400000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief I2C3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C3_Init(void) {

	/* USER CODE BEGIN I2C3_Init 0 */

	/* USER CODE END I2C3_Init 0 */

	/* USER CODE BEGIN I2C3_Init 1 */

	/* USER CODE END I2C3_Init 1 */
	hi2c3.Instance = I2C3;
	hi2c3.Init.ClockSpeed = 400000;
	hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c3.Init.OwnAddress1 = 0;
	hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c3.Init.OwnAddress2 = 0;
	hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C3_Init 2 */

	/* USER CODE END I2C3_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LED2_Pin | LED3_Pin | LED4_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : INT_CURSOR_Pin INT_HALL_Pin */
	GPIO_InitStruct.Pin = INT_CURSOR_Pin | INT_HALL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : INT_ADC_Pin INT_BUTTONS_Pin */
	GPIO_InitStruct.Pin = INT_ADC_Pin | INT_BUTTONS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : LED1_Pin */
	GPIO_InitStruct.Pin = LED1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LED2_Pin LED3_Pin LED4_Pin */
	GPIO_InitStruct.Pin = LED2_Pin | LED3_Pin | LED4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	switch (GPIO_Pin) {
	case GPIO_PIN_3:
		ADS122C04_ProcessInterrupt(&adc);
		break;
	case GPIO_PIN_4:
		PCAL6524_ProcessInterrupt(&buttons);
		break;
	case GPIO_PIN_5:
		MLX90393_ProcessInterrupt(&hall);
		break;
	}
}

void HAL_NVIC_Enable() {
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void AXIS_Assign(CalibrationStoreValues *_dev, ADS122C04 *_adc,
		ADS122C04 *_cursor, MLX90393 *_hall) {
	_dev->rz = MLX90393_Rotation(_hall);
	_dev->y_axis = _cursor->ain[0];
	_dev->x_axis = _cursor->ain[1];
	_dev->rx = _adc->ain[2];
	_dev->ry = _adc->ain[3];

}

void SYNC() {
	MLX90393_ReadFromInterrupt(&hall);
	ADS122C04_ReadAll(&adc);
	ADS122C04_ReadAll(&cursor);
	AXIS_Assign(&values, &adc, &cursor, &hall);

	PCAL6524_ReadFromInterrupt(&buttons);
}

void JumpToBootloader(void) {
	__enable_irq();
	HAL_RCC_DeInit();
	HAL_DeInit();
	SysTick->CTRL = SysTick->LOAD = SysTick->VAL = 0;
	__HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

	const uint32_t p = (*((uint32_t*) 0x1FFF0000));
	__set_MSP(p);

	void (*SysMemBootJump)(void);
	SysMemBootJump = (void (*)(void)) (*((uint32_t*) 0x1FFF0004));
	SysMemBootJump();

	while (1) {
	}
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
