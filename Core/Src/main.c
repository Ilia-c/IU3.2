/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * 
 * 
 * BY PROGRAMMERA
 * VERSION Ver0.45
 * 
 * 
 *
 ******************************************************************************
 */

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
xSemaphoreHandle SLEEP_semaphore;

extern const uint16_t Timer_key_one_press;
extern const uint16_t Timer_key_press_fast;

extern uint8_t gsmRxChar;
extern EEPROM_Settings_item EEPROM;
extern ERRCODE_item ERRCODE;



I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
SPI_HandleTypeDef hspi2;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart4;

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc3;

TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim8;

IWDG_HandleTypeDef hiwdg;

void MX_GPIO_Init(void);  
void MX_ADC1_Init(void);        // 
void MX_ADC3_Init(void);        // 
void MX_I2C1_Init(void);        // 
void MX_I2C2_Init(void);        // 
void MX_SPI2_Init(void);        // АЦП+FLASH
//static void MX_UART1_Init(void);     // RS-485
void MX_UART4_Init(void);       // GSM
void MX_TIM5_Init(void);
void MX_TIM6_Init(void);
void MX_IWDG_Init(void);
void MX_TIM7_Init(void);
void MX_TIM8_Init(void);

void HAL_TIM5_Callback(void);
/* Definitions for Main */
osThreadId_t MainHandle;
const osThreadAttr_t Main_attributes = {
    .name = "Main",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityHigh5,
};
osThreadId_t Main_Cycle_taskHandle;
const osThreadAttr_t Main_Cycle_task_attributes = {
    .name = "Main_Cycle_task",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityHigh5,
};

/* Definitions for Display_I2C */
osThreadId_t Display_I2CHandle;
const osThreadAttr_t Display_I2C_attributes = {
    .name = "Display_I2C",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityHigh,
};
/* Definitions for ADC_read */
osThreadId_t ADC_readHandle;
const osThreadAttr_t ADC_read_attributes = {
    .name = "ADC_read",
    .stack_size = 1024 * 3,
    .priority = (osPriority_t)osPriorityLow3,
};
/* Definitions for RS485_data */
osThreadId_t RS485_dataHandle;
const osThreadAttr_t RS485_data_attributes = {
    .name = "RS485_data",
    .stack_size = 1024 * 3,
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
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityLow2,
};

osThreadId_t  ERROR_INDICATE_taskHandle;
const osThreadAttr_t Erroe_indicate_task_attributes = {
    .name = "Erroe_indicate_task",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)osPriorityLow1,
};

osThreadId_t  UART_PARSER_taskHandle;
const osThreadAttr_t UART_PARSER_task_attributes = {
    .name = "UART_PARSER_task",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityHigh4,
};

