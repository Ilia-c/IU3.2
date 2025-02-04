/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "cmsis_os.h"
#include "fatfs.h"
#include "RTC_data.h"
#include "OLED.h"
#include "Display.h"
#include "OLED_Fonts.h"
#include "OLED_Icons.h"
#include "Keyboard.h"
#include "Settings.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "w25q128.h"
#include "MS5193T.h"
#include "SD.h"
#include "usb_host.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include <stdio.h>
#include "USB_COMPORT.h"
#include "GSM.h"
#include "Sleep.h"
#include "AT24C02.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

extern char Keyboard_press_code;
extern uint16_t time_update_display;

xSemaphoreHandle Keyboard_semapfore;
xSemaphoreHandle Display_semaphore;
xSemaphoreHandle Display_cursor_semaphore;
xSemaphoreHandle USB_COM_semaphore;
xSemaphoreHandle Main_semaphore;

extern const uint16_t Timer_key_one_press;
extern const uint16_t Timer_key_press;
extern const uint16_t Timer_key_press_fast;

extern uint8_t gsmRxChar;
extern EEPROM_Settings_item EEPROM;


ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc3;
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
SD_HandleTypeDef hsd1;
DMA_HandleTypeDef hdma_sdmmc1_rx;
DMA_HandleTypeDef hdma_sdmmc1_tx;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart4;

