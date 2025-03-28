/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * 
 * 
 *
 * VERSION Ver0.41
 * 
 * 
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
#include "Parser.h"
#include "Diagnostics.h"
#include "USB_FATFS_SAVE.h"
#include "ds18b20.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

extern char Keyboard_press_code;
extern uint16_t time_update_display;

xSemaphoreHandle Keyboard_semapfore;
xSemaphoreHandle Display_semaphore;
xSemaphoreHandle Display_cursor_semaphore;
xSemaphoreHandle USB_COM_semaphore;
xSemaphoreHandle Main_semaphore;
xSemaphoreHandle UART_PARSER_semaphore;
xSemaphoreHandle ADC_READY; // Окончания преобразования при работе в циклическом режиме
xSemaphoreHandle SD_WRITE; // Окончания преобразования при работе в циклическом режиме

extern const uint16_t Timer_key_one_press;
extern const uint16_t Timer_key_press;
extern const uint16_t Timer_key_press_fast;

extern uint8_t gsmRxChar;
extern EEPROM_Settings_item EEPROM;
extern ERRCODE_item ERRCODE;


ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc3;
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
SD_HandleTypeDef hsd1;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart4;
DMA_HandleTypeDef hdma_sdmmc1;

IWDG_HandleTypeDef hiwdg;

/* Definitions for Main */
osThreadId_t MainHandle;
const osThreadAttr_t Main_attributes = {
    .name = "Main",
    .stack_size = 256 * 1,
    .priority = (osPriority_t)osPriorityHigh5,
};
osThreadId_t Main_Cycle_taskHandle;
const osThreadAttr_t Main_Cycle_task_attributes = {
    .name = "Main_Cycle_task",
    .stack_size = 1024 * 3,
    .priority = (osPriority_t)osPriorityHigh5,
};

/* Definitions for Display_I2C */
osThreadId_t Display_I2CHandle;
const osThreadAttr_t Display_I2C_attributes = {
    .name = "Display_I2C",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)osPriorityHigh,
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
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityLow3,
};

/* Definitions for Keyboard_task */
osThreadId_t Keyboard_taskHandle;
const osThreadAttr_t Keyboard_task_attributes = {
    .name = "Keyboard_task",
    .stack_size = 128 * 2,
    .priority = (osPriority_t)osPriorityHigh1,
};

osThreadId_t USB_COM_taskHandle;
const osThreadAttr_t USB_COM_task_attributes = {
    .name = "USB_COM_task",
    .stack_size = 512 * 2,
    .priority = (osPriority_t)osPriorityLow2,
};

osThreadId_t  ERROR_INDICATE_taskHandle;
const osThreadAttr_t Erroe_indicate_task_attributes = {
    .name = "Erroe_indicate_task",
    .stack_size = 1024 * 1,
    .priority = (osPriority_t)osPriorityLow1,
};

osThreadId_t  UART_PARSER_taskHandle;
const osThreadAttr_t UART_PARSER_task_attributes = {
    .name = "UART_PARSER_task",
    .stack_size = 1024 * 3,
    .priority = (osPriority_t)osPriorityHigh4,
};

osThreadId_t SD_taskHandle;
const osThreadAttr_t SD_task_attributes = {
    .name = "SD_task",
    .stack_size = 1024*2,
    .priority = (osPriority_t)osPriorityLow,
};
osThreadId_t WATCDOG_taskHandle;
const osThreadAttr_t WATCDOG_task_attributes = {
    .name = "Watch_dog_task",
    .stack_size = 256,
    .priority = (osPriority_t)osPriorityHigh3,
};

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);       
static void MX_DMA_Init(void);        // ДЛЯ SD
static void MX_ADC1_Init(void);       // 
static void MX_ADC3_Init(void);        // 
static void MX_I2C1_Init(void);        // 
static void MX_I2C2_Init(void);        // 
void MX_SDMMC1_SD_Init(void);   //  SD
static void MX_SPI2_Init(void);        // АЦП+FLASH
//static void MX_UART1_Init(void);     // RS-485
static void MX_UART4_Init(void);       // GSM
static void MX_TIM5_Init(void);
static void MX_TIM6_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM7_Init(void);

