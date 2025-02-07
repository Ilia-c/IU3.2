#include "keyboard.h"
extern xSemaphoreHandle Keyboard_semapfore;

extern xSemaphoreHandle Display_semaphore;
extern char Keyboard_press_code;
char Prot_Keyboard_press_code;
// Определение пинов для строк

extern xSemaphoreHandle TIM6_semaphore_100us;
extern TIM_HandleTypeDef htim6;
extern const uint16_t Timer_key_press;

extern int mode_redact;

#define delay osDelay(1); // HAL_TIM_Base_Start(&htim6); xSemaphoreTake(TIM6_semaphore_100us, 10)

// Карта клавиш матрицы
const char keyMap[4][4] = {
    {'1', '2', '3', 'U'},
    {'4', '5', '6', 'D'},
    {'7', '8', '9', 'L'},
    {'0', 'P', 'O', 'R'}};
void ret_keyboard()
{   
    
    if (Prot_Keyboard_press_code == 0xFF){
        __HAL_TIM_SET_AUTORELOAD(&htim6, Timer_key_press);
        if (Keyboard_press_code != 0xFF){
            xSemaphoreGive(Display_semaphore);
        }
    }
    else{
        if (Keyboard_press_code == Prot_Keyboard_press_code){
            xSemaphoreGive(Display_semaphore);
        }
        Keyboard_press_code = Prot_Keyboard_press_code;
        if ((mode_redact == 1) || ((mode_redact == 0) && ((Keyboard_press_code == 'U') || (Keyboard_press_code == 'D'))))
        HAL_TIM_Base_Start_IT(&htim6);
    }
    //Keyboard_press_code = Prot_Keyboard_press_code;

    //xSemaphoreGive(Display_semaphore);
    //if ((mode_redact == 1) || ((mode_redact == 0) && ((Keyboard_press_code == 'U') || (Keyboard_press_code == 'D')))) // что бы при зажатии не проваливаться во вложенные пункты меню

    HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, 1);
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, 1);
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, 1);
    HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, 1);
    __HAL_GPIO_EXTI_CLEAR_IT(STR_B1_Pin);
    __HAL_GPIO_EXTI_CLEAR_IT(STR_B2_Pin);
    __HAL_GPIO_EXTI_CLEAR_IT(STR_B3_Pin);
    __HAL_GPIO_EXTI_CLEAR_IT(STR_B4_Pin);
    HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
    HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}


void ScanKeypad()
{
    /*
    osDelay(2);
    // Если был дребезг
    if ((HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == 0) && (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == 0) && (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == 0) && (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == 0)){
        __HAL_TIM_SET_AUTORELOAD(&htim6, Timer_key_press-1);
        __HAL_TIM_SET_COUNTER(&htim6, 0);
        return;
    }
    */
    
    HAL_TIM_Base_Stop_IT(&htim6);
    TIM6->SR &= ~TIM_SR_UIF;

    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_IRQn);

    HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, 1);
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, 0);
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, 0);
    HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, 0);

    delay;
    if (HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[0][0];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[0][1];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[0][2];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[0][3];
        ret_keyboard();
        return;
    }
    HAL_GPIO_WritePin(COL_B1_GPIO_Port, COL_B1_Pin, 0);
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, 1);
    delay;
    if (HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[1][0];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[1][1];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[1][2];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[1][3];
        ret_keyboard();
        return;
    }
    HAL_GPIO_WritePin(COL_B2_GPIO_Port, COL_B2_Pin, 0);

    ////

    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, 1);
    delay;
    if (HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[2][0];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[2][1];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[2][2];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[2][3];
        ret_keyboard();
        return;
    }
    HAL_GPIO_WritePin(COL_B3_GPIO_Port, COL_B3_Pin, 0);

    ///

    HAL_GPIO_WritePin(COL_B4_GPIO_Port, COL_B4_Pin, 1);
    delay;
    if (HAL_GPIO_ReadPin(STR_B1_GPIO_Port, STR_B1_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[3][0];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B2_GPIO_Port, STR_B2_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[3][1];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B3_GPIO_Port, STR_B3_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[3][2];
        ret_keyboard();
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B4_GPIO_Port, STR_B4_Pin) == 1)
    {
        Prot_Keyboard_press_code = keyMap[3][3];
        ret_keyboard();
        return;
    }

    Prot_Keyboard_press_code = 0xFF;
    ret_keyboard();
}