osThreadId_t WATCDOG_taskHandle;
const osThreadAttr_t WATCDOG_task_attributes = {
    .name = "Watch_dog_task",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)osPriorityHigh3,
};

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);     

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
  MX_TIM8_Init();
  

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
  HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 1);         // Включение 

  HAL_Delay(10);

  // Чтение данных из EEPROM
  if (EEPROM_CHECK() == HAL_OK)
  {
    if (EEPROM_IsDataExists() != HAL_OK)
    {
      // Данных нету - первый запуск
      if (EEPROM_SaveSettings(&EEPROM) != HAL_OK)
      {
        // Сохранение не вышло
      }
    }
    else
    {

      if (EEPROM_LoadSettings(&EEPROM) != HAL_OK)
      {
        // Ошибка - неверный идентификатор данных
        if (EEPROM_SaveSettings(&EEPROM) != HAL_OK)
        {
          // Сохранение не вышло
        }
      }
    }
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
      if (ERRCODE.STATUS & STATUS_EEPROM_INIT_ERROR)
      {
        if (EEPROM_CheckDataValidity() != HAL_OK)
        {
          ERRCODE.STATUS |= STATUS_EEPROM_READ_ERROR;
        }
      }
    }
  }
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); // Сброс флага пробуждения из сна, для корректной работы сна

  HAL_PWR_EnableBkUpAccess();
  uint32_t errcode_low = HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_INDEX_ERROR_CODE_1);
  uint32_t errcode_high = HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_INDEX_ERROR_CODE_2);
  ERRCODE.STATUS = (((uint64_t)errcode_high << 32) | errcode_low);
  if (EEPROM.Mode != 0){
    // Проверяем есть ли подтвержденная фатальная ошибка
    if (ERRCODE.STATUS & STATUS_STACK_OVERFLOW) if (ERRCODE.STATUS & STATUS_STACK_OVERFLOW_ACK) Enter_StandbyMode_NoWakeup();
    if (ERRCODE.STATUS & STATUS_HARDFAULT_OCCURRED) if (ERRCODE.STATUS & STATUS_HARDFAULT_OCCURRED_ACK) Enter_StandbyMode_NoWakeup();
    if (ERRCODE.STATUS & STATUS_NMI_OCCURRED) if (ERRCODE.STATUS & STATUS_NMI_OCCURRED_ACK) Enter_StandbyMode_NoWakeup();
    if (ERRCODE.STATUS & STATUS_MEMMANAGE_FAULT) if (ERRCODE.STATUS & STATUS_MEMMANAGE_FAULT_ACK) Enter_StandbyMode_NoWakeup();
    if (ERRCODE.STATUS & STATUS_BUSFAULT_OCCURRED) if (ERRCODE.STATUS & STATUS_BUSFAULT_OCCURRED_ACK) Enter_StandbyMode_NoWakeup();
    if (ERRCODE.STATUS & STATUS_USAGEFAULT_OCCURRED) if (ERRCODE.STATUS & STATUS_USAGEFAULT_OCCURRED_ACK) Enter_StandbyMode_NoWakeup();
  }
  if (ERRCODE.STATUS & STATUS_STACK_OVERFLOW)  ERRCODE.STATUS |= STATUS_STACK_OVERFLOW_ACK;
  if (ERRCODE.STATUS & STATUS_HARDFAULT_OCCURRED)  ERRCODE.STATUS |= STATUS_HARDFAULT_OCCURRED_ACK;
  if (ERRCODE.STATUS & STATUS_NMI_OCCURRED)  ERRCODE.STATUS |= STATUS_NMI_OCCURRED_ACK;
  if (ERRCODE.STATUS & STATUS_MEMMANAGE_FAULT)  ERRCODE.STATUS |= STATUS_MEMMANAGE_FAULT_ACK;
  if (ERRCODE.STATUS & STATUS_BUSFAULT_OCCURRED)  ERRCODE.STATUS |= STATUS_BUSFAULT_OCCURRED_ACK;
  if (ERRCODE.STATUS & STATUS_USAGEFAULT_OCCURRED)  ERRCODE.STATUS |= STATUS_USAGEFAULT_OCCURRED_ACK;
  ERRCODE.STATUS = ERRCODE.STATUS & (STATUS_FAULTS_ACK); // Сбрасываем все ошибки кроме ACK
  errcode_low  = (uint32_t)(ERRCODE.STATUS & 0xFFFFFFFF);
  errcode_high = (uint32_t)(ERRCODE.STATUS >> 32);
  HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_1, errcode_low);
  HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_2, errcode_high);
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
    #if Debug_mode == 0
    MX_IWDG_Init();
    #endif
    HAL_IWDG_Refresh(&hiwdg);
    HAL_GPIO_WritePin(ON_DISP_GPIO_Port, ON_DISP_Pin, 1); // Включаем экран
    HAL_Delay(300);
    OLED_Init(&hi2c2);
    HAL_Delay(150);

    if (EEPROM.screen_sever_mode == 1) Start_video();

    HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, 1);
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, 1);
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, 1);
    HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, 1);
    HAL_NVIC_SetPriority(EXTI4_IRQn, 7, 0);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    
    // Таймер ухода в сон (либо заставки)

    // Настройка таймера клавиатуры
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 8, 0); // Установите приоритет
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);        // Включите прерывание
  }
  else{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  }

  MX_FATFS_Init();
  w25_init();


  HAL_IWDG_Refresh(&hiwdg);
  MS5193T_Init();
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

  HAL_IWDG_Refresh(&hiwdg);
  osKernelInitialize();

  ADC_readHandle = osThreadNew(ADC_read, NULL, &ADC_read_attributes);
  RS485_dataHandle = osThreadNew(RS485_data, NULL, &RS485_data_attributes);
  UART_PARSER_taskHandle = osThreadNew(UART_PARSER_task, NULL, &UART_PARSER_task_attributes);
  
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



