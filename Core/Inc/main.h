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
#define RESERVED_Pin GPIO_PIN_13
#define RESERVED_GPIO_Port GPIOC
#define EN_5V_Pin GPIO_PIN_1
#define EN_5V_GPIO_Port GPIOC
#define EN_3P3V_Pin GPIO_PIN_2
#define EN_3P3V_GPIO_Port GPIOC
#define ON_N25_Pin GPIO_PIN_3
#define ON_N25_GPIO_Port GPIOC
#define UART4_WU_Pin GPIO_PIN_2
#define UART4_WU_GPIO_Port GPIOA
#define ON_OWEN_Pin GPIO_PIN_3
#define ON_OWEN_GPIO_Port GPIOA
#define NUM_RES_Pin GPIO_PIN_4
#define NUM_RES_GPIO_Port GPIOA
#define STR_B1_Pin GPIO_PIN_5
#define STR_B1_GPIO_Port GPIOA
#define STR_B1_EXTI_IRQn EXTI9_5_IRQn
#define STR_B2_Pin GPIO_PIN_6
#define STR_B2_GPIO_Port GPIOA
#define STR_B2_EXTI_IRQn EXTI9_5_IRQn
#define STR_B3_Pin GPIO_PIN_7
#define STR_B3_GPIO_Port GPIOA
#define STR_B3_EXTI_IRQn EXTI9_5_IRQn
#define STR_B4_Pin GPIO_PIN_4
#define STR_B4_GPIO_Port GPIOC
#define STR_B4_EXTI_IRQn EXTI4_IRQn
#define COL_B4_Pin GPIO_PIN_5
#define COL_B4_GPIO_Port GPIOC
#define COL_B3_Pin GPIO_PIN_0
#define COL_B3_GPIO_Port GPIOB
#define COL_B2_Pin GPIO_PIN_1
#define COL_B2_GPIO_Port GPIOB
#define COL_B1_Pin GPIO_PIN_2
#define COL_B1_GPIO_Port GPIOB
#define ON_DISP_Pin GPIO_PIN_12
#define ON_DISP_GPIO_Port GPIOB
#define SPI2_CS_ADC_Pin GPIO_PIN_6
#define SPI2_CS_ADC_GPIO_Port GPIOC
#define One_Wire_Pin GPIO_PIN_7
#define One_Wire_GPIO_Port GPIOC
#define USART1_DATA_DETECT_Pin GPIO_PIN_8
#define USART1_DATA_DETECT_GPIO_Port GPIOA
#define SDMMC1_DET_Pin GPIO_PIN_15
#define SDMMC1_DET_GPIO_Port GPIOA
#define ON_RS_Pin GPIO_PIN_3
#define ON_RS_GPIO_Port GPIOB
#define SPI2_CS_ROM_Pin GPIO_PIN_6
#define SPI2_CS_ROM_GPIO_Port GPIOB
#define ON_ROM_Pin GPIO_PIN_7
#define ON_ROM_GPIO_Port GPIOB


/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
