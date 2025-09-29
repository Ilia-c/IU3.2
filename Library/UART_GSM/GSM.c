/* GSM.c */
#include "GSM.h"

extern UART_HandleTypeDef huart4;
extern xSemaphoreHandle UART_PARSER_semaphore;
extern xSemaphoreHandle UART_PARSER_MQTT_semaphore;
extern xSemaphoreHandle USB_COM_SEND_semaphore;

extern USBD_HandleTypeDef hUsbDeviceFS; 

/* Однобайтовый приём */
uint8_t gsmRxChar;

/* Двойное буферирование: буферы размещаются в области ОЗУ ".ram2" */
static char buffer1[CMD_BUFFER_SIZE] __attribute__((section(".ram2"))) = {0};
static char buffer2[CMD_BUFFER_SIZE] __attribute__((section(".ram2"))) = {0};

/* Указатели на активный и обрабатываемый (парсера) буферы */
static char *activeBuffer = buffer1;
char *parseBuffer = buffer2;
static uint16_t activeIndex = 0;

/* Одиночный таймер, который сбрасывается при каждом принятом символе */
static TimerHandle_t gsmTimer = NULL;


/* Инициализация модуля GSM: создание таймера и запуск приёма по UART */
void GSM_Init(void)
{
    gsmTimer = xTimerCreate("GSM_Timer",
                            pdMS_TO_TICKS(RX_TIMEOUT_MS),
                            pdFALSE,       // однократный таймер (one-shot)
                            NULL,
                            GSM_TimerCallback);
    if (gsmTimer == NULL) {
        // Обработка ошибки создания таймера (например, логирование или аварийный сбой)
    }
    HAL_UART_Receive_IT(&huart4, &gsmRxChar, 1);
}