void Main(void *argument)
{
  UNUSED(argument);
  vSemaphoreCreateBinary(Keyboard_semapfore);
  vSemaphoreCreateBinary(Display_semaphore);
  vSemaphoreCreateBinary(Display_cursor_semaphore);
  vSemaphoreCreateBinary(USB_COM_semaphore);
  vSemaphoreCreateBinary(UART_PARSER_semaphore);
  vSemaphoreCreateBinary(Main_semaphore);
  vSemaphoreCreateBinary(SLEEP_semaphore);
  xSemaphoreTake(SLEEP_semaphore, 0);

    // Запуск глобального таймера для обновления экрана
  HAL_NVIC_SetPriority(TIM5_IRQn, 8, 0); // Установите приоритет
  HAL_NVIC_EnableIRQ(TIM5_IRQn);        // Включите прерывание
  HAL_TIM_Base_Start_IT(&htim5);


  __HAL_TIM_CLEAR_FLAG(&htim8, TIM_FLAG_UPDATE);
  HAL_NVIC_SetPriority(TIM8_UP_IRQn, 10, 0);
  HAL_NVIC_EnableIRQ(TIM8_UP_IRQn);
  HAL_TIM_Base_Start_IT(&htim8);

  // Если была критическая ошибка
  if (ERRCODE.STATUS & (FAULTS_MASK)){
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
  // 1. Включено -  АЦП, flash, EEPROM
  // 2. Уже конфигурация EEPROM прочитана
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
        HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
        osDelay(1000);
        HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 1);
        HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 1);
        osDelay(600);
        HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 0);
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
      EEPROM_CHECK();
      if (status == 1)
      {
        // Если зарегеистрировались - запрашиваем настройки
        GSM_data.Status |= HTTP_READ;
        for (int i = 0; i < 120; i++)
        {
          if (GSM_data.Status & HTTP_READ_Successfully)
          {
            break;
          }
          if (ERRCODE.STATUS & STATUS_HTTP_SERVER_COMM_ERROR)
          {
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
    if (ERRCODE.STATUS & STATUS_VOLTAGE_TOO_LOW) Enter_StandbyMode_NoWakeup();
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
        ERRCODE.STATUS |= STATUS_ADC_TIMEOUT_CYCLE_ERROR;
      }
    }

    // 6. Отключить АЦП (датчик)
    //osThreadSuspend(ADC_readHandle);
    suspend = 0xFF;
    osDelay(350);
    
    HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 0);
    osThreadSuspend(ADC_readHandle);
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
        for (int i = 0; i < 120; i++)
        {
          if (GSM_data.Status & HTTP_SEND_Successfully)
          {
            status = 1;
            break;
          }
          if (ERRCODE.STATUS & STATUS_HTTP_SERVER_COMM_ERROR)
          {
            status = 0;
            break;
          }
          osDelay(1000);
        }
        if ((ERRCODE.STATUS & STATUS_HTTP_SERVER_COMM_ERROR) || (status == 0))
        {
          GSM_data.Status &= ~HTTP_SEND;
          GSM_data.Status |= SMS_SEND;
          for (int i = 0; i < 60; i++)
          {
            if (GSM_data.Status & SMS_SEND_Successfully)
            {
              status = 1;
              break;
            }
            if (ERRCODE.STATUS & STATUS_GSM_SMS_SEND_ERROR)
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
    uint8_t send_status = 1;
    if ((ERRCODE.STATUS & STATUS_HTTP_SERVER_COMM_ERROR) || (ERRCODE.STATUS & STATUS_GSM_REG_ERROR)){
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
    Enter_StandbyMode(EEPROM.time_sleep_h, EEPROM.time_sleep_m);
    osDelay(10000);
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
    if (xSemaphoreTake(SLEEP_semaphore, 10) == pdTRUE)
    {
      __HAL_TIM_SET_COUNTER(&htim8, 0);
      if (ERRCODE.STATUS & STATUS_VOLTAGE_TOO_LOW) Enter_StandbyMode_NoWakeup();
      if (EEPROM.block == 2) return;

      if (EEPROM.block == 1)
      {
        Screen_saver();
        continue;
      }

      // Если бездействие больше 5 минут
      ERRCODE.STATUS |= STATUS_IDLE_LOOP_MODE;
      EEPROM.Mode = 1;
      EEPROM.time_sleep_h = 1;
      EEPROM.time_sleep_m = 0;

      EEPROM_SaveSettings(&EEPROM);
      if (EEPROM_CheckDataValidity() != HAL_OK)
      {
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
      }

      HAL_PWR_EnableBkUpAccess();
      HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_RESET_PROG, DATA_RESET_PROG);
      uint32_t errcode_low  = (uint32_t)(ERRCODE.STATUS & 0xFFFFFFFF);
      uint32_t errcode_high = (uint32_t)(ERRCODE.STATUS >> 32);
      HAL_PWR_EnableBkUpAccess();
      HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_1, errcode_low);
      HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_2, errcode_high);

      HAL_PWR_DisableBkUpAccess();
      NVIC_SystemReset();
      HAL_Delay(1000);
    }
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
  Collect_DATA();
  for (;;)
  {
    //Collect_DATA();
    //flash_append_record(save_data, 0);
    //osDelay(1);
    //continue;
  
    Diagnostics();
    BlinkLED(GPIOC, GPIO_PIN_13, 1, 2000, 2000, 0);
    // Ошибка инициализации EEPROM
    ErrorMask = STATUS_EEPROM_INIT_ERROR
    | STATUS_EEPROM_READY_ERROR
    | STATUS_EEPROM_WRITE_ERROR
    | STATUS_EEPROM_READ_ERROR
    | STATUS_EEPROM_CRC_ERROR
    | STATUS_EEPROM_TIMEOUT_I2C_ERROR;
    if (ERRCODE.STATUS & ErrorMask){
      BlinkLED(GPIOC, GPIO_PIN_13, 1, 500, 500, 0);
      goto skip;
    }
    // Ошибка инициализации АЦП
    ErrorMask = STATUS_ADC_EXTERNAL_INIT_ERROR
    | STATUS_ADC_TIMEOUT_ERROR
    | STATUS_ADC_READY_ERROR
    | STATUS_ADC_TIMEOUT_CYCLE_ERROR;
    if (ERRCODE.STATUS & ErrorMask){
      BlinkLED(GPIOC, GPIO_PIN_13, 2, 500, 500, 0);
      goto skip;
    }
    
    // Ошибка инициализации Flash
    ErrorMask = STATUS_FLASH_INIT_ERROR
    | STATUS_FLASH_READY_ERROR
    | STATUS_FLASH_SEND_ERROR
    | STATUS_FLASH_RECV_ERROR
    | STATUS_FLASH_TIEOUT_ERROR
    | STATUS_FLASH_CRC_ERROR
    | STATUS_FLASH_OVERFLOW_ERROR;
    if (ERRCODE.STATUS & ErrorMask){
      BlinkLED(GPIOC, GPIO_PIN_13, 3, 500, 500, 0);
      goto skip;
    }
    
    
    skip:
      osDelay(1000);
  }
}

