#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "RTC_data.h"
#include "OLED.h"
#include "Menu_data.h"
#include "OLED_Fonts.h"
#include "OLED_Icons.h"
#include "Keyboard.h"
#include "Settings.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "w25q128.h"
#include "MS5193T.h"
#include "Data_collect.h"
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
#include "stm32l4xx_hal_crc.h"

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
xSemaphoreHandle ADC_conv_end_semaphore;

extern const uint16_t Timer_key_one_press;
extern const uint16_t Timer_key_press_fast;

extern uint8_t gsmRxChar;
extern ERRCODE_item ERRCODE;
extern USBD_HandleTypeDef hUsbDeviceFS; 

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
SPI_HandleTypeDef hspi2;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;

RNG_HandleTypeDef hrng;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim8;

IWDG_HandleTypeDef hiwdg;
extern ADC_HandleTypeDef    hadc2;
extern ADC_HandleTypeDef    hadc1;
extern DMA_HandleTypeDef    hdma_adc2;
extern DMA_HandleTypeDef    hdma_adc1;
extern int mode_redact;

void MX_GPIO_Init(void);  
void MX_ADC2_Init(void);        // 
void MX_ADC1_Init(void);        // 
void MX_I2C1_Init(void);        // 
void MX_I2C2_Init(void);        // 
void MX_I2C3_Init(void);        // 
void MX_SPI2_Init(void);        // АЦП+FLASH
//static void MX_UART1_Init(void);     // RS-485
void MX_UART4_Init(void);       // GSM
void MX_UART5_Init(void);
void MX_TIM5_Init(void);
void MX_TIM6_Init(void);
void MX_IWDG_Init(void);
void MX_TIM7_Init(void);
void MX_TIM8_Init(void);
void MX_RNG_Init(void);

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
    .stack_size = 1024 * 7,
    .priority = (osPriority_t)osPriorityHigh,
};
/* Definitions for ADC_read */
osThreadId_t ADC_readHandle;
const osThreadAttr_t ADC_read_attributes = {
    .name = "ADC_read",
    .stack_size = 1024 * 5,
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
    .stack_size = 1024 * 5,
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
void MX_DMA_Init(void);
void USB_DEBUG_MESSAGE(const char message[], uint8_t category, uint8_t debugLVL);
void CRC_Init(void);

unsigned int id = 0x00;
extern RTC_HandleTypeDef hrtc;
uint8_t units = 0;
uint8_t suspend = 0;

int main(void)
{
  SCB->VTOR = 0x08010000U;
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_Init();

  SystemClock_Config();
  PeriphCommonClock_Config();
  RTC_Init();
	HAL_RTC_GetTime(&hrtc, &Time_start, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &Date_start, RTC_FORMAT_BIN);

  RTC_read();
  MX_GPIO_Init();

  HAL_GPIO_WritePin(RESERVED_GPIO_Port, RESERVED_Pin, GPIO_PIN_SET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(RESERVED_GPIO_Port, RESERVED_Pin, GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(RESERVED_GPIO_Port, RESERVED_Pin, GPIO_PIN_SET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(RESERVED_GPIO_Port, RESERVED_Pin, GPIO_PIN_RESET);
  HAL_Delay(100);

  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();

  MX_I2C1_Init();
  #if BOARD_VERSION == Version3_80
     MX_I2C3_Init();
     MX_UART5_Init();
     RS485_StartReceive_IT();
  #elif BOARD_VERSION == Version3_75
     MX_I2C2_Init();
  #endif
  MX_SPI2_Init();
  MX_UART4_Init();
  MX_TIM5_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_TIM8_Init();


  InitMenus();
  Boot_CopyVersion(&bootloader_data, sizeof(bootloader_data)); // Копируем версию загрузчика в переменную

  // Начальные состояния переферии - ВСЕ ОТКЛЮЧЕНО
  HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, 1);
  HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ADC_Pin, 1);
  #if BOARD_VERSION == Version3_80
    HAL_GPIO_WritePin(ON_OWEN_1_GPIO_Port, ON_OWEN_1_Pin, 0);
    HAL_GPIO_WritePin(ON_OWEN_2_GPIO_Port, ON_OWEN_2_Pin, 0);
    HAL_GPIO_WritePin(ON_OWEN_3_GPIO_Port, ON_OWEN_3_Pin, 0);
  #elif BOARD_VERSION == Version3_75
     HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 0);
  #endif

  HAL_GPIO_WritePin(ON_RS_GPIO_Port, ON_RS_Pin, 0);     // Не включаем RS по умолчанию
  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, 0);
  HAL_GPIO_WritePin(EN_3P3V_GPIO_Port, EN_3P3V_Pin, 0);
  HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
  HAL_GPIO_WritePin(ON_DISP_GPIO_Port, ON_DISP_Pin, 0);
  #if BOARD_VERSION == Version3_75 
  HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 0);
  #endif
  
  HAL_Delay(10);

  // ЗАПУСКАЕТСЯ ВСЕГДА 
  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, 1);             // Общее питание 5В
  HAL_GPIO_WritePin(EN_3P3V_GPIO_Port, EN_3P3V_Pin, 1);         // Общее питание 3.3В (АЦП, темп., и т.д.)
  #if BOARD_VERSION == Version3_75 
    HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 1);           // Включение Памяти на плате
    HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 1);
  #elif BOARD_VERSION == Version3_80
    HAL_GPIO_WritePin(ON_OWEN_1_GPIO_Port, ON_OWEN_1_Pin, 1);
    HAL_GPIO_WritePin(ON_OWEN_2_GPIO_Port, ON_OWEN_2_Pin, 1);
    HAL_GPIO_WritePin(ON_OWEN_3_GPIO_Port, ON_OWEN_3_Pin, 1);
  #endif

  HAL_Delay(20);
  //EEPROM_SaveSettings(&EEPROM); // Сохраняем настройки EEPROM в первый раз
  // !Чтение данных из EEPROM
  if (EEPROM_CHECK() == HAL_OK)
  {
    if (EEPROM_IsDataExists() != HAL_OK)
    {
      // !Данных нету - первый запуск
      if (EEPROM_SaveSettings(&EEPROM) != HAL_OK)
      {
        //! Сохранение не вышло
      }
    }
    else
    {

      if (EEPROM_LoadSettings(&EEPROM) != HAL_OK)
      {
        //! Ошибка - неверный идентификатор данных
        if (EEPROM_SaveSettings(&EEPROM) != HAL_OK)
        {
          //! Сохранение не вышло
        }
      }
    }
  }

  HAL_PWR_EnableBkUpAccess();
  uint32_t value = HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_INDEX_RESET_PROG);
  HAL_PWR_DisableBkUpAccess();
  if (value == DATA_RESET_PROG){  
    // Если сброс из перехода в цикл
    EEPROM.Mode = 1;
  }
  else{ 
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) == RESET){
      // Если сброс не из перехода в цикл и не из за wakeup (Питание дернули)
      EEPROM_clear_time_init();
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
  State_update();
  HAL_PWR_EnableBkUpAccess();
  EEPROM_LoadLastTimeWork(); // Загружаем последнее время работы
  HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_RESET_PROG, 0);
  PowerUP_counter();
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

  ADC_Start();
  CRC_Init();
  
  // Запуск в режиме настройки (экран вкл)
  if (EEPROM.Mode == 0){
    // Включение переферии
    #if Debug_mode == 0
    MX_IWDG_Init();
    #endif
    HAL_IWDG_Refresh(&hiwdg);
    HAL_GPIO_WritePin(ON_DISP_GPIO_Port, ON_DISP_Pin, 1); // Включаем экран
    HAL_Delay(300);
    OLED_Init(&i2cDisplay);
    HAL_Delay(150);

    if (EEPROM.screen_sever_mode == 1) Start_video();
    else
    {
      OLED_SendCommand(0xD9); //  Установка фаз на рабочий режим
      OLED_SendCommand(0xF1); //  Установка фаз на рабочий режим
      OLED_SetContrast(0xFF);
    }
    HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, 1);
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, 1);
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, 1);
    HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, 1);
    
    // Таймер ухода в сон (либо заставки)+
    // Настройка таймера клавиатуры
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 8, 0); // Установите приоритет
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);        // Включите прерывание
  }
  else{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  }

  if (EEPROM.USB_mode == USB_DEBUG){
    MX_USB_DEVICE_Init_COMPORT();            // Режим работы в VirtualComPort
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 6, 0); // Приоритет прерывания
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);         // Включение прерывания
  }
  MX_FATFS_Init();
  w25_init();


  HAL_IWDG_Refresh(&hiwdg);
  MS5193T_Init();

  HAL_IWDG_Refresh(&hiwdg);
  osKernelInitialize();

  if (EEPROM.block != 2)
  {
    ADC_readHandle = osThreadNew(ADC_read, NULL, &ADC_read_attributes);
    RS485_dataHandle = osThreadNew(RS485_data, NULL, &RS485_data_attributes);
    UART_PARSER_taskHandle = osThreadNew(UART_PARSER_task, NULL, &UART_PARSER_task_attributes);
  }

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
    osThreadSuspend(UART_PARSER_taskHandle);
  }
  osKernelStart();
  while (1){}
}