/* Definitions for SD_card */
osThreadId_t SD_cardHandle;
const osThreadAttr_t SD_card_attributes = {
    .name = "SD_card",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for Display_I2C */
osThreadId_t Display_I2CHandle;
const osThreadAttr_t Display_I2C_attributes = {
    .name = "Display_I2C",
    .stack_size = 1024 * 1,
    .priority = (osPriority_t)osPriorityLow2,
};
/* Definitions for ADC_read */
osThreadId_t ADC_readHandle;
const osThreadAttr_t ADC_read_attributes = {
    .name = "ADC_read",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityLow3,
};
/* Definitions for RS485_data */
osThreadId_t RS485_dataHandle;
const osThreadAttr_t RS485_data_attributes = {
    .name = "RS485_data",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityLow3,
};
/* Definitions for SIM800_data */
osThreadId_t SIM800_dataHandle;
const osThreadAttr_t SIM800_data_attributes = {
    .name = "SIM800_data",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for SIM800_data */
osThreadId_t Monitor_taskHandle;
const osThreadAttr_t Monitor_task_attributes = {
    .name = "Monitor_task",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for Main */
osThreadId_t MainHandle;
const osThreadAttr_t Main_attributes = {
    .name = "Main",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for Keyboard_task */
osThreadId_t Keyboard_taskHandle;
const osThreadAttr_t Keyboard_task_attributes = {
    .name = "Keyboard_task",
    .stack_size = 128 * 2,
    .priority = (osPriority_t)osPriorityLow4,
};

osThreadId_t USB_COM_taskHandle;
const osThreadAttr_t USB_COM_task_attributes = {
    .name = "USB_COM_task",
    .stack_size = 1024 * 8,
    .priority = (osPriority_t)osPriorityLow2,
};


osThreadId_t Main_Cycle_taskHandle;
const osThreadAttr_t Main_Cycle_task_attributes = {
    .name = "Main_Cycle_task",
    .stack_size = 128 * 1,
    .priority = (osPriority_t)osPriorityLow5,
};

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC3_Init(void);
static void MX_I2C1_Init(void);        //
static void MX_I2C2_Init(void);        //
static void MX_SDMMC1_SD_Init(void);   //
static void MX_SPI2_Init(void);        //
//static void MX_UART1_Init(void);     //
static void MX_UART4_Init(void);       //
static void MX_TIM5_Init(void);
static void MX_TIM6_Init(void);

void SD_card(void *argument);
void Display_I2C(void *argument);
void ADC_read(void *argument);
void RS485_data(void *argument);
void SIM800_data(void *argument);
void Monitor_task(void *argument);
void Main(void *argument);
void Main_Cycle(void *argument); // основной режим в циклическом режиме
void Keyboard_task(void *argument);
void USB_COM_task(void *argument);
void HAL_TIM6_Callback(void);
void SetTimerPeriod(uint32_t period_ms);


unsigned int id = 0x00;
extern RTC_HandleTypeDef hrtc;

int main(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess(); 

  HAL_Init();
  SystemClock_Config();
  PeriphCommonClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();
  //MX_ADC3_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_SPI2_Init();
  MX_TIM5_Init();
  MX_TIM6_Init();
  RTC_Init();

  // Начальные состояния переферии - ВСЕ ОТКЛЮЧЕНО
  HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, 1);
  HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ADC_Pin, 1);
  HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 0);
  HAL_GPIO_WritePin(ON_RS_GPIO_Port, ON_RS_Pin, 0);
  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, 0);
  HAL_GPIO_WritePin(EN_3P3V_GPIO_Port, EN_3P3V_Pin, 0);
  HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
  HAL_GPIO_WritePin(ON_DISP_GPIO_Port, ON_DISP_Pin, 0);
  HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 0);
  HAL_Delay(10);


  // ЗАПУСКАЕТСЯ ВСЕГДА 
  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, 1);             // Общее питание 5В
  HAL_GPIO_WritePin(EN_3P3V_GPIO_Port, EN_3P3V_Pin, 1);         // Общее питание 3.3В
  HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 1);           // Включение Памяти на плате

  
  HAL_Delay(10);
  // Чтение данных из EEPROM
  if (!(EEPROM_IsDataExists()))
  {
    // Данных нету - первый запуск 
    if (!EEPROM_SaveSettings(&EEPROM))
    {
      // Сохранение не вышло - нет связи с EEPROM
    }
  }
  else
  {
    if (!EEPROM_LoadSettings(&EEPROM))
    {
      // Ошибка - неверный идентификатор данных
      if (!EEPROM_SaveSettings(&EEPROM))
      {
        // Сохранение не вышло - нет связи с EEPROM
      }
    }
  }
  
  if (EEPROM.Communication == 1) HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 1);
  

  osKernelInitialize();
  if (EEPROM.Mode == 1) Main_Cycle_taskHandle = osThreadNew(Main_Cycle, NULL, &Main_Cycle_task_attributes); // Задача для циклического режима
  if (EEPROM.Mode == 0) MainHandle = osThreadNew(Main, NULL, &Main_attributes); // Задача для циклического режима

  SD_cardHandle = osThreadNew(SD_card, NULL, &SD_card_attributes);
  Display_I2CHandle = osThreadNew(Display_I2C, NULL, &Display_I2C_attributes);
  ADC_readHandle = osThreadNew(ADC_read, NULL, &ADC_read_attributes);
  RS485_dataHandle = osThreadNew(RS485_data, NULL, &RS485_data_attributes);
  SIM800_dataHandle = osThreadNew(SIM800_data, NULL, &SIM800_data_attributes);
  Monitor_taskHandle = osThreadNew(Monitor_task, NULL, &Monitor_task_attributes);
  Keyboard_taskHandle = osThreadNew(Keyboard_task, NULL, &Keyboard_task_attributes);
  USB_COM_taskHandle = osThreadNew(USB_COM_task, NULL, &USB_COM_task_attributes);
  osKernelStart();


  while (1)
  {
  }

}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if ((GPIO_Pin == STR_B1_Pin) || (GPIO_Pin == STR_B2_Pin) || (GPIO_Pin == STR_B3_Pin) || (GPIO_Pin == STR_B4_Pin))
  {
    uint8_t B1 = HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin);
    uint8_t B2 = HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin);
    uint8_t B3 = HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B3_Pin);
    uint8_t B4 = HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin);
    osDelay(2);
    if ((HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == B1) && (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == B2) && (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == B3) && (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == B4))
    {
      static portBASE_TYPE xTaskWoken;
      xSemaphoreGiveFromISR(Keyboard_semapfore, &xTaskWoken);
    }
  }
}

void SetTimerPeriod(uint32_t period_ms)
{
    // Остановим таймер, чтобы избежать конфликтов
    if (htim5.Init.Period == period_ms-1) return;
    HAL_TIM_Base_Stop_IT(&htim5);
    // Расчёт предделителя и периода
    // Частота таймера (TIM5) = 40 МГц (согласно конфигурации SystemClock)
    // Предположим делитель для удобного расчёта: 40 000 (40 МГц / 40 000 = 1 кГц)
    uint32_t prescaler = 39999;  // Делитель: делит тактовую частоту до 1 кГц
    uint32_t timer_clock = 1000; // Частота после делителя (1 кГц)

    // Период (ARR) = (требуемый период * частота таймера) - 1
    uint32_t auto_reload = period_ms - 1;

    // Ограничение значений ARR (16-бит или 32-бит таймер)
    if (auto_reload > 0xFFFFFFFF) // TIM5 - 32-битный, но ограничение для других таймеров
    {
        auto_reload = 0xFFFFFFFF;
    }

    // Настраиваем таймер
    htim5.Init.Prescaler = prescaler;
    htim5.Init.Period = auto_reload; // Период соответствует времени в мс
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    // Применяем настройки
    if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
    {
        Error_Handler();
    }

    // Сброс счётчика
    __HAL_TIM_SET_COUNTER(&htim5, 0);

    // Запускаем таймер с прерываниями
    HAL_TIM_Base_Start_IT(&htim5);
}