static uint8_t delay_AT_OK = 0; // Количество попыток получить OK от модема 
void UART_PARSER_task(void *argument)
{
  UNUSED(argument);
  GSM_Init();
  for (;;)
  {
    // Если GSM должен быть включен
    if ((EEPROM.Communication == 1))
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

    // if (EEPROM.Mode != 0) osThreadSuspend(SIM800_dataHandle); // Остановить, если циклический режим (для однократного выполнения)

    if ((!(GSM_data.Status & GSM_RDY)) && (EEPROM.USB_mode != 2))
    {
      int result = SendCommandAndParse("AT\r", parse_ERROR_OK, 1000);
      delay_AT_OK++;
      if (result == 1)
      {
        delay_AT_OK = 0;
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
      if (delay_AT_OK > 20){
        ERRCODE.STATUS |= STATUS_UART_NO_RESPONSE;
      }
    }
    if (GSM_data.Status & NETWORK_REGISTERED_SET_HTTP){
      GSM_data.Status&=~NETWORK_REGISTERED_SET_HTTP;
      SendCommandAndParse("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r", waitForOKResponse, 1000); 
      SendCommandAndParse("AT+CGACT=1,1\r", waitForOKResponse, 60000);
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
        ERRCODE.STATUS |= STATUS_GSM_SMS_SEND_ERROR;
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
        ERRCODE.STATUS &= ~STATUS_HTTP_SERVER_COMM_ERROR;
      }
      else
      {
        if (EEPROM.Mode == 0)
          strcpy(GSM_data.GSM_site_status, "ERR");
        if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
        ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
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
          ERRCODE.STATUS &= ~STATUS_HTTP_SERVER_COMM_ERROR;
        }
      }
      else
      {
        if (EEPROM.Mode == 0)
        {
          strcpy(GSM_data.GSM_site_read_status, "ERR");
          xSemaphoreGive(Display_semaphore);
        }
        ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
      }
    }
  }
}


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