/* Колбэк таймера: вызывается, если в течение RX_TIMEOUT_MS не получено новых символов */
void GSM_TimerCallback(TimerHandle_t xTimer)
{
    if (activeIndex > 0)
    {
        /* Завершаем строку в активном буфере */
        activeBuffer[activeIndex++] = '\0';
        
        /* Меняем местами буферы: накопленные данные будут обрабатываться через parseBuffer */
        char *temp = activeBuffer;
        activeBuffer = parseBuffer;
        parseBuffer = temp;
        while (EEPROM.USB_mode == USB_DEBUG)
        {
            if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) break;
            if (USB_TERMINAL_STATUS == TERMINAL_DISABLE) break;
            
            if (EEPROM.DEBUG_Mode == USB_SNIFFING)
            {
                if (EEPROM.DEBUG_CATEG & AT_COMMANDS) collect_message(parseBuffer);
            }
            if (EEPROM.DEBUG_Mode == USB_AT_DEBUG) collect_message(parseBuffer);
            break;
        }
        if (GSM_data.Status & MQTT_SEARCH)
        {
            // Если в режиме MQTT, то данные уже помечены в колбэке приёма
            // просто сигнализируем задаче парсера о готовности данных
            GSM_data.Status &= ~MQTT_SEARCH;
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(UART_PARSER_MQTT_semaphore, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }

        /* Сигнализируем задаче парсера о готовности данных */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(UART_PARSER_semaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
    /* Сбрасываем накопление в активном буфере для нового приёма */
    activeIndex = 0;
    memset(activeBuffer, 0, CMD_BUFFER_SIZE);
}

static uint8_t sig_idx = 0;
static char sig[8]; // степень двойки — удобнее

static inline uint8_t match5(const char *s){
    // сравниваем 5 последних байт с "+MSUB"
    uint8_t i = (sig_idx + 8 - 5) & 7;
    return (sig[i]=='+' && sig[i+1]=='M' && sig[i+2]=='S' && sig[i+3]=='U' && sig[i+4]=='B');
}
static inline uint8_t match6_closed(void){
    uint8_t i = (sig_idx + 8 - 6) & 7;
    return (sig[i+0]=='C' && sig[i+1]=='L' && sig[i+2]=='O' && sig[i+3]=='S' && sig[i+4]=='E' && sig[i+5]=='D');
}

/* Колбэк, вызываемый по завершении приёма одного байта по UART4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)
    {
        HAL_UART_Receive_IT(&huart4, &gsmRxChar, 1);
        // Если MQTT, то анализируем все строки и ищем разрыв соединения или данные с топика
        if (EEPROM.Communication_http_mqtt == MQTT)
        {
            sig[sig_idx] = gsmRxChar;
            sig_idx++;
            if (sig_idx >= 8) sig_idx = 0;
            if (match5("+MSUB"))
            {
                // Установить флаг обработки принятых данных по MQTT - в отдельную задачу
                activeBuffer[activeIndex] = '+';
                activeBuffer[activeIndex++] = 'M';
                activeBuffer[activeIndex++] = 'S';
                activeBuffer[activeIndex++] = 'U';
                activeBuffer[activeIndex++] = 'B';

                GSM_data.Status |= DATA_READ;
                GSM_data.Status |= MQTT_SEARCH;
            }
            if (match6_closed())
            {
                // Соединение пропало
                // ! Нужно сбросить машину состояний g_mqtt
                GSM_data.Status &= ~MQTT_SEARCH;
            }
        }

        if ((!(GSM_data.Status & DATA_READ)) && (EEPROM.DEBUG_Mode != USB_AT_DEBUG) || (GSM_data.Status & MQTT_SEARCH))
        {
            return;
        }
        /* Если есть место в активном буфере – сохраняем принятый символ */
        if (activeIndex < CMD_BUFFER_SIZE - 1)
        {
            activeBuffer[activeIndex] = gsmRxChar;
            activeIndex++;
        }
        else
        {
            activeIndex = 0;
            activeBuffer[activeIndex] = gsmRxChar;
        }
        
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (gsmTimer != NULL)
        {
            xTimerResetFromISR(gsmTimer, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        
    }
    if (huart->Instance == UART5) {
        RS485_RxCpltCallback(rs485_rx_byte);
        RS485_StartReceive_IT();
    }
}

/* Функция, вызываемая перед отправкой новой команды – очищает накопленный буфер */
void SendSomeCommandAndSetFlag(void)
{
    activeIndex = 0;
    memset(activeBuffer, 0, CMD_BUFFER_SIZE);
    
}

/* Функция обновления данных GSM */
void Update_Data(void)
{
    if (!(GSM_data.Status & GSM_RDY))
    {
        USB_DEBUG_MESSAGE("[WARNING AT] Сброс состояния модуля", DEBUG_GSM, DEBUG_LEVL_2);
        /* Модуль неактивен: сбрасываем все состояния */
        GSM_data.Status &= ~DATA_READ;
        GSM_data.GSM_Signal_Level = 99;
        GSM_data.GSM_Signal_Level_3 = -2;
        GSM_data.GSM_status_char = (char *)STATUS_CHAR[1];            // "ND" – статус не определён
        GSM_data.GSM_SIMCARD_char = (char *)SIM_STATUS[2];            // SIM не установлена
        GSM_data.Modem_mode = MODEM_STATUS[2];                        // "UNKNOWN" – режим неизвестен
        GSM_data.GSM_status_ready_char = (char *)GSM_READY_STATUS[1]; // "NRDY" – GSM не готов
        GSM_data.GSM_status_reg_char = (char *)GSM_REG_STATUS[2];     // "UNKNOWN" – регистрация неизвестна
        GSM_data.GSM_region_char = (char *)Countries[0].iso;          // Например, "UNDEF"
        GSM_data.GSM_operator_char = (char *)GSM_Operators[0].name;   // Например, "Undefine"
        strcpy(GSM_data.GSM_signal_lvl_char, "99");                   // Текстовое представление
        strcpy(GSM_data.GSM_gprs_on_char, (char *)GPRS_STATUS[1]);    // "DISCONNECTED"
        InitMenus();
        return;
    }

    GSM_data.GSM_status_ready_char = (char *)GSM_READY_STATUS[0];   // "RDY"
    GSM_data.GSM_status_char = (char *)STATUS_CHAR[0];              // "OK"
    if (!(GSM_data.Status & SIM_PRESENT))
    {
        USB_DEBUG_MESSAGE("[WARNING AT] Сим не найдена!", DEBUG_GSM, DEBUG_LEVL_2);
        ERRCODE.STATUS |= STATUS_GSM_NO_SIM; // Устанавливаем бит ошибки "SIM не установлена"
        /* SIM-карта не обнаружена: сброс состояний */
        GSM_data.Status = GSM_RDY;
        GSM_data.GSM_Signal_Level = 99;
        GSM_data.GSM_Signal_Level_3 = -2;
        GSM_data.GSM_SIMCARD_char = (char *)SIM_STATUS[1];        // SIM не установлена
        GSM_data.Modem_mode = MODEM_STATUS[2];                    // "UNKNOWN" – исправлено (присвоение)
        GSM_data.GSM_status_reg_char = (char *)GSM_REG_STATUS[2]; // "UNKNOWN"
        GSM_data.GSM_region_char = (char *)Countries[0].iso;
        GSM_data.GSM_operator_char = (char *)GSM_Operators[0].name;
        strcpy(GSM_data.GSM_signal_lvl_char, "99");
        strcpy(GSM_data.GSM_gprs_on_char, (char *)GPRS_STATUS[1]);
        InitMenus();
        return;
    }
    ERRCODE.STATUS &= ~STATUS_GSM_NO_SIM;
    USB_DEBUG_MESSAGE("[DEBUG AT] Сим карта установлена", DEBUG_GSM, DEBUG_LEVL_3);
    GSM_data.GSM_SIMCARD_char = (char *)SIM_STATUS[0]; // "PRESENT"
    if ((GSM_data.GSM_Signal_Level < 0) || (GSM_data.GSM_Signal_Level >= 99))
    {
        /* Неприемлемый уровень сигнала – сброс состояний */
        USB_DEBUG_MESSAGE("[DEBUG AT] Слабый сигнал", DEBUG_GSM, DEBUG_LEVL_2);
        ERRCODE.STATUS |= STATUS_GSM_SIGNEL_ERROR;
        GSM_data.Status = SIM_PRESENT | GSM_RDY;
        GSM_data.GSM_Signal_Level = 99;
        GSM_data.GSM_Signal_Level_3 = -1;
        GSM_data.Modem_mode = MODEM_STATUS[2];                       // "UNKNOWN" – исправлено (присвоение)
        GSM_data.GSM_status_reg_char = (char *)GSM_REG_STATUS[2];
        GSM_data.GSM_region_char = (char *)Countries[0].iso;
        GSM_data.GSM_operator_char = (char *)GSM_Operators[0].name;
        strcpy(GSM_data.GSM_signal_lvl_char, "99");
        strcpy(GSM_data.GSM_gprs_on_char, (char *)GPRS_STATUS[1]);
        InitMenus();
        return;
    }
    ERRCODE.STATUS &= ~STATUS_GSM_SIGNEL_ERROR;
    USB_DEBUG_MESSAGE("[DEBUG AT] Уровень сигнала приемлемый", DEBUG_GSM, DEBUG_LEVL_3);
    GSM_data.Status |= SIGNAL_PRESENT;
    sprintf(GSM_data.GSM_signal_lvl_char, "%02d", GSM_data.GSM_Signal_Level);
    sprintf(GSM_data.GSM_err_lvl_char, "%02d", GSM_data.GSM_Signal_Errors);

    if (strcmp(GSM_data.Modem_mode, MODEM_STATUS[2]) == 0)
    {
        /* Если не режим GSM */
        USB_DEBUG_MESSAGE("[WARNING AT] Нет регистрации модуля связи", DEBUG_GSM, DEBUG_LEVL_2);
        GSM_data.Status = SIM_PRESENT | GSM_RDY | SIGNAL_PRESENT;
        GSM_data.GSM_status_reg_char = (char *)GSM_REG_STATUS[1]; // Нет регистрации
        GSM_data.GSM_region_char = (char *)Countries[0].iso;
        GSM_data.GSM_operator_char = (char *)GSM_Operators[0].name;
        GSM_data.GSM_gprs_on_char = (char *)GPRS_STATUS[1];
        InitMenus();
        return;
    }
    USB_DEBUG_MESSAGE("[DEBUG AT] Моудль связи зарегистрирован", DEBUG_GSM, DEBUG_LEVL_3);
    GSM_data.GSM_status_reg_char = (char *)GSM_REG_STATUS[0];
    if (!(GSM_data.Status & NETWORK_REGISTERED)) GSM_data.Status |= NETWORK_REGISTERED_SET_HTTP;
    GSM_data.Status |= NETWORK_REGISTERED;
    if (determineRegionAndOperator() == 0){
        GSM_data.Status = SIM_PRESENT | GSM_RDY | SIGNAL_PRESENT;
        GSM_data.GSM_region_char = (char *)Countries[0].iso;
        GSM_data.GSM_operator_char = (char *)GSM_Operators[0].name;
        GSM_data.GSM_gprs_on_char = (char *)GPRS_STATUS[1];
        InitMenus();
        return;
    }
    
    /* Определение текстового представления уровня сигнала */
    if (GSM_data.GSM_Signal_Level < 5)
        GSM_data.GSM_Signal_Level_3 = 0;
    else if (GSM_data.GSM_Signal_Level < 15)
        GSM_data.GSM_Signal_Level_3 = 1;
    else if (GSM_data.GSM_Signal_Level < 25)
        GSM_data.GSM_Signal_Level_3 = 2;
    else
        GSM_data.GSM_Signal_Level_3 = 3;

    InitMenus();
}

/* Функция определения региона и оператора по коду оператора */
int determineRegionAndOperator(void)
{
    uint32_t opCode = GSM_data.Operator_code; // Например, 25001 или 310410
    uint16_t mcc = 0;
    
    /* Определяем MCC (код региона) в зависимости от количества цифр в opCode */
    if (opCode >= 100000) {
        mcc = opCode / 1000;
    } else if (opCode >= 10000) {
        mcc = opCode / 100;
    } else {
        mcc = opCode;
    }
    
    int regionFound = 0;
    const char *regionIso = NULL;
    int countriesCount = CountriesCount;
    for (int i = 0; i < countriesCount; i++) {
        if (Countries[i].mcc == mcc) {
            regionFound = 1;
            regionIso = Countries[i].iso;
            break;
        }
    }
    
    /* Если регион не найден, возвращаем ошибку */
    if (!regionFound) {
        return 0;
    }
    
    int operatorFound = 0;
    const char *operatorName = NULL;
    int operatorsCount = GSM_OperatorsCount;
    for (int j = 0; j < operatorsCount; j++) {
        if (GSM_Operators[j].code == opCode) {
            operatorFound = 1;
            operatorName = GSM_Operators[j].name;
            break;
        }
    }
    
    /* Если оператор не найден, используем значение по умолчанию */
    if (!operatorFound) {
        operatorName = GSM_Operators[0].name;
    }
    
    /* Записываем найденные данные в GSM_data */
    GSM_data.GSM_region_char   = (char *)regionIso;
    GSM_data.GSM_operator_char = (char *)operatorName;
    return 1;
}




