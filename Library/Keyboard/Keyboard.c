#include "stm32l4xx_hal.h"
#include "Settings.h"
extern int Display_update;
extern char Keyboard_press_code;


// Определение пинов для строк
#define STR_B1_PIN    GPIO_PIN_12
#define STR_B2_PIN    GPIO_PIN_8
#define STR_B3_PIN    GPIO_PIN_9
#define STR_B4_PIN    GPIO_PIN_10

// Определение пинов для колонок
#define COL_B1_PIN    GPIO_PIN_7
#define COL_B2_PIN    GPIO_PIN_6
#define COL_B3_PIN    GPIO_PIN_15
#define COL_B4_PIN    GPIO_PIN_14

// Порты, к которым подключены строки
#define STR_B1_PORT   GPIOB
#define STR_B2_PORT   GPIOA
#define STR_B3_PORT   GPIOA
#define STR_B4_PORT   GPIOA

// Порты, к которым подключены колонки
#define COL_B1_PORT   GPIOC
#define COL_B2_PORT   GPIOC
#define COL_B3_PORT   GPIOB
#define COL_B4_PORT   GPIOB

// Карта клавиш матрицы
const char keyMap[4][4] = {
    {'1', '4', '7', '0'},
    {'2', '5', '8', 'P'},
    {'3', '6', '9', 'O'},
    {'U', 'D', 'L', 'R'}
};

void ScanKeypad() {
    osDelay(100);
    
    HAL_GPIO_WritePin(COL_B1_PORT, COL_B1_PIN, 1);
    osDelay(10);
    if (HAL_GPIO_ReadPin(STR_B1_PORT, STR_B1_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B1_PORT, COL_B1_PIN, 0);
        Keyboard_press_code = keyMap[0][0];
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B2_PORT, STR_B2_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B1_PORT, COL_B1_PIN, 0);
        Keyboard_press_code = keyMap[0][1];
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B3_PORT, STR_B3_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B1_PORT, COL_B1_PIN, 0);
        Keyboard_press_code = keyMap[0][2];
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B4_PORT, STR_B4_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B1_PORT, COL_B1_PIN, 0);
        Keyboard_press_code = keyMap[0][3];
        return;
    }
    HAL_GPIO_WritePin(COL_B1_PORT, COL_B1_PIN, 0);

    ////

    HAL_GPIO_WritePin(COL_B2_PORT, COL_B2_PIN, 1);
    osDelay(10);
    if (HAL_GPIO_ReadPin(STR_B1_PORT, STR_B1_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B2_PORT, COL_B2_PIN, 0);
        Keyboard_press_code = keyMap[1][0];
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B2_PORT, STR_B2_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B2_PORT, COL_B2_PIN, 0);
        Keyboard_press_code = keyMap[1][1];
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B3_PORT, STR_B3_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B2_PORT, COL_B2_PIN, 0);
        Keyboard_press_code = keyMap[1][2];
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B4_PORT, STR_B4_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B2_PORT, COL_B2_PIN, 0);
        Keyboard_press_code = keyMap[1][3];
        return;
    }
    HAL_GPIO_WritePin(COL_B2_PORT, COL_B2_PIN, 0);

    ////

    HAL_GPIO_WritePin(COL_B3_PORT, COL_B3_PIN, 1);
    osDelay(10);
    if (HAL_GPIO_ReadPin(STR_B1_PORT, STR_B1_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B3_PORT, COL_B3_PIN, 0);
        Keyboard_press_code = keyMap[2][0];
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B2_PORT, STR_B2_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B3_PORT, COL_B3_PIN, 0);
        Keyboard_press_code = keyMap[2][1];
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B3_PORT, STR_B3_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B3_PORT, COL_B3_PIN, 0);
        Keyboard_press_code = keyMap[2][2];
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B4_PORT, STR_B4_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B3_PORT, COL_B3_PIN, 0);
        Keyboard_press_code = keyMap[2][3];
        return;
    }
    HAL_GPIO_WritePin(COL_B3_PORT, COL_B3_PIN, 0);

    ///

    HAL_GPIO_WritePin(COL_B4_PORT, COL_B4_PIN, 1);
    osDelay(10);
    if (HAL_GPIO_ReadPin(STR_B1_PORT, STR_B1_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B4_PORT, COL_B4_PIN, 0);
        Keyboard_press_code = keyMap[3][0];
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B2_PORT, STR_B2_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B4_PORT, COL_B4_PIN, 0);
        Keyboard_press_code = keyMap[3][1];
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B3_PORT, STR_B3_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B4_PORT, COL_B4_PIN, 0);
        Keyboard_press_code = keyMap[3][2];
        return;
    }
    if (HAL_GPIO_ReadPin(STR_B4_PORT, STR_B4_PIN) == 1) {
        HAL_GPIO_WritePin(COL_B4_PORT, COL_B4_PIN, 0);
        Keyboard_press_code = keyMap[3][3];
        return;
    }
    HAL_GPIO_WritePin(COL_B4_PORT, COL_B4_PIN, 0);
    
}