void HAL_TIM5_Callback(void)
{
  static portBASE_TYPE xTaskWoken;
  xSemaphoreGiveFromISR(Main_semaphore, &xTaskWoken); 
}

void HAL_TIM6_Callback(void)
{
  //HAL_TIM_Base_Stop_IT(&htim6);
  //__HAL_TIM_SET_AUTORELOAD(&htim6, Timer_key_press-1);
  //TIM6->CNT = 0;
  __HAL_TIM_SET_AUTORELOAD(&htim6, Timer_key_press_fast);
  static portBASE_TYPE xTaskWoken;
  xSemaphoreGiveFromISR(Keyboard_semapfore, &xTaskWoken); 
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_SDMMC1
                              |RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLLSAI1;
  PeriphClkInit.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSE;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 12;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK|RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}





/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV6;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

static void MX_TIM5_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim5.Instance = TIM5;
    htim5.Init.Prescaler = 40000-1;                       // Предделитель (40 МГц / 40 000 = 1 кГц)
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 20000-1;                          // Период (1 кГц / 20 000 = 0.05 Гц или 20 сек)
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}


static void MX_TIM6_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 40000 - 1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = Timer_key_press - 1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_ADC3_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc3.Instance                      = ADC3;
  hadc3.Init.ClockPrescaler          = ADC_CLOCK_ASYNC_DIV1;     // Делитель тактирования
  hadc3.Init.Resolution              = ADC_RESOLUTION_12B;       // 12 бит
  hadc3.Init.DataAlign               = ADC_DATAALIGN_RIGHT;
  hadc3.Init.ScanConvMode            = ADC_SCAN_DISABLE;         // Один канал
  hadc3.Init.EOCSelection            = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait        = DISABLE;
  hadc3.Init.ContinuousConvMode      = DISABLE;                  // Одиночное преобразование
  hadc3.Init.NbrOfConversion         = 1;
  hadc3.Init.DiscontinuousConvMode   = DISABLE;
  hadc3.Init.ExternalTrigConv        = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge    = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.DMAContinuousRequests   = DISABLE;
  hadc3.Init.Overrun                 = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.OversamplingMode        = DISABLE;

  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  // Настраиваем регулярный канал — PC0 = Channel 1 (ADC123_IN1)
  sConfig.Channel      = ADC_CHANNEL_1;
  sConfig.Rank         = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_24CYCLES_5; // При необходимости выбрать больше (например, 47.5 или 92.5)
  sConfig.SingleDiff   = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset       = 0;

  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00F12981;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
 * @brief I2C2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00F12981;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }

}


__attribute__((unused)) static void MX_SDMMC1_SD_Init(void)
{
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 16;
}


/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH; // ???? LOW
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;  // ???? SPI_PHASE_1EDGE
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}