// ВАЖНО! Сбрасывает DATA_READ при DEBUG_GSM
void USB_DEBUG_MESSAGE(const char message[], uint8_t category, uint8_t debugLVL)
{
    if (EEPROM.USB_mode != USB_DEBUG)                return;
    if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) return;
    if ((EEPROM.DEBUG_CATEG & category) == 0)        return;
    if (EEPROM.DEBUG_LEVL   < debugLVL)              return;
    if (USB_TERMINAL_STATUS == TERMINAL_DISABLE)     return;

    size_t len = strlen(message);
    while (CDC_Transmit_FS((uint8_t *)"---",  3) == USBD_BUSY);
    while (CDC_Transmit_FS((uint8_t *)message, len) == USBD_BUSY);
    while ( CDC_Transmit_FS((uint8_t *)"\r\n", 2) == USBD_BUSY);
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

  HAL_NVIC_SetPriority(EXTI4_IRQn, 7, 0);
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

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
  vSemaphoreCreateBinary(ADC_conv_end_semaphore);

  for (;;)
  {
    osDelay(10);
    ADC_Voltage_Calculate(); // Читаем текущее напряжение питания
    if (ERRCODE.STATUS & STATUS_VOLTAGE_TOO_LOW) Enter_StandbyMode_NoWakeup();
    osDelay(1000);
    if (EEPROM.block == 2) Enter_StandbyMode_NoWakeup();

    // Получаем показания датчиков
    // !!! Переписать этот ужас
    if (!(ERRCODE.STATUS & STATUS_ADC_EXTERNAL_INIT_ERROR))
    {
      osThreadResume(ADC_readHandle);
      osDelay(1000);
      if (xSemaphoreTake(ADC_conv_end_semaphore, pdMS_TO_TICKS(10000)) != pdFALSE)
      {
        if ((ADC_data.ADC_SI_value_char[0][0] != 'N'))
        {
        }
        else{
          ERRCODE.STATUS |= STATUS_ADC_TIMEOUT_CYCLE_ERROR;
        }
      }
      else{
        ERRCODE.STATUS |= STATUS_ADC_TIMEOUT_CYCLE_ERROR;
      }
    }

    //suspend = 0xFF;
    osThreadSuspend(ADC_readHandle);
    
    #if BOARD_VERSION == Version3_75 
      HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 0);
      HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 1);
    #elif BOARD_VERSION == Version3_80
      HAL_GPIO_WritePin(ON_OWEN_1_GPIO_Port, ON_OWEN_1_Pin, 0);
      HAL_GPIO_WritePin(ON_OWEN_2_GPIO_Port, ON_OWEN_2_Pin, 0);
      HAL_GPIO_WritePin(ON_OWEN_3_GPIO_Port, ON_OWEN_3_Pin, 0);
    #endif

    HAL_GPIO_WritePin(ON_RS_GPIO_Port, ON_RS_Pin, 0);
    HAL_GPIO_WritePin(EN_3P3V_GPIO_Port, EN_3P3V_Pin, 1);
    HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, 1);
    //HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 1);

    osThreadResume(UART_PARSER_taskHandle); // Вклюбчаем GSM
    
    // Вызов функции отправки и полчучения настроек
    HAL_StatusTypeDef status = HAL_ERROR;
    if (EEPROM.Communication != M2M_DISABLE)
    {
      // 60 секунд на попытки зарагистрироваться
      for (int i = 0; i < 120; i++)
      {
        if ((GSM_data.Status & NETWORK_REGISTERED) && (GSM_data.Status & SIGNAL_PRESENT))
        {
          status = HAL_OK;
          break;
        }
        if (GSM_data.Status & STATUS_UART_NO_RESPONSE)
        {
          USB_DEBUG_MESSAGE("[DEBUG AT] Модуля связи нету", DEBUG_GSM, DEBUG_LEVL_2);
          HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
          osThreadSuspend(UART_PARSER_taskHandle);
          status = HAL_ERROR;
          GSM_data.Status = 0;
        }
        // Если прошло больше 22.5 секунд и сим карта не вставлена - перпезапускаем модуль
        if ((i == 45) && (!(GSM_data.Status & SIM_PRESENT)))
        {
          USB_DEBUG_MESSAGE("[DEBUG AT] Таймаут сим карты, перезагрузка модуля связи", DEBUG_GSM, DEBUG_LEVL_2);
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

      if ((status == HAL_ERROR) && ((GSM_data.Status & SIM_PRESENT)))
      {
        // Если регистрация не прошла, но сим карта есть - перезагружаем модем
        USB_DEBUG_MESSAGE("[DEBUG AT] Таймаут регистрации, перезагрузка модуля связи", DEBUG_GSM, DEBUG_LEVL_2);
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
            status = HAL_OK;
            break;
          }
          osDelay(500);
        }
      }
      if (status == HAL_ERROR)
      {
        // Связи нет и не предвидится - отключаем GSM модуль
        USB_DEBUG_MESSAGE("[DEBUG AT] Все попытки зарегистрироваться неуспешны - выключаем связь", DEBUG_GSM, DEBUG_LEVL_2);
        HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
        osThreadSuspend(UART_PARSER_taskHandle);
        GSM_data.Status = 0;
      }

      // Если регистрация есть
      if (status == HAL_OK)
      {
        status = HAL_ERROR;
        USB_DEBUG_MESSAGE("[DEBUG AT] Начата процедура отправки HTTP в цикле", DEBUG_GSM, DEBUG_LEVL_2);
        GSM_data.Status |= HTTP_SEND;
        for (int i = 0; i < 240; i++)
        {
          if (GSM_data.Status & HTTP_SEND_Successfully)
          {
            status = HAL_OK;

            break;
          }
          if (ERRCODE.STATUS & STATUS_HTTP_SERVER_COMM_ERROR)
          {
            break;
          }
          osDelay(1000);
        }
        if (((ERRCODE.STATUS & STATUS_HTTP_SERVER_COMM_ERROR) || (status == HAL_ERROR)) && !(ERRCODE.STATUS & STATUS_HTTP_NO_BINDING_ERROR))
        {
          USB_DEBUG_MESSAGE("[DEBUG AT] Все попытки HTTP неуспешны - начать отправку смс", DEBUG_GSM, DEBUG_LEVL_2);
          GSM_data.Status &= ~HTTP_SEND;
          GSM_data.Status &= ~SMS_SEND_Successfully;
          GSM_data.Status &= ~STATUS_GSM_SMS_SEND_ERROR;
          GSM_data.Status |= SMS_SEND;
          for (int timeout_sms = 0; timeout_sms < 120; timeout_sms++)
          {
            if (GSM_data.Status & SMS_SEND_Successfully)
            {
              USB_DEBUG_MESSAGE("[DEBUG AT] СМС отправлена", DEBUG_GSM, DEBUG_LEVL_3);
              status = HAL_OK;
              break;
            }
            if (ERRCODE.STATUS & STATUS_GSM_SMS_SEND_ERROR)
            {
              USB_DEBUG_MESSAGE("[DEBUG AT] Ошибка отправки смс", DEBUG_GSM, DEBUG_LEVL_3);
              status = HAL_ERROR;
              break;
            }
            osDelay(1000);
            char send[20] = "\0";
            sprintf(send, "[INFO] %d", timeout_sms);
            USB_DEBUG_MESSAGE(send, DEBUG_GSM, DEBUG_LEVL_3);
          }
        }
      }
      else
      {
        ERRCODE.STATUS |= STATUS_GSM_REG_ERROR;
      }
    }
    osThreadSuspend(ADC_readHandle);
    osThreadSuspend(RS485_dataHandle);
    osThreadSuspend(UART_PARSER_taskHandle);
    HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
    #if BOARD_VERSION == Version3_75 
    HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 1); 
    #endif
    HAL_GPIO_WritePin(EN_3P3V_GPIO_Port, EN_3P3V_Pin, 1);
    osDelay(50);

    USB_DEBUG_MESSAGE("[INFO] Завершаем работу, сохраняем данные", DEBUG_OTHER, DEBUG_LEVL_3);
    Collect_DATA();
    uint8_t send_status = 1;
    if ((ERRCODE.STATUS & STATUS_HTTP_SERVER_COMM_ERROR) || (ERRCODE.STATUS & STATUS_GSM_REG_ERROR))
    {
      send_status = 0; // Отметить как не отправленную
    }

    // Ускорение SPI2
    __HAL_SPI_DISABLE(&hspi2);  // выключить периферию
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    if (HAL_SPI_Init(&hspi2) != HAL_OK)
    {
        Error_Handler();  // или своя обработка ошибки
    }
    __HAL_SPI_ENABLE(&hspi2);   // включить обратно


    // ! перенести в отдельную задачу
    flash_append_record(save_data, send_status);
    osDelay(200);

    HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, 1);
    HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ADC_Pin, 1);
    HAL_GPIO_WritePin(ON_RS_GPIO_Port, ON_RS_Pin, 0); // Не включаем RS по умолчанию
    HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, 0);
    HAL_GPIO_WritePin(EN_3P3V_GPIO_Port, EN_3P3V_Pin, 0);
    HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
    HAL_GPIO_WritePin(ON_DISP_GPIO_Port, ON_DISP_Pin, 0);
    #if BOARD_VERSION == Version3_75 
    HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 0);
    HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 0);
    #elif BOARD_VERSION == Version3_80
    HAL_GPIO_WritePin(ON_OWEN_1_GPIO_Port, ON_OWEN_1_Pin, 0);
    HAL_GPIO_WritePin(ON_OWEN_2_GPIO_Port, ON_OWEN_2_Pin, 0);
    HAL_GPIO_WritePin(ON_OWEN_3_GPIO_Port, ON_OWEN_3_Pin, 0);
    #endif
    osDelay(10);
    USB_DEBUG_MESSAGE("[INFO] Переход в сон", DEBUG_OTHER, DEBUG_LEVL_3);
    Enter_StandbyMode(EEPROM.time_sleep_h, EEPROM.time_sleep_m);
    osDelay(10000);
  }
}