void BlinkLED(GPIO_TypeDef *LEDPort, uint16_t LEDPin, uint8_t blinkCount, uint32_t onTime, uint32_t offTime, uint32_t cycleDelay);
void Display_I2C(void *argument);
void ADC_read(void *argument);
void RS485_data(void *argument);
void Main(void *argument);
void Main_Cycle(void *argument); // основной режим в циклическом режиме
void Keyboard_task(void *argument);
void USB_COM_task(void *argument);
void UART_PARSER_task(void *argument);
void SD_Task(void *argument);
void Erroe_indicate(void *argument);
void HAL_TIM6_Callback(void);
void SetTimerPeriod(uint32_t period_ms);
void Watch_dog_task(void *argument);

unsigned int id = 0x00;
extern RTC_HandleTypeDef hrtc;
uint8_t units = 0;
uint8_t suspend = 0;

int main(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_Init();

  SystemClock_Config();
  PeriphCommonClock_Config();
  RTC_Init();
	HAL_RTC_GetTime(&hrtc, &Time_start, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &Date_start, RTC_FORMAT_BIN);
  RTC_read();

  MX_GPIO_Init();
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  HAL_Delay(500);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
  HAL_Delay(500);

  MX_ADC1_Init();
  //MX_ADC3_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_SPI2_Init();
  MX_UART4_Init();
  MX_TIM5_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  

  InitMenus();
  // Начальные состояния переферии - ВСЕ ОТКЛЮЧЕНО
  HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, 1);
  HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ADC_Pin, 1);
  HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 0);
  HAL_GPIO_WritePin(ON_RS_GPIO_Port, ON_RS_Pin, 0);     // Не включаем RS по умолчанию
  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, 0);
  HAL_GPIO_WritePin(EN_3P3V_GPIO_Port, EN_3P3V_Pin, 0);
  HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
  HAL_GPIO_WritePin(ON_DISP_GPIO_Port, ON_DISP_Pin, 0);
  HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 0);
  
  HAL_Delay(10);

  // ЗАПУСКАЕТСЯ ВСЕГДА 
  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, 1);             // Общее питание 5В
  HAL_GPIO_WritePin(EN_3P3V_GPIO_Port, EN_3P3V_Pin, 1);         // Общее питание 3.3В (АЦП, темп., и т.д.)
  HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 1);           // Включение Памяти на плате
  HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 1);

  HAL_Delay(10);
  //EEPROM_SaveSettings(&EEPROM);
  // Чтение данных из EEPROM
  if (!(EEPROM_IsDataExists()))
  {
    // Данных нету - первый запуск
    if (!EEPROM_SaveSettings(&EEPROM))
    {
      // Сохранение не вышло
    }
  }
  else
  {
    if (!EEPROM_LoadSettings(&EEPROM))
    {
      // Ошибка - неверный идентификатор данных
      if (!EEPROM_SaveSettings(&EEPROM))
      {
        // Сохранение не вышло
      }
    }
  }
  //EEPROM.Mode = 0;
  HAL_PWR_EnableBkUpAccess();
  uint16_t faultCode = HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_INDEX_ERROR_CODE);
  if (faultCode != 0U){
    switch (faultCode)
        {
        case FAULT_CODE_NMI:
            ERRCODE.STATUS |= STATUS_NMI_OCCURRED;
            break;
        case FAULT_CODE_HARDFAULT:
            ERRCODE.STATUS |= STATUS_HARDFAULT_OCCURRED;
            break;
        case FAULT_CODE_MEMMANAGE:
            ERRCODE.STATUS |= STATUS_MEMMANAGE_FAULT;
            break;
        case FAULT_CODE_BUSFAULT:
            ERRCODE.STATUS |= STATUS_BUSFAULT_OCCURRED;
            break;
        case FAULT_CODE_USAGEFAULT:
            ERRCODE.STATUS |= STATUS_USAGEFAULT_OCCURRED;
            break;
        default:
            // Неизвестный код — обрабатываем при желании
            break;
        }
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE, 0U);
  }

  uint32_t value = HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_INDEX_RESET_PROG);
  if (value == DATA_RESET_PROG){  
    // Если сброс из перехода в цикл
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_RESET_PROG, 0x00); // сбрасывавем флаг
  }
  else{
    //EEPROM.Mode = 0;
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) == RESET){
      // Если сброс не из перехода в цикл и не из за wakeup
      EEPROM.Mode = 0;
      // !! Добавить проверку на флаг доступности EEPROM
      if (!EEPROM_CheckDataValidity()){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
      }
    }
  }
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); // Сброс флага пробуждения из сна, для корректной работы сна
  HAL_PWR_DisableBkUpAccess();
  
  HAL_UART_Receive_IT(&huart4, &gsmRxChar, 1);
  HAL_NVIC_SetPriority(UART4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(UART4_IRQn);
  

  if ((EEPROM.USB_mode == 1) || (EEPROM.USB_mode == 2)){
    MX_USB_DEVICE_Init_COMPORT(); // Режим работы в VirtualComPort
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 6, 0); // Приоритет прерывания
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);         // Включение прерывания
  }
  // Запуск в режиме настройки (экран вкл)
  if (EEPROM.Mode == 0){
    // Включение переферии
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //MX_IWDG_Init();
    //!/
    HAL_IWDG_Refresh(&hiwdg);
    //HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 1); // Включение датчика давления и !!!  измерение текущего напряжения питания 1:10
    HAL_GPIO_WritePin(ON_DISP_GPIO_Port, ON_DISP_Pin, 1); // Включаем экран
    HAL_Delay(10);
    OLED_Init(&hi2c2);
    HAL_Delay(20);


    if (EEPROM.screen_sever_mode == 1) Start_video();
    HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, 1);
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, 1);
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, 1);
    HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, 1);
    HAL_NVIC_SetPriority(EXTI4_IRQn, 7, 0);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    
    // Настройка таймера клавиатуры
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 8, 0); // Установите приоритет
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);        // Включите прерывание
  }
  else{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  }

  // Инициализация переферии
  MX_DMA_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  w25_init();


  /*
  Ds18b20_Init();
  if (Ds18b20_ManualConvert())
  {
    for (uint8_t i = 0; i < 1; i++)
    {
      if (ds18b20[i].DataIsValid)
      {
        printf("Датчик %d: температура = %.2f °C\r\n", i, ds18b20[i].Temperature);
      }
      else
      {
        printf("Датчик %d: ошибка измерения\r\n", i);
      }
    }
  }
  else
  {
    printf("Преобразование температуры не завершилось вовремя!\r\n");
  }
*/


  HAL_IWDG_Refresh(&hiwdg);
  MS5193T_Init();
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

  HAL_IWDG_Refresh(&hiwdg);
  osKernelInitialize();

  //Ds18b20_Init((osPriority_t)osPriorityLow3);

  ADC_readHandle = osThreadNew(ADC_read, NULL, &ADC_read_attributes);
  RS485_dataHandle = osThreadNew(RS485_data, NULL, &RS485_data_attributes);
  UART_PARSER_taskHandle = osThreadNew(UART_PARSER_task, NULL, &UART_PARSER_task_attributes);
  SD_taskHandle = osThreadNew(SD_Task, NULL, &SD_task_attributes);
  
  if (EEPROM.Mode == 0){
    USB_COM_taskHandle = osThreadNew(USB_COM_task, NULL, &USB_COM_task_attributes);
    MainHandle = osThreadNew(Main, NULL, &Main_attributes); // Задача для настроечного режима
    Keyboard_taskHandle = osThreadNew(Keyboard_task, NULL, &Keyboard_task_attributes);
    Display_I2CHandle = osThreadNew(Display_I2C, NULL, &Display_I2C_attributes);
    ERROR_INDICATE_taskHandle = osThreadNew(Erroe_indicate, NULL, &Erroe_indicate_task_attributes);
    WATCDOG_taskHandle = osThreadNew(Watch_dog_task, NULL, &WATCDOG_task_attributes);
  }
  if (EEPROM.Mode == 1)
  {
    Main_Cycle_taskHandle = osThreadNew(Main_Cycle, NULL, &Main_Cycle_task_attributes); // Задача для циклического режима
    // Приостановка всех задач
    osThreadSuspend(ADC_readHandle);
    osThreadSuspend(RS485_dataHandle);
  }

  osKernelStart();


  while (1)
  {
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB | RCC_PERIPHCLK_SDMMC1 | RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLLSAI1;
  PeriphClkInit.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSE;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 12;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK | RCC_PLLSAI1_ADC1CLK;
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
ADC_HandleTypeDef hadc1; // Глобальный или статический, как у вас заведено

void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    // Включаем тактирование АЦП1 и порта PC0
    __HAL_RCC_ADC_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // Настраиваем PC0 как аналоговый вход (без подтяжки)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // Настраиваем ADC1: 12-битное разрешение, одиночное преобразование, программный запуск
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;    // Асинхронный клок без предделителя
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;          // 12-битное разрешение АЦП (значения от 0 до 4095)
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;          // Выравнивание результата по правому краю
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;          // Отключаем сканирование (одноканальный режим)
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;       // Флаг конца каждого одиночного преобразования
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;             // Непрерывный режим отключен (однократное преобразование)
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;    // Преобразование запускается программно (софтверный триггер)
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        // Обработка ошибки инициализации
        Error_Handler();
    }

    // Выбираем канал, соответствующий PC0 (канал 1 ADC1)
    sConfig.Channel = ADC_CHANNEL_1;         // PC0 подключен к Channel 1 АЦП1
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;  // Время выборки канала (47.5 тактов, пример)
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        // Обработка ошибки настройки канала
        Error_Handler();
    }
}

