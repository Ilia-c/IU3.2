#include "keyboard.h"

// ----- Внешние объекты, объявленные в других файлах -----
extern xSemaphoreHandle Keyboard_semapfore;
extern xSemaphoreHandle Display_semaphore;
extern char Keyboard_press_code;  // итоговый код, который использует Display
char Prot_Keyboard_press_code;      // сырые данные, полученные при сканировании

extern TIM_HandleTypeDef htim6;
extern const uint16_t Timer_key_press;       // начальная задержка (например, 800 мс)
extern const uint16_t Timer_key_press_fast;  // интервал автоповтора (100 мс)

extern int mode_redact;  // режим редактирования

// ----- Автомат состояний клавиши -----
typedef enum {
    STATE_IDLE,      // Нет нажатия
    STATE_PRESSED,   // Клавиша нажата, ждем либо отпускания (короткое нажатие), либо истечения задержки
    STATE_AUTOREPEAT // Автоповтор запущен
} key_state_t;

static key_state_t key_state = STATE_IDLE;
static char last_key = 0xFF;  // Запоминаем нажатую клавишу

// Упрощённая задержка для ScanKeypad (не используется в ISR)
#define delay osDelay(1)

// Карта клавиш матрицы (4х4)
const char keyMap[4][4] = {
    {'1', '2', '3', 'U'},
    {'4', '5', '6', 'D'},
    {'7', '8', '9', 'L'},
    {'0', 'P', 'O', 'R'}
};

/*
  ret_keyboard() вызывается после сканирования матрицы.
  
  Если никакая клавиша не нажата (Prot_Keyboard_press_code == 0xFF):
    – Если мы были в STATE_PRESSED (короткое нажатие) – выдаём событие.
    – В любом случае останавливаем таймер и сбрасываем автомат в STATE_IDLE.
    
  Если клавиша нажата:
    – Если автомат в STATE_IDLE, фиксируем нажатие (last_key = текущий код).
        При этом, если mode_redact==1 и нажата клавиша 'R' или 'L' – выдаём событие сразу
        (сброс автоповтора), иначе переходим в STATE_PRESSED и запускаем таймер с задержкой Timer_key_press.
    – Если автомат уже в STATE_PRESSED или STATE_AUTOREPEAT, и обнаружена другая клавиша (current_key != last_key),
        то сбрасываем автоповтор и начинаем обработку нового нажатия (аналогично предыдущему пункту).
*/
void ret_keyboard(void)
{
    char current_key = Prot_Keyboard_press_code;

    if (current_key == 0xFF)
    {
        // Клавиша отпущена.
        if (key_state == STATE_PRESSED)
        {
            // Короткое нажатие – выдаём событие один раз.
            Keyboard_press_code = last_key;
            xSemaphoreGive(Display_semaphore);
        }
        // Независимо от состояния, останавливаем таймер и сбрасываем автомат.
        HAL_TIM_Base_Stop_IT(&htim6);
        key_state = STATE_IDLE;
        last_key = 0xFF;
    }
    else
    {
        // Клавиша нажата.
        if (key_state == STATE_IDLE)
        {
            // Новое нажатие.
            // Если в режиме редактирования (mode_redact==1) и нажата клавиша вправо или влево ('R' или 'L'),
            // сразу выдаём событие и не запускаем автоповтор.
            if (mode_redact == 0 && (current_key == 'R' || current_key == 'L'))
            {
                Keyboard_press_code = current_key;
                xSemaphoreGive(Display_semaphore);
                key_state = STATE_IDLE;
                last_key = 0xFF;
                HAL_TIM_Base_Stop_IT(&htim6);
            }
            else
            {
                last_key = current_key;
                key_state = STATE_PRESSED;
                __HAL_TIM_SET_AUTORELOAD(&htim6, Timer_key_press);
                HAL_TIM_Base_Start_IT(&htim6);
            }
        }
        else
        {
            // Если уже в STATE_PRESSED или STATE_AUTOREPEAT.
            // Если обнаружена другая клавиша, сбрасываем автоповтор и начинаем обработку нового нажатия.
            if (current_key != last_key)
            {
                HAL_TIM_Base_Stop_IT(&htim6);
                if (mode_redact == 1 && (current_key == 'R' || current_key == 'L'))
                {
                    Keyboard_press_code = current_key;
                    xSemaphoreGive(Display_semaphore);
                    key_state = STATE_IDLE;
                    last_key = 0xFF;
                }
                else
                {
                    last_key = current_key;
                    key_state = STATE_PRESSED;
                    __HAL_TIM_SET_AUTORELOAD(&htim6, Timer_key_press);
                    HAL_TIM_Base_Start_IT(&htim6);
                }
            }
            // Если та же самая клавиша, ничего не меняем.
        }
    }

    // Восстанавливаем линии столбцов и прерывания.
    HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, GPIO_PIN_SET);
    __HAL_GPIO_EXTI_CLEAR_IT(STR_B1_Pin);
    __HAL_GPIO_EXTI_CLEAR_IT(STR_B2_Pin);
    __HAL_GPIO_EXTI_CLEAR_IT(STR_B3_Pin);
    __HAL_GPIO_EXTI_CLEAR_IT(STR_B4_Pin);
    HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
    HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/*
  HAL_TIM6_Callback() – вызывается по прерыванию таймера.
  
  Здесь выполняется быстрый опрос клавиатуры (без задержек) для обновления Prot_Keyboard_press_code.
  Если в состоянии STATE_PRESSED (начальная задержка истекла) и клавиша всё ещё нажата,
  происходит переход в STATE_AUTOREPEAT: генерируется первое событие автоповтора, а период
  таймера переключается на быстрый (100 мс).
  
  В состоянии STATE_AUTOREPEAT каждое срабатывание таймера генерирует событие автоповтора.
  
  Если клавиша отпущена, производится сброс автомата в STATE_IDLE.
*/
void HAL_TIM6_Callback(void)
{
    char key = 0xFF;

    /* Быстрый опрос клавиатуры без задержек */

    // --- Колонка 1 ---
    HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, GPIO_PIN_RESET);
    if (HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == GPIO_PIN_SET)
        key = keyMap[0][0];
    else if (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == GPIO_PIN_SET)
        key = keyMap[0][1];
    else if (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == GPIO_PIN_SET)
        key = keyMap[0][2];
    else if (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == GPIO_PIN_SET)
        key = keyMap[0][3];

    // Если не найдено, проверим колонку 2.
    if (key == 0xFF)
    {
        HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, GPIO_PIN_RESET);
        if (HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == GPIO_PIN_SET)
            key = keyMap[1][0];
        else if (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == GPIO_PIN_SET)
            key = keyMap[1][1];
        else if (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == GPIO_PIN_SET)
            key = keyMap[1][2];
        else if (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == GPIO_PIN_SET)
            key = keyMap[1][3];
    }
    // Если всё ещё не найдено, проверяем колонку 3.
    if (key == 0xFF)
    {
        HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, GPIO_PIN_RESET);
        if (HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == GPIO_PIN_SET)
            key = keyMap[2][0];
        else if (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == GPIO_PIN_SET)
            key = keyMap[2][1];
        else if (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == GPIO_PIN_SET)
            key = keyMap[2][2];
        else if (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == GPIO_PIN_SET)
            key = keyMap[2][3];
    }
    // Если всё ещё не найдено, проверяем колонку 4.
    if (key == 0xFF)
    {
        HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, GPIO_PIN_SET);
        if (HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == GPIO_PIN_SET)
            key = keyMap[3][0];
        else if (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == GPIO_PIN_SET)
            key = keyMap[3][1];
        else if (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == GPIO_PIN_SET)
            key = keyMap[3][2];
        else if (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == GPIO_PIN_SET)
            key = keyMap[3][3];
    }
    // Восстанавливаем все колонки в состояние 1.
    HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, GPIO_PIN_SET);

    // Обновляем глобальную переменную.
    Prot_Keyboard_press_code = key;

    // Далее логика автомата.
    if (key_state == STATE_PRESSED)
    {
        if (Prot_Keyboard_press_code != 0xFF)
        {
            // Начальная задержка истекла – переходим в автоповтор.
            key_state = STATE_AUTOREPEAT;
            Keyboard_press_code = last_key;
            xSemaphoreGiveFromISR(Display_semaphore, NULL);
            __HAL_TIM_SET_AUTORELOAD(&htim6, Timer_key_press_fast);
        }
        else
        {
            HAL_TIM_Base_Stop_IT(&htim6);
            key_state = STATE_IDLE;
            last_key = 0xFF;
        }
    }
    else if (key_state == STATE_AUTOREPEAT)
    {
        if (Prot_Keyboard_press_code != 0xFF)
        {
            Keyboard_press_code = last_key;
            xSemaphoreGiveFromISR(Display_semaphore, NULL);
        }
        else
        {
            HAL_TIM_Base_Stop_IT(&htim6);
            key_state = STATE_IDLE;
            last_key = 0xFF;
        }
    }
}

