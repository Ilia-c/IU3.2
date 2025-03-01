#include "keyboard.h"

// ----- Внешние объекты -----
extern xSemaphoreHandle Keyboard_semapfore;
extern xSemaphoreHandle Display_semaphore;
extern char Keyboard_press_code;  // Итоговый код, который использует Display
char Prot_Keyboard_press_code;    // Сырые данные при сканировании

extern TIM_HandleTypeDef htim6;
extern const uint16_t Timer_key_press;
extern const uint16_t Timer_key_press_fast;
uint8_t Timer = 0;

extern int mode_redact;  // Режим редактирования
static char last_key = 0xFF;  // Запоминаем последнюю нажатую клавишу

// Карта клавиш матрицы (4x4)
const char keyMap[4][4] = {
    {'1', '2', '3', 'U'},
    {'4', '5', '6', 'D'},
    {'7', '8', '9', 'L'},
    {'0', 'P', 'O', 'R'}
};

// Определяем массивы портов и пинов
GPIO_TypeDef* COL_PORTS[4] = {COL_B1_GPIO_Port, COL_B2_GPIO_Port, COL_B3_GPIO_Port, COL_B4_GPIO_Port};
uint16_t COL_PINS[4] = {COL_B1_Pin, COL_B2_Pin, COL_B3_Pin, COL_B4_Pin};

GPIO_TypeDef* STR_PORTS[4] = {STR_B1_GPIO_Port, STR_B2_GPIO_Port, STR_B3_GPIO_Port, STR_B4_GPIO_Port};
uint16_t STR_PINS[4] = {STR_B1_Pin, STR_B2_Pin, STR_B3_Pin, STR_B4_Pin};

void ret_keyboard(void)
{
    // Восстанавливаем линии столбцов и включаем прерывания
    for (int i = 0; i < 4; i++)
    {
        HAL_GPIO_WritePin(COL_PORTS[i], COL_PINS[i], GPIO_PIN_SET);
    }

    for (int i = 0; i < 4; i++)
    {
        __HAL_GPIO_EXTI_CLEAR_IT(STR_PINS[i]);
    }

    HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, GPIO_PIN_SET);
    osDelay(20);
    __HAL_GPIO_EXTI_CLEAR_IT(STR_B1_Pin);
    __HAL_GPIO_EXTI_CLEAR_IT(STR_B2_Pin);
    __HAL_GPIO_EXTI_CLEAR_IT(STR_B3_Pin);
    __HAL_GPIO_EXTI_CLEAR_IT(STR_B4_Pin);
    HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
    HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void Keyboard(void)
{
    Keyboard_press_code = ScanKeypad();
    if ((mode_redact == 0) && ((Keyboard_press_code == 'L') || (Keyboard_press_code == 'R')) && ((Keyboard_press_code != 0xFF))){
        HAL_TIM_Base_Stop_IT(&htim6);
        TIM6->SR &= ~TIM_SR_UIF;
        TIM6->CNT = 0;
        ret_keyboard();
        xSemaphoreGive(Display_semaphore);
        return;
    }
    
    if (Timer == 1){
        Timer = 0;
        // Если клавиша не нажата — останавливаем таймер
        if ((Keyboard_press_code == 0xFF))
        {
            HAL_TIM_Base_Stop_IT(&htim6);
            TIM6->SR &= ~TIM_SR_UIF;
            TIM6->CNT = 0;
            ret_keyboard();
            return;
        }

        xSemaphoreGive(Display_semaphore);
        ret_keyboard();
    }
    else
    {

        // Остановка таймера, если клавиша не нажата
        HAL_TIM_Base_Stop_IT(&htim6);
        TIM6->SR &= ~TIM_SR_UIF;
        TIM6->CNT = 0;
        __HAL_TIM_SET_AUTORELOAD(&htim6, Timer_key_press);
        //TIM6->EGR |= TIM_EGR_UG;

        if (Keyboard_press_code != 0xFF)
        {
            xSemaphoreGive(Display_semaphore);
            // Запуск таймера автоповтора
            HAL_TIM_Base_Start_IT(&htim6);
            //__HAL_TIM_SET_AUTORELOAD(&htim6, Timer_key_press_fast);
        }

        ret_keyboard();
        last_key = Keyboard_press_code;
    }
}

char ScanKeypad(void)
{
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_IRQn);
    char detected_key = 0xFF; // Переменная для хранения нажатой клавиши

    // ----- Перед началом сканирования выключаем все колонки -----
    for (int i = 0; i < 4; i++)
    {
        HAL_GPIO_WritePin(COL_PORTS[i], COL_PINS[i], GPIO_PIN_RESET);
    }

    osDelay(1); // Небольшая задержка

    // ----- Сканирование клавиатуры -----
    for (int col = 0; col < 4; col++)
    {
        // Включаем текущий столбец
        HAL_GPIO_WritePin(COL_PORTS[col], COL_PINS[col], GPIO_PIN_SET);
        osDelay(1);
        for (int row = 0; row < 4; row++)
        {
            if (HAL_GPIO_ReadPin(STR_PORTS[row], STR_PINS[row]) == GPIO_PIN_SET)
            {
                detected_key = keyMap[col][row];
            }
        }
        HAL_GPIO_WritePin(COL_PORTS[col], COL_PINS[col], GPIO_PIN_RESET);
    }

    return detected_key;
}

void HAL_TIM6_Callback(void)
{
    /*
    Keyboard_press_code = ScanKeypad();
    
    // Если клавиша не нажата — останавливаем таймер
    if (Keyboard_press_code == 0xFF)
    {
        HAL_TIM_Base_Stop_IT(&htim6);
        TIM6->SR &= ~TIM_SR_UIF;
        TIM6->CNT = 0;
        ret_keyboard();
        return;
    }
    */
    // Отправляем сигнал для обновления дисплея
    __HAL_TIM_SET_AUTORELOAD(&htim6, Timer_key_press_fast);
    Timer = 1;
    static portBASE_TYPE xTaskWoken;
    xSemaphoreGiveFromISR(Keyboard_semapfore, &xTaskWoken);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if ((GPIO_Pin == STR_B1_Pin) || (GPIO_Pin == STR_B2_Pin) ||
        (GPIO_Pin == STR_B3_Pin) || (GPIO_Pin == STR_B4_Pin))
    {
        static portBASE_TYPE xTaskWoken;
        xSemaphoreGiveFromISR(Keyboard_semapfore, &xTaskWoken);
    }
}
