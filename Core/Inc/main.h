/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DC_display_Pin GPIO_PIN_0
#define DC_display_GPIO_Port GPIOC
#define RES_display_Pin GPIO_PIN_1
#define RES_display_GPIO_Port GPIOC
#define Display_ON_OFF_Pin GPIO_PIN_0
#define Display_ON_OFF_GPIO_Port GPIOA
#define SPI_2_SS_Pin GPIO_PIN_1
#define SPI_2_SS_GPIO_Port GPIOA
#define DCDC__ON_OFF_Pin GPIO_PIN_4
#define DCDC__ON_OFF_GPIO_Port GPIOA
#define ADC_ON_OFF_Pin GPIO_PIN_5
#define ADC_ON_OFF_GPIO_Port GPIOC
#define Data_FES_ON_OFF_Pin GPIO_PIN_0
#define Data_FES_ON_OFF_GPIO_Port GPIOB
#define RS485__ON_OFF_Pin GPIO_PIN_1
#define RS485__ON_OFF_GPIO_Port GPIOB
#define Reset_key_Pin GPIO_PIN_2
#define Reset_key_GPIO_Port GPIOB
#define Str_b1_Pin GPIO_PIN_12
#define Str_b1_GPIO_Port GPIOB
#define Col_b4_Pin GPIO_PIN_14
#define Col_b4_GPIO_Port GPIOB
#define Col_b3_Pin GPIO_PIN_15
#define Col_b3_GPIO_Port GPIOB
#define Col_b2_Pin GPIO_PIN_6
#define Col_b2_GPIO_Port GPIOC
#define Col_b1_Pin GPIO_PIN_7
#define Col_b1_GPIO_Port GPIOC
#define Str_b2_Pin GPIO_PIN_8
#define Str_b2_GPIO_Port GPIOA
#define Str_b3_Pin GPIO_PIN_9
#define Str_b3_GPIO_Port GPIOA
#define Str_b4_Pin GPIO_PIN_10
#define Str_b4_GPIO_Port GPIOA
#define GSM_ON_OFF_Pin GPIO_PIN_15
#define GSM_ON_OFF_GPIO_Port GPIOA
#define Temp_ON_OFF_Pin GPIO_PIN_3
#define Temp_ON_OFF_GPIO_Port GPIOB
#define SD_detect_Pin GPIO_PIN_4
#define SD_detect_GPIO_Port GPIOB
#define RS_485_Data_det_Pin GPIO_PIN_5
#define RS_485_Data_det_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