static void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4000;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }


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

static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 40-1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 0xFFFF;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

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


void MX_SDMMC1_SD_Init(void)
{
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 128;
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
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;  
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
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
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel4_IRQn);

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
                          |COL_B4_Pin|SPI2_CS_ADC_Pin|One_Wire_Pin|EN_3P3V_Pin;
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
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : STR_B4_Pin */
  GPIO_InitStruct.Pin = STR_B4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
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

void Main(void *argument)
{
  UNUSED(argument);
  vSemaphoreCreateBinary(Keyboard_semapfore);
  vSemaphoreCreateBinary(Display_semaphore);
  vSemaphoreCreateBinary(Display_cursor_semaphore);
  vSemaphoreCreateBinary(USB_COM_semaphore);
  vSemaphoreCreateBinary(UART_PARSER_semaphore);
  vSemaphoreCreateBinary(Main_semaphore);
  vSemaphoreCreateBinary(SD_WRITE);
    // Запуск глобального таймера для обновления экрана
  HAL_NVIC_SetPriority(TIM5_IRQn, 8, 0); // Установите приоритет
  HAL_NVIC_EnableIRQ(TIM5_IRQn);        // Включите прерывание
  HAL_TIM_Base_Start_IT(&htim5);

  // Если была критическая ошибка
  if (ERRCODE.STATUS & STATUS_FAULTS){
    Collect_DATA();
    flash_append_record(save_data, 0);
  }
  //int32_t adc =  ADC1_Read_PC0();

  for (;;)
  {
    RTC_read();
    if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
    xSemaphoreTake(Main_semaphore, portMAX_DELAY);
    osDelay(10);
  }
}
// Запускается только при циклическом режиме
void Main_Cycle(void *argument)
{
  UNUSED(argument);
  vSemaphoreCreateBinary(ADC_READY);                       
  vSemaphoreCreateBinary(Keyboard_semapfore);
  vSemaphoreCreateBinary(Display_cursor_semaphore);
  vSemaphoreCreateBinary(USB_COM_semaphore);
  vSemaphoreCreateBinary(UART_PARSER_semaphore);
  vSemaphoreCreateBinary(Main_semaphore);
  vSemaphoreCreateBinary(SD_WRITE);
  // 1. Включено -  АЦП, flash, EEPROM
  // 2. Уже конфигурация EEPROM прочитана

  // Если была критическая ошибка
  if (ERRCODE.STATUS & STATUS_FAULTS){
    Collect_DATA();
    flash_append_record(save_data, 0);
  }
  for (;;)
  {
    osDelay(10);
    // 3. Выключить EEPROM
    //HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 0); // Выключение Памяти на плате

    uint8_t status = 0;
    // 4. Запрос настроек с сайта для текущей конфигурации, запуск задачи параллельно, если фатальная ошибка - отключить GSM
    if (EEPROM.Communication != 0)
    {
      // 60 секунд на попытки зарагистрироваться
      for (int i = 0; i < 120; i++)
      {
        if ((GSM_data.Status & NETWORK_REGISTERED) && (GSM_data.Status & SIGNAL_PRESENT))
        {
          status = 1;
          break;
        }
        // Если прошло больше 22.5 секунд и сим карта не вставлена - перпезапускаем модуль
        if ((i==45) && (!(GSM_data.Status & SIM_PRESENT))){
          osThreadSuspend(UART_PARSER_taskHandle);
          HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
          osDelay(1000);
          HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 1);
          HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 1);
          osDelay(600);
          HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 0);
          osThreadResume(UART_PARSER_taskHandle);
          GSM_data.Status = 0;
        } 
        osDelay(500);
      }
      if ((status == 0) && ((GSM_data.Status & SIM_PRESENT))){
        // Если регистрация не прошла, но сим карта есть - перезагружаем модем
        osThreadSuspend(UART_PARSER_taskHandle);
        HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
        osDelay(1000);
        HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 1);
        HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 1);
        osDelay(600);
        HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 0);
        osThreadResume(UART_PARSER_taskHandle);
        GSM_data.Status = 0;
        for (int i = 0; i < 120; i++)
        {
          if ((GSM_data.Status & NETWORK_REGISTERED) && (GSM_data.Status & SIGNAL_PRESENT))
          {
            status = 1;
            break;
          }
          osDelay(500);
        }
      }
      if (status == 0)
      {
        // Связи нет и не предвидится - отключаем GSM модуль
        HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
        osThreadSuspend(UART_PARSER_taskHandle);
      }

      if (status == 1)
      {
        // Если зарегеистрировались - запрашиваем настройки
        GSM_data.Status |= HTTP_READ;
        for (int i = 0; i < 120; i++)
        {
          if (GSM_data.Status & HTTP_READ_Successfully)
          {
            status = 1;
            break;
          }
          if (ERRCODE.STATUS & STATUS_UART_SERVER_COMM_ERROR)
          {

            status = 0;
            break;
          }
          osDelay(500);
        }
      }
    }

    

    osDelay(500);
    // Читаем текущее напряжение питания
    for (int i = 0; i < 10; i++)
    {
      Read_ADC_Voltage();
    }
    osDelay(1000);
    // 5. Получения показаний АЦП
    //  Запускаем преобразования
    uint8_t  status_ADC = 0;
    if (!(ERRCODE.STATUS & STATUS_ADC_EXTERNAL_INIT_ERROR))
    {
      osThreadResume(ADC_readHandle);
      osDelay(500);
      status_ADC = 0;
      for (uint8_t i = 0; i < 50; i++)
      {
        if ((ADC_data.ADC_SI_value_char[0] != 'N') && (ADC_data.ADC_MS5193T_temp_char[0] != 'N')){
            status_ADC = 1;
            break;
        }
        osDelay(100);
      }
      if (status_ADC == 0)
      {
        ERRCODE.STATUS |= STATUS_ADC_EXTERNAL_SENSOR_ERROR;
      }
    }

    // 6. Отключить АЦП (датчик)
    //osThreadSuspend(ADC_readHandle);
    suspend = 0xFF;
    osDelay(350);
    osThreadSuspend(ADC_readHandle);
    HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 0);
    // Вызов функции отправки и полчучения настроек


    
    HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 0);
    HAL_GPIO_WritePin(ON_RS_GPIO_Port, ON_RS_Pin, 0);

    // Отправка данных на сервер

    // Если регистрация есть
    if (status == 1)
    {
      if (EEPROM.Communication != 0)
      {
        status = 0;
        GSM_data.Status |= HTTP_SEND;
        for (int i = 0; i < 5000; i++)
        {
          if (GSM_data.Status & HTTP_SEND_Successfully)
          {
            status = 1;
            break;
          }
          if (ERRCODE.STATUS & STATUS_UART_SERVER_COMM_ERROR)
          {
            status = 0;
            break;
          }
          osDelay(1000);
        }
        if ((ERRCODE.STATUS & STATUS_UART_SERVER_COMM_ERROR) || (status == 0))
        {
          GSM_data.Status |= SMS_SEND;
          for (int i = 0; i < 60; i++)
          {
            if (GSM_data.Status & SMS_SEND_Successfully)
            {
              status = 1;
              break;
            }
            if (ERRCODE.STATUS & STATUS_UART_SMS_SEND_ERROR)
            {
              status = 0;
              break;
            }
            osDelay(1000);
          }
        }
      }
    }
    else
    {
      ERRCODE.STATUS |= STATUS_GSM_REG_ERROR;
    }

    osThreadSuspend(UART_PARSER_taskHandle);
    osDelay(10);
    
    HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
    HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 1);
    osDelay(10);

    // 8.  Сохранение данных

    Collect_DATA();
    xSemaphoreGive(SD_WRITE);
    uint8_t send_status = 1;
    if ((ERRCODE.STATUS & STATUS_UART_SERVER_COMM_ERROR) || (ERRCODE.STATUS & STATUS_GSM_REG_ERROR)){
      send_status = 0; // Отметить как не отправленную 
    }
    // ! перенести в отдельную задачу
    flash_append_record(save_data, send_status);
    osDelay(200);


    HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, 1);
    HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ADC_Pin, 1);
    HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 0);
    HAL_GPIO_WritePin(ON_RS_GPIO_Port, ON_RS_Pin, 0); // Не включаем RS по умолчанию
    HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, 0);
    HAL_GPIO_WritePin(EN_3P3V_GPIO_Port, EN_3P3V_Pin, 0);
    HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
    HAL_GPIO_WritePin(ON_DISP_GPIO_Port, ON_DISP_Pin, 0);
    HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 0);
    osDelay(10);
    //Enter_StandbyMode(EEPROM.time_sleep_h, EEPROM.time_sleep_m);
    Enter_StandbyMode(0, 3);
    osDelay(10000);
    ERRCODE.STATUS |= STATUS_CRITICAL_ERROR;
  }
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
  for (;;)
  {
    // MX_USB_HOST_Process();

    ADC_data.update_value();
    if (EEPROM.Mode == 0)
      osDelay(300);
    else
    {
      osDelay(150);
      if (suspend == 0xFF) osThreadSuspend(ADC_readHandle);
    }
  }
}

