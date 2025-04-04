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
#include "stm32l4xx_it.h"
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
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim8;
extern UART_HandleTypeDef huart4;

extern xSemaphoreHandle SLEEP_semaphore;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */

extern RTC_HandleTypeDef hrtc;
 void NMI_Handler(void)
 {
   // Устанавливаем флаг в ERRCODE (не сохранится после reset, если не в backup SRAM)
   ERRCODE.STATUS |= STATUS_NMI_OCCURRED;  
   uint32_t errcode_low  = (uint32_t)(ERRCODE.STATUS & 0xFFFFFFFF);
   uint32_t errcode_high = (uint32_t)(ERRCODE.STATUS >> 32);
   HAL_PWR_EnableBkUpAccess();
   HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_1, errcode_low);
   HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_2, errcode_high);
   NVIC_SystemReset();
 
   // Если по каким-то причинам сброс не произошёл, зацикливаемся
   while (1) {}
 }
 
 void HardFault_Handler(void)
 {
   ERRCODE.STATUS |= STATUS_HARDFAULT_OCCURRED;
   uint32_t errcode_low  = (uint32_t)(ERRCODE.STATUS & 0xFFFFFFFF);
   uint32_t errcode_high = (uint32_t)(ERRCODE.STATUS >> 32);
   HAL_PWR_EnableBkUpAccess();
   HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_1, errcode_low);
   HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_2, errcode_high);
   NVIC_SystemReset();
   while (1) {}
 }
 
 void MemManage_Handler(void)
 {
   ERRCODE.STATUS |= STATUS_MEMMANAGE_FAULT;
   uint32_t errcode_low  = (uint32_t)(ERRCODE.STATUS & 0xFFFFFFFF);
   uint32_t errcode_high = (uint32_t)(ERRCODE.STATUS >> 32);
   HAL_PWR_EnableBkUpAccess();
   HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_1, errcode_low);
   HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_2, errcode_high);
   NVIC_SystemReset();
   while (1) {}
 }
 
 void BusFault_Handler(void)
 {
   ERRCODE.STATUS |= STATUS_BUSFAULT_OCCURRED;
   uint32_t errcode_low  = (uint32_t)(ERRCODE.STATUS & 0xFFFFFFFF);
   uint32_t errcode_high = (uint32_t)(ERRCODE.STATUS >> 32);
   HAL_PWR_EnableBkUpAccess();
   HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_1, errcode_low);
   HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_2, errcode_high);
   NVIC_SystemReset();
   while (1) {}
 }
 
 void UsageFault_Handler(void)
 {
   ERRCODE.STATUS |= STATUS_USAGEFAULT_OCCURRED;
   uint32_t errcode_low  = (uint32_t)(ERRCODE.STATUS & 0xFFFFFFFF);
   uint32_t errcode_high = (uint32_t)(ERRCODE.STATUS >> 32);
   HAL_PWR_EnableBkUpAccess();
   HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_1, errcode_low);
   HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_2, errcode_high);
   NVIC_SystemReset();
   while (1) {}
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

static uint32_t time_counter;
void TIM8_UP_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim8);
  time_counter++;
  if (time_counter>=300){
    // Еасли прошло 5 минут - вызываем либо заставку, либо засыпаем (зависит от режима block)
    time_counter=0;
    static portBASE_TYPE xTaskWoken;
    xSemaphoreGiveFromISR(SLEEP_semaphore, &xTaskWoken);
  }
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
static uint8_t last_USB_state = 255;
void OTG_FS_IRQHandler(void)
{
  // ! При несовпадении - полная деинициализация USB
  if (last_USB_state == 255) last_USB_state = EEPROM.USB_mode;
  if (last_USB_state == 1 || last_USB_state == 2){
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
  }
  if (last_USB_state == 0){
    HAL_HCD_IRQHandler(&hhcd_USB_OTG_FS);
  }
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // Событие переполнения стека
    taskDISABLE_INTERRUPTS(); 
    ERRCODE.STATUS |= STATUS_USAGEFAULT_OCCURRED;
    uint32_t errcode_low  = (uint32_t)(ERRCODE.STATUS & 0xFFFFFFFF);
    uint32_t errcode_high = (uint32_t)(ERRCODE.STATUS >> 32);
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_1, errcode_low);
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_2, errcode_high);
    NVIC_SystemReset();
    for(;;);
}