static void MX_UART4_Init(void)
{
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 57600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_DMADISABLEONERROR_INIT;
  huart4.AdvancedInit.DMADisableonRxError = UART_ADVFEATURE_DMA_DISABLEONRXERROR;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * Enable DMA controller clock
  */
__attribute__((unused)) static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel4_IRQn);
  /* DMA2_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel5_IRQn);

}
/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RESERVED_Pin|EN_5V_Pin|EN_3P8V_Pin|ON_N25_Pin
                          |COL_B4_Pin|SPI2_CS_ADC_Pin|One_Wire_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, UART4_WU_Pin|ON_OWEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, COL_B3_Pin|COL_B2_Pin|COL_B1_Pin|ON_DISP_Pin
                          |ON_RS_Pin|GPIO_PIN_4|SPI2_CS_ROM_Pin|ON_ROM_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RESERVED_Pin EN_5V_Pin EN_3P3V_Pin ON_N25_Pin
                           COL_B4_Pin SPI2_CS_ADC_Pin One_Wire_Pin */
  GPIO_InitStruct.Pin = RESERVED_Pin|EN_5V_Pin|EN_3P8V_Pin|ON_N25_Pin
                          |COL_B4_Pin|SPI2_CS_ADC_Pin|One_Wire_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;  // Отключение подтяжек
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  /*Configure GPIO pins : UART4_WU_Pin ON_OWEN_Pin */
  GPIO_InitStruct.Pin = UART4_WU_Pin|ON_OWEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : NUM_RES_Pin USART1_DATA_DETECT_Pin */
  GPIO_InitStruct.Pin = NUM_RES_Pin|USART1_DATA_DETECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : STR_B1_Pin STR_B2_Pin STR_B3_Pin */
  GPIO_InitStruct.Pin = STR_B1_Pin|STR_B2_Pin|STR_B3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : STR_B4_Pin */
  GPIO_InitStruct.Pin = STR_B4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(STR_B4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : COL_B3_Pin COL_B2_Pin COL_B1_Pin ON_DISP_Pin
                           ON_RS_Pin PB4 ON_ROM_Pin */
  GPIO_InitStruct.Pin = COL_B3_Pin|COL_B2_Pin|COL_B1_Pin|ON_DISP_Pin
                          |ON_RS_Pin|GPIO_PIN_4|ON_ROM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SDMMC1_DET_Pin */
  GPIO_InitStruct.Pin = SDMMC1_DET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SDMMC1_DET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI2_CS_ROM_Pin */
  GPIO_InitStruct.Pin = SPI2_CS_ROM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SPI2_CS_ROM_GPIO_Port, &GPIO_InitStruct);

  /**/
  __HAL_SYSCFG_FASTMODEPLUS_ENABLE(SYSCFG_FASTMODEPLUS_PB6);
}



int32_t ADC1_Read_PC0(void) { 
    // --- Калибровка АЦП ---
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK) {
        Error_Handler();
    }

    // --- Чтение VREFINT ---
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_VREFINT;  // VREFINT
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
        Error_Handler();
    }
    uint32_t vrefint_raw = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    // --- Чтение PC0 ---
    sConfig.Channel = ADC_CHANNEL_1;  // PC0
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
        Error_Handler();
    }
    uint32_t measurement_raw = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return measurement_raw;  // Возвращаем скорректированное значение АЦП
}
uint32_t data_read_adc_in = 0;

/* USER CODE BEGIN Header_Display_I2C */
/**
 * @brief Function implementing the Display_I2C thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Display_I2C */
void Display_I2C(void *argument)
{
  UNUSED(argument);
  /* USER CODE BEGIN Display_I2C */
  /* Infinite loop */
  for (;;)
  {
    xSemaphoreTake(Display_semaphore, portMAX_DELAY);
    Keyboard_processing();
    Display_all_menu();
  }
  /* USER CODE END Display_I2C */
}

/* USER CODE BEGIN Header_ADC_read */
/**
 * @brief Function implementing the ADC_read thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_ADC_read */
void ADC_read(void *argument)
{
  UNUSED(argument);
  /* USER CODE BEGIN ADC_read */
  /* Infinite loop */
  for (;;)
  {
    Read_MS5193T_Data();
    osDelay(100);
    //xSemaphoreGive(Display_semaphore);
  }
  /* USER CODE END ADC_read */
}

/* USER CODE BEGIN Header_RS485_data */
/**
 * @brief Function implementing the RS485_data thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RS485_data */
void RS485_data(void *argument)
{
  UNUSED(argument);
  /* USER CODE BEGIN RS485_data */
  /* Infinite loop */
  for (;;)
  {
    osDelay(1000);
  }
  /* USER CODE END RS485_data */
}

/* USER CODE BEGIN Header_SIM800_data */
/**
 * @brief Function implementing the SIM800_data thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_SIM800_data */
void SIM800_data(void *argument)
{
  UNUSED(argument);
  HAL_Delay(100);
  HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 1);
  HAL_Delay(600);
  HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 0);
  /* USER CODE BEGIN SIM800_data */
  /* Infinite loop */
  for (;;)
  {
    osDelay(1000);
  }
  /* USER CODE END SIM800_data */
}

