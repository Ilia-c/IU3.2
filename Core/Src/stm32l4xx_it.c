/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32l4xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_it.h"
#include "Settings.h"
#include "Status_codes.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern HCD_HandleTypeDef hhcd_USB_OTG_FS;
extern SD_HandleTypeDef hsd1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart4;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  // ! Добавить сохранения кода ошибки и стека вызывов
  ERRCODE.STATUS |= STATUS_NMI_OCCURRED;
  //HAL_PWR_EnableBkUpAccess();
  //HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_RESET_PROG, DATA_RESET_PROG);
  //NVIC_SystemReset();
  while (1){}
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  // ! Добавить сохранения кода ошибки и стека вызывов
  ERRCODE.STATUS |= STATUS_HARDFAULT_OCCURRED;
  //HAL_PWR_EnableBkUpAccess();
  //HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_RESET_PROG, DATA_RESET_PROG);
  //NVIC_SystemReset();
  while (1){}
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  // ! Добавить сохранения кода ошибки и стека вызывов
  ERRCODE.STATUS |= STATUS_MEMMANAGE_FAULT;
  //HAL_PWR_EnableBkUpAccess();
  //HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_RESET_PROG, DATA_RESET_PROG);
  //NVIC_SystemReset();
  while (1){}
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  // ! Добавить сохранения кода ошибки и стека вызывов
  ERRCODE.STATUS |= STATUS_BUSFAULT_OCCURRED;
  //HAL_PWR_EnableBkUpAccess();
  //HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_RESET_PROG, DATA_RESET_PROG);
  //NVIC_SystemReset();
  while (1){}
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  // ! Добавить сохранения кода ошибки и стека вызывов
  ERRCODE.STATUS |= STATUS_USAGEFAULT_OCCURRED;
  //HAL_PWR_EnableBkUpAccess();
  //HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_RESET_PROG, DATA_RESET_PROG);
  //NVIC_SystemReset();
  while (1){}
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles RCC global interrupt.
  */
void RCC_IRQHandler(void)
{
  /* USER CODE BEGIN RCC_IRQn 0 */

  /* USER CODE END RCC_IRQn 0 */
  /* USER CODE BEGIN RCC_IRQn 1 */

  /* USER CODE END RCC_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */


void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

void TIM5_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim5);
  HAL_TIM5_Callback();
}

void TIM6_DAC_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim6);
  HAL_TIM6_Callback();
}

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line3 interrupt.
  */


/**
  * @brief This function handles EXTI line4 interrupt.
  */
void EXTI4_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(STR_B4_Pin);
}
void EXTI9_5_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(STR_B1_Pin);
  HAL_GPIO_EXTI_IRQHandler(STR_B2_Pin);
  HAL_GPIO_EXTI_IRQHandler(STR_B3_Pin);
}
/**
  * @brief This function handles USB OTG FS global interrupt.
  */



void SDMMC1_IRQHandler(void)
{
  /* USER CODE BEGIN SDMMC1_IRQn 0 */

  /* USER CODE END SDMMC1_IRQn 0 */
  HAL_SD_IRQHandler(&hsd1);
  /* USER CODE BEGIN SDMMC1_IRQn 1 */

  /* USER CODE END SDMMC1_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel4 global interrupt.
  */
 extern DMA_HandleTypeDef hdma_sdmmc1;
 void DMA2_Channel4_IRQHandler(void)
 {
   /* USER CODE BEGIN DMA2_Channel4_IRQn 0 */
     if((hsd1.Context == (SD_CONTEXT_DMA | SD_CONTEXT_READ_SINGLE_BLOCK)) ||
        (hsd1.Context == (SD_CONTEXT_DMA | SD_CONTEXT_READ_MULTIPLE_BLOCK)))
     {
       BSP_SD_DMA_Rx_IRQHandler();
     }
     else if((hsd1.Context == (SD_CONTEXT_DMA | SD_CONTEXT_WRITE_SINGLE_BLOCK)) ||
             (hsd1.Context == (SD_CONTEXT_DMA | SD_CONTEXT_WRITE_MULTIPLE_BLOCK)))
     {
       BSP_SD_DMA_Tx_IRQHandler();
     }
   /* USER CODE END DMA2_Channel4_IRQn 0 */
   HAL_DMA_IRQHandler(&hdma_sdmmc1);
   /* USER CODE BEGIN DMA2_Channel4_IRQn 1 */
   /* USER CODE END DMA2_Channel4_IRQn 1 */
 }
 

void UART4_IRQHandler(void)
{
  /* USER CODE BEGIN UART4_IRQn 0 */

  /* USER CODE END UART4_IRQn 0 */
  HAL_UART_IRQHandler(&huart4);
  /* USER CODE BEGIN UART4_IRQn 1 */

  /* USER CODE END UART4_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel5 global interrupt.
  */

extern EEPROM_Settings_item EEPROM;
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */

  /* USER CODE END OTG_FS_IRQn 0 */
  
  if (EEPROM.USB_mode == 1){
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
  }
  if (EEPROM.USB_mode == 2){
    HAL_HCD_IRQHandler(&hhcd_USB_OTG_FS);
  }
 
  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // Здесь можно поставить breakpoint или выводить сообщение
    printf("Stack overflow in task: %s\r\n", pcTaskName);
    // Остановить выполнение, чтобы посмотреть регистры
    taskDISABLE_INTERRUPTS();
    for(;;);
}