void RS485_data(void *argument)
{
  UNUSED(argument);
  for (;;)
  {
    osDelay(3000);
    Read_ADC_Voltage(); // Измерение напряжения на АКБ
    if (EEPROM.Mode == 1) osThreadSuspend(RS485_dataHandle); // Остановить, если циклический режим (для однократного выполнения)
  }
}

void SD_Task(void *argument) {
  // Код задачи
  for (;;) {
    osDelay(1000);
    SD_check();
    if(xSemaphoreTake(SD_WRITE, pdMS_TO_TICKS(20000)) == pdTRUE)
    {
        WriteToSDCard();
    }
  }
}

uint32_t data_read_adc_in = 0;
void Display_I2C(void *argument)
{
  UNUSED(argument);
  for (;;)
  {
    xSemaphoreTake(Display_semaphore, portMAX_DELAY);
    Keyboard_processing();
    Display_all_menu();
  }
}

void Keyboard_task(void *argument)
{
  UNUSED(argument);
  for (;;)
  {
    xSemaphoreTake(Keyboard_semapfore, portMAX_DELAY);
    Keyboard();
    //Keyboard_processing();
    //osDelay(Timer_key_one_press);
  }
}


void Watch_dog_task(void *argument)
{
  UNUSED(argument);
  for (;;)
  {
    HAL_IWDG_Refresh(&hiwdg);
    osDelay(1000);
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

void BlinkLED(GPIO_TypeDef *LEDPort, uint16_t LEDPin, uint8_t blinkCount, uint32_t onTime, uint32_t offTime, uint32_t cycleDelay)
{
    for (uint8_t i = 0; i < blinkCount; i++)
    {
        // Включаем светодиод
        HAL_GPIO_WritePin(LEDPort, LEDPin, GPIO_PIN_SET);
        osDelay(onTime);
        
        // Выключаем светодиод
        HAL_GPIO_WritePin(LEDPort, LEDPin, GPIO_PIN_RESET);
        osDelay(offTime);
    }
    
    // Задержка между циклами моргания
    osDelay(cycleDelay);
}


// Индикация ошибок


void Erroe_indicate(void *argument)
{
  UNUSED(argument);
  uint64_t ErrorMask = 0;
  if (EEPROM.USB_mode == 0){
    MX_USB_HOST_Init();
  }
  for (;;)
  {
    Diagnostics();
    BlinkLED(GPIOC, GPIO_PIN_13, 1, 2000, 2000, 0);
    // Ошибка инициализации EEPROM
    ErrorMask = STATUS_EEPROM_INIT_ERROR
    | STATUS_EEPROM_WRITE_ERROR
    | STATUS_EEPROM_READ_ERROR
    | STATUS_EEPROM_CRC_ERROR;
    if (ERRCODE.STATUS & ErrorMask){
      BlinkLED(GPIOC, GPIO_PIN_13, 1, 500, 500, 0);
      goto skip;
    }
    // Ошибка инициализации АЦП
    ErrorMask = STATUS_ADC_EXTERNAL_INIT_ERROR
    | STATUS_ADC_EXTERNAL_SENSOR_ERROR
    | STATUS_ADC_BOARD_TEMP_ERROR
    | STATUS_ADC_RANGE_ERROR;
    if (ERRCODE.STATUS & ErrorMask){
      BlinkLED(GPIOC, GPIO_PIN_13, 2, 500, 500, 0);
      goto skip;
    }
    
    // Ошибка инициализации Flash
    ErrorMask = STATUS_FLASH_INIT_ERROR
    | STATUS_FLASH_WRITE_ERROR
    | STATUS_FLASH_READ_ERROR
    | STATUS_FLASH_CRC_ERROR;
    if (ERRCODE.STATUS & ErrorMask){
      BlinkLED(GPIOC, GPIO_PIN_13, 3, 500, 500, 0);
      goto skip;
    }
    
    
    // Ошибка инициализации SD
    ErrorMask = STATUS_SD_INIT_ERROR
                     | STATUS_SD_MOUNT_ERROR
                     | STATUS_SD_WRITE_ERROR
                     | STATUS_SD_READ_ERROR
                     | STATUS_SD_CORRUPTED_DATA
                     | STATUS_SD_CRC_MISMATCH
                     | STATUS_SD_FILE_OPEN_ERROR
                     | STATUS_SD_TEMP_OUT_OF_RANGE;
    if (ERRCODE.STATUS & ErrorMask) {
      BlinkLED(GPIOC, GPIO_PIN_13, 4, 500, 500, 0);
      goto skip;
    }
    skip:
      osDelay(1000);

      time_counter++;
      if (time_counter>120){
        time_counter = 0;
        //Screen_saver();
      }
      //Process_USB_Flash();
  }
}
void UART_PARSER_task(void *argument)
{
  UNUSED(argument);
  GSM_Init();
  for (;;)
  {
    osDelay(5000);
    if (EEPROM.Mode == 0)
    {
      if (!(GSM_data.Status & SMS_SEND))
      {
        strcpy(GSM_data.GSM_sms_status, "");
      }
      if (!(GSM_data.Status & HTTP_READ))
      {
        strcpy(GSM_data.GSM_site_read_status, "");
      }
      if (!(GSM_data.Status & HTTP_SEND))
      {
        strcpy(GSM_data.GSM_site_status, "");
      }
    }

    // Если GSM должен быть включен
    if ((EEPROM.Communication == 1) && (EEPROM.Mode != 1))
    {
      // Если GSM был выключен
      if (HAL_GPIO_ReadPin(EN_3P8V_GPIO_Port, EN_3P8V_Pin) == 0)
      {
        HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 1); // Включение GSM
        osDelay(100);
        HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 1);
        osDelay(600);
        HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 0);
      }
    }
    // Если GSM должен быть выключен
    if (EEPROM.Communication == 0)
    {
      HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
      continue;
    }
    // if (EEPROM.Mode != 0) osThreadSuspend(SIM800_dataHandle); // Остановить, если циклический режим (для однократного выполнения)

    if ((!(GSM_data.Status & GSM_RDY)) && (EEPROM.USB_mode != 2))
    {
      int result = SendCommandAndParse("AT\r", parse_ERROR_OK, 1000);
      if (result == 1)
      {
        GSM_data.Status |= GSM_RDY;
        if (SendCommandAndParse("AT+CFUN=0\r", waitForOKResponse, 1000) != 1)
        {
        }
        osDelay(300);
        if (SendCommandAndParse("AT+CFGDUALMODE=1,0\r", waitForOKResponse, 1000) != 1)
        {
        }
        osDelay(300);
        if (SendCommandAndParse("AT+CFGRATPRIO=2\r", waitForOKResponse, 1000) != 1)
        {
        }
        osDelay(300);
        if (SendCommandAndParse("AT+CFUN=1\r", waitForOKResponse, 1000) != 1)
        {
        }
        if (SendCommandAndParse("AT+CSCON=0\r", waitForOKResponse, 1000) != 1)
        {
        }
        if (SendCommandAndParse("AT&W\r", waitForOKResponse, 1000) != 1)
        {
        }
      }
    }
    if (GSM_data.Status & NETWORK_REGISTERED_SET_HTTP){
      GSM_data.Status&=~NETWORK_REGISTERED_SET_HTTP;
      SendCommandAndParse("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r", waitForOKResponse, 1000); 
      SendCommandAndParse("AT+CGACT=1,1\r", waitForOKResponse, 150000);
      SendCommandAndParse("AT+CDNSCFG=\"8.8.8.8\",\"77.88.8.8\"\r", waitForOKResponse, 1000);
    }

    if ((GSM_data.Status & GSM_RDY) && (EEPROM.USB_mode != 2))
    {
      SendCommandAndParse("AT+CPIN?\r", parse_CPIN, 1000);
      SendCommandAndParse("AT+CSQ\r", parse_CSQ, 1000);
      SendCommandAndParse("AT+CEREG?\r", parse_CEREG, 1000);
      SendCommandAndParse("AT+COPS?\r", parse_COPS, 1000);
      GSM_data.update_value();
      if (EEPROM.Mode == 0)
      if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
    }

    if (GSM_data.Status & SMS_SEND)
    {
      // флаг того что нужно отправить смс
      GSM_data.Status &= ~SMS_SEND;
      Collect_DATA();
      if (sendSMS() == HAL_OK)
      {
        if (EEPROM.Mode == 0) strcpy(GSM_data.GSM_sms_status, "OK");
        if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
        GSM_data.Status |= SMS_SEND_Successfully;
      }
      else
      {
        if (EEPROM.Mode == 0) strcpy(GSM_data.GSM_sms_status, "ERR");
        if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
        ERRCODE.STATUS |= STATUS_UART_SMS_SEND_ERROR;
      }
    }

    if (GSM_data.Status & HTTP_SEND)
    {
      GSM_data.Status &= ~HTTP_SEND;
      Collect_DATA();
      if (sendHTTP() == HAL_OK)
      {
        GSM_data.Status |= HTTP_SEND_Successfully;
        if (EEPROM.Mode == 0)
          strcpy(GSM_data.GSM_site_status, "OK");
        if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
      }
      else
      {
        if (EEPROM.Mode == 0)
          strcpy(GSM_data.GSM_site_status, "ERR");
        if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
        ERRCODE.STATUS |= STATUS_UART_SERVER_COMM_ERROR;
      }
    }
    if (GSM_data.Status & HTTP_READ)
    {
      // флаг того что нужно отправить HTTP
      GSM_data.Status &= ~HTTP_READ;
      SETTINGS_REQUEST_DATA();
      if (READ_Settings_sendHTTP() == HAL_OK)
      {
        GSM_data.Status |= HTTP_READ_Successfully;
        if (EEPROM.Mode == 0){
          strcpy(GSM_data.GSM_site_read_status, "OK");
          xSemaphoreGive(Display_semaphore);
        }
      }
      else
      {
        if (EEPROM.Mode == 0)
          strcpy(GSM_data.GSM_site_read_status, "ERR");
        if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
        ERRCODE.STATUS |= STATUS_UART_SERVER_COMM_ERROR;
      }
    }
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