void ADC_read(void *argument)
{
  uint8_t update_counter = 0;
  UNUSED(argument);
  for (;;)
  {

    ADC_data.update_value();
    update_counter++;
    if (update_counter >= 3)
    {
      update_counter = 0;
      if (EEPROM.Mode == 1) xSemaphoreGive(ADC_conv_end_semaphore);
    }
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
    RS485_StartReceive_IT();
    
    for (;;)
    {
        osDelay(500);
        ADC_Voltage_Calculate();
        
        uint8_t msg[] = "Hello RS-485";
        RS485_Transmit_IT(msg, sizeof(msg)-1);
        
        if (EEPROM.Mode == 1) {
            osThreadSuspend(RS485_dataHandle);
        }
    }
}


uint32_t data_read_adc_in = 0;
void Display_I2C(void *argument)
{

  if (Flash_IsCalibEmpty())
  {
    Initial_setup();
  }
  else
  {
    HAL_StatusTypeDef res = Flash_ReadCalib(&Main_data);

  }
  // Проверка на защиту калибровочных данных
  if (!Flash_IsCalibProtected())
  {
        OLED_Clear(0);
        FontSet(font);
        Display_TopBar(selectedMenuItem);
        const char Disable_protect[2][40] = {"Защита снята!", "Protect disable!"};
        OLED_DrawCenteredString(Disable_protect, 30);
        OLED_UpdateScreen();
        osDelay(2000);
  }

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
    xSemaphoreTake(SLEEP_semaphore, 0);
    Keyboard();
    //Keyboard_processing();
    //osDelay(Timer_key_one_press);
  }
}