void Main(void *argument)
{
  UNUSED(argument);

  HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 0);
  HAL_GPIO_WritePin(ON_RS_GPIO_Port, ON_RS_Pin, 0);
  HAL_GPIO_WritePin(ON_DISP_GPIO_Port, ON_DISP_Pin, 1);
  HAL_Delay(10);
  OLED_Init(&hi2c2);
  HAL_Delay(20);

  EnableUsbCDC_UART(*None_func);
  MX_UART4_Init();
  HAL_NVIC_SetPriority(UART4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(UART4_IRQn);
  W25_Ini();
  id = W25_Read_ID();
  MS5193T_Init();
  // WriteToSDCard();

  RTC_read();
  
  //MX_USB_HOST_Init();
  MX_USB_DEVICE_Init();
  HAL_NVIC_SetPriority(OTG_FS_IRQn, 5, 0); // Приоритет прерывания
  HAL_NVIC_EnableIRQ(OTG_FS_IRQn);         // Включение прерывания
  HAL_UART_Receive_IT(&huart4, &gsmRxChar, 1);

  if (EEPROM.screen_sever_mode == 1) Start_video();

  HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, 1);
  HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, 1);
  HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, 1);
  HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, 1);
  HAL_GPIO_WritePin(ON_N25_GPIO_Port, ON_N25_Pin, 1);
  HAL_NVIC_SetPriority(EXTI4_IRQn, 7, 0);
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);


  // Настройка таймера клавиатуры
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 8, 0); // Установите приоритет
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);        // Включите прерывание


  /*
  Read_MS5193T_Data();
  HAL_Delay(200);
  Read_MS5193T_Data();
  HAL_Delay(200);
  Read_MS5193T_Data();
  HAL_Delay(200);
  Read_MS5193T_Data();
  HAL_Delay(200);

  
  data_read_adc_in = ADC1_Read_PC0();
  MX_DMA_Init();
  MX_SDMMC1_SD_Init();
  HAL_StatusTypeDef res = HAL_SD_Init(&hsd1);
  HAL_SD_CardInfoTypeDef CardInfo;
  FRESULT res_2 = HAL_SD_GetCardInfo(&hsd1, &CardInfo);
  HAL_SD_CardStateTypeDef res_1 = HAL_SD_GetCardState(&hsd1);
  res = HAL_SD_ConfigWideBusOperation(&hsd1, SDMMC_BUS_WIDE_4B);
  res_1 = HAL_SD_GetCardState(&hsd1);
  HAL_Delay(200);
  MX_FATFS_Init();
  WriteToSDCard();
  
  
  if (Check_Wakeup_Reason() == 1) {
    Enter_StandbyMode(0, 30);// Если не аппаратный сброс. не работает, нужно переписать
  } 
  */
 
  vSemaphoreCreateBinary(Keyboard_semapfore);
  vSemaphoreCreateBinary(Display_semaphore);
  vSemaphoreCreateBinary(Display_cursor_semaphore);
  vSemaphoreCreateBinary(USB_COM_semaphore);
  vSemaphoreCreateBinary(Main_semaphore);

    // Запуск глобального таймера для обновления экрана
  HAL_NVIC_SetPriority(TIM5_IRQn, 8, 0); // Установите приоритет
  HAL_NVIC_EnableIRQ(TIM5_IRQn);        // Включите прерывание
  HAL_TIM_Base_Start_IT(&htim5);

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1); // ВКЛЮЧИТЬ СИГНАЛЬНЫЙ СВЕТОДИОД

  for (;;)
  {
    RTC_read();
    xSemaphoreGive(Display_semaphore);
    xSemaphoreTake(Main_semaphore, portMAX_DELAY);
  }
}

void Monitor_task(void *argument)
{
  UNUSED(argument);
  for (;;)
  {
    osDelay(60000);
  }
}

void Keyboard_task(void *argument)
{
  UNUSED(argument);
  for (;;)
  {
    xSemaphoreTake(Keyboard_semapfore, portMAX_DELAY);
    ScanKeypad();
    //osDelay(Timer_key_one_press);
  }
}

void SD_card(void *argument)
{
    UNUSED(argument);
    for (;;)
    {
      osDelay(60000);
    }
}

void Main_Cycle(void *argument)
{
    UNUSED(argument);
    for (;;)
    {
      osDelay(10000);
    }
}
void USB_COM_task(void *argument)
{
    UNUSED(argument);
    for (;;)
    {
        // Ожидаем семафор (данные готовы к обработке)
        xSemaphoreTake(USB_COM_semaphore, portMAX_DELAY);
        USB_COM();
    }
}

int a = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    HAL_IncTick();
  }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
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