/*
  ScanKeypad() – функция сканирования матрицы клавиатуры.
  По окончании сканирования вызывается ret_keyboard() для обновления автомата.
  (Функция вызывается по семафору из внешних прерываний или таймера.)
*/
void ScanKeypad(void)
{
    HAL_TIM_Base_Stop_IT(&htim6);
    TIM6->SR &= ~TIM_SR_UIF;  // Сброс флага таймера

    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_IRQn);

    Prot_Keyboard_press_code = 0xFF;

    // ----- Сканирование: колонка 1 -----
    HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, GPIO_PIN_RESET);
    delay;
    if (HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[0][0];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[0][1];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[0][2];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[0][3];
        ret_keyboard();
        return;
    }

    // ----- Колонка 2 -----
    HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, GPIO_PIN_SET);
    delay;
    if (HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[1][0];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[1][1];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[1][2];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[1][3];
        ret_keyboard();
        return;
    }
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, GPIO_PIN_RESET);

    // ----- Колонка 3 -----
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, GPIO_PIN_SET);
    delay;
    if (HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[2][0];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[2][1];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[2][2];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[2][3];
        ret_keyboard();
        return;
    }
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, GPIO_PIN_RESET);

    // ----- Колонка 4 -----
    HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, GPIO_PIN_SET);
    delay;
    if (HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[3][0];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[3][1];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[3][2];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == GPIO_PIN_SET)
    {
        Prot_Keyboard_press_code = keyMap[3][3];
        ret_keyboard();
        return;
    }

    Prot_Keyboard_press_code = 0xFF;
    ret_keyboard();
}

/*
  HAL_GPIO_EXTI_Callback() – обработчик внешних прерываний по линиям строк (антидребезг).
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if ((GPIO_Pin == STR_B1_Pin) || (GPIO_Pin == STR_B2_Pin) ||
        (GPIO_Pin == STR_B3_Pin) || (GPIO_Pin == STR_B4_Pin))
    {
        uint8_t B1 = HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin);
        uint8_t B2 = HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin);
        uint8_t B3 = HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin);
        uint8_t B4 = HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin);
        osDelay(1);
        if ((HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == B1) &&
            (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == B2) &&
            (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == B3) &&
            (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == B4))
        {
            static portBASE_TYPE xTaskWoken;
            xSemaphoreGiveFromISR(Keyboard_semapfore, &xTaskWoken);
        }
    }
}