void Watch_dog_task(void *argument)
{
  UNUSED(argument);
  TickType_t startTick_counter = xTaskGetTickCount();
  TickType_t timeoutTicks = pdMS_TO_TICKS(5*60*1000+10); // 5 минут в тиках (60000 мс)
  for (;;)
  {
    if ((xTaskGetTickCount() - startTick_counter) > timeoutTicks){
      startTick_counter = xTaskGetTickCount(); // Сброс таймера
      PowerUP_counter(); // Учет времени работы
    }
    
    HAL_IWDG_Refresh(&hiwdg);
    osDelay(1000);
    if (xSemaphoreTake(SLEEP_semaphore, 10) == pdTRUE)
    {
      __HAL_TIM_SET_COUNTER(&htim8, 0);
      if (ERRCODE.STATUS & STATUS_VOLTAGE_TOO_LOW) Enter_StandbyMode_NoWakeup();
      if (EEPROM.block == 2)
      {
        osThreadSuspend(ADC_readHandle);
        osThreadSuspend(RS485_dataHandle);
        osThreadSuspend(UART_PARSER_taskHandle);
      }
      if (EEPROM.block == 1) 
      {
        if ((mode_redact == 0) || (mode_redact == 4)) Screen_saver();
        continue;
      }
      if (mode_redact != 0) continue;
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
  //EEPROM.DEBUG_CATEG = 0;
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
  if (EEPROM.USB_mode == USB_FLASH){
    MX_USB_HOST_Init();
  }
  Collect_DATA();
  for (;;)
  {
    
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
    // Если GSM должен быть выключен
    if (EEPROM.Communication == M2M_DISABLE)
    {
      HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
      delay_AT_OK = 0;
      GSM_data.Status = 0;
      osDelay(5000);
      continue;
    }
    // Если GSM должен быть включен
    if (EEPROM.Communication != M2M_DISABLE)
    {
      // Если GSM был выключен
      if (HAL_GPIO_ReadPin(EN_3P8V_GPIO_Port, EN_3P8V_Pin) == 0)
      {
        HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 1); // Включение GSM
        osDelay(100);
        HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 1);
        osDelay(600);
        HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 0);
        osDelay(1000);
      }
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
    // Если модуль связи не готов
    if ((!(GSM_data.Status & GSM_RDY)) && ((EEPROM.DEBUG_Mode != USB_AT_DEBUG) || (EEPROM.USB_mode != USB_DEBUG)))
    {
      delay_AT_OK++;
      if (SendCommandAndParse("AT\r", waitForOKResponse, 2000) == HAL_OK)
      {
        delay_AT_OK = 0;
        USB_DEBUG_MESSAGE("[DEBUG AT] Модуль связи обнаружен, начата установка настроек модуля связи", DEBUG_GSM, DEBUG_LEVL_1);
        if (SendCommandAndParse("AT+CFUN=0\r", waitForOKResponse, 2000) == HAL_OK)
        {
          if (SendCommandAndParse("AT+CFGDUALMODE=1,0\r", waitForOKResponse, 2000) == HAL_OK)
          {
            if (SendCommandAndParse("AT+CFGRATPRIO=2\r", waitForOKResponse, 2000) == HAL_OK)
            {
              if (SendCommandAndParse("AT+CFUN=1\r", waitForOKResponse, 2000) == HAL_OK)
              {
                if (SendCommandAndParse("AT+CSCON=0\r", waitForOKResponse, 2000) == HAL_OK)
                {
                  if (SendCommandAndParse("AT&W\r", waitForOKResponse, 2000) == HAL_OK)
                  {
                    GSM_data.Status |= GSM_RDY;
                    USB_DEBUG_MESSAGE("[DEBUG AT] Модуль связи настроен", DEBUG_GSM, DEBUG_LEVL_1);
                  }
                }
              }
            }
          }
        }
      }
      else
      {
        USB_DEBUG_MESSAGE("[ERROR AT] Модуль связи не отвечает, ждем", DEBUG_GSM, DEBUG_LEVL_1);
      }
      if (delay_AT_OK > 2){
        USB_DEBUG_MESSAGE("[ERROR AT] Таймаут 30с превышен, модуль связи не найден", DEBUG_GSM, DEBUG_LEVL_1);
        ERRCODE.STATUS |= STATUS_UART_NO_RESPONSE;
      }
      continue;
    }

    // Если модуль связи готов
    // ! перепроверить
    if (GSM_data.Status & NETWORK_REGISTERED_SET_HTTP){
      GSM_data.Status &=~ NETWORK_REGISTERED_SET_HTTP;
      USB_DEBUG_MESSAGE("[DEBUG AT] Настройка GPRS", DEBUG_GSM, DEBUG_LEVL_3);
      SendCommandAndParse("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r", waitForOKResponse, 1000); 
      SendCommandAndParse("AT+CGACT=1,1\r", waitForOKResponse, 60000);
      SendCommandAndParse("AT+CDNSCFG=\"8.8.8.8\",\"77.88.8.8\"\r", waitForOKResponse, 1000);
    }
    if ((EEPROM.DEBUG_Mode != USB_AT_DEBUG) || (EEPROM.USB_mode != USB_DEBUG))
    {
      USB_DEBUG_MESSAGE("[DEBUG AT] Начато обновление статуса GSM", DEBUG_GSM, DEBUG_LEVL_2);
      SendCommandAndParse("AT+CPIN?\r", waitForCPINResponse, 1000);
      SendCommandAndParse("AT+CSQ\r", waitForCSQResponse, 1000);
      SendCommandAndParse("AT+CEREG?\r", waitForCEREGResponse, 1000);
      SendCommandAndParse("AT+COPS?\r", waitForCOPSResponse, 1000);
      GSM_data.update_value();
      if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
    }

    if (GSM_data.Status & SMS_SEND)
    {
      USB_DEBUG_MESSAGE("[DEBUG AT] Начата отправка СМС", DEBUG_GSM, DEBUG_LEVL_2);
      // флаг того что нужно отправить смс
      GSM_data.Status &= ~SMS_SEND;
      Collect_DATA();
      if (sendSMS() == HAL_OK)
      {
        USB_DEBUG_MESSAGE("[DEBUG AT] СМС отправлена", DEBUG_GSM, DEBUG_LEVL_2);
        if (EEPROM.Mode == 0) strcpy(GSM_data.GSM_sms_status, "OK");
        if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
        GSM_data.Status |= SMS_SEND_Successfully;
      }
      else
      {
        USB_DEBUG_MESSAGE("[ERROR AT] СМС не отправлена", DEBUG_GSM, DEBUG_LEVL_1);
        if (EEPROM.Mode == 0) strcpy(GSM_data.GSM_sms_status, "ERR");
        if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
        ERRCODE.STATUS |= STATUS_GSM_SMS_SEND_ERROR;
      }
    }

    if (GSM_data.Status & HTTP_SEND)
    {
      GSM_data.Status &= ~HTTP_SEND;
      Collect_DATA();
      USB_DEBUG_MESSAGE("[DEBUG AT] Начало HTTP GET", DEBUG_GSM, DEBUG_LEVL_2);
      if (sendHTTP() == HAL_OK)
      {
        USB_DEBUG_MESSAGE("[DEBUG AT] Успешно HTTP GET", DEBUG_GSM, DEBUG_LEVL_2);
        GSM_data.Status |= HTTP_SEND_Successfully;
        if (EEPROM.Mode == 0)
          strcpy(GSM_data.GSM_site_status, "OK");
        if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
        ERRCODE.STATUS &= ~STATUS_HTTP_SERVER_COMM_ERROR;
      }
      else
      {
        USB_DEBUG_MESSAGE("[ERROR AT] Ошибка HTTP GET", DEBUG_GSM, DEBUG_LEVL_1);
        if (EEPROM.Mode == 0)
          strcpy(GSM_data.GSM_site_status, "ERR");
        if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
        ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
      }
    }
    if (GSM_data.Status & HTTP_READ)
    {
      USB_DEBUG_MESSAGE("[DEBUG AT] Начало HTTP READ", DEBUG_GSM, DEBUG_LEVL_1);
      // флаг того что нужно отправить HTTP
      GSM_data.Status &= ~HTTP_READ;
      SETTINGS_REQUEST_DATA();
      if (READ_Settings_sendHTTP() == HAL_OK)
      {
        USB_DEBUG_MESSAGE("[ERROR AT] Успешно HTTP READ", DEBUG_GSM, DEBUG_LEVL_1);
        GSM_data.Status |= HTTP_READ_Successfully;
        if (EEPROM.Mode == 0){
          strcpy(GSM_data.GSM_site_read_status, "OK");
          xSemaphoreGive(Display_semaphore);
          ERRCODE.STATUS &= ~STATUS_HTTP_SERVER_COMM_ERROR;
        }
      }
      else
      {
        USB_DEBUG_MESSAGE("[ERROR AT] Ошибка HTTP READ", DEBUG_GSM, DEBUG_LEVL_1);
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

