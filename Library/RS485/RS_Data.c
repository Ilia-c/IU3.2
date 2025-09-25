#include "RS_Data.h"

void RS_send(void)
{
    Collect_DATA();
    HAL_StatusTypeDef res = RS485_Transmit_IT(save_data, sizeof(save_data) - 1);
    if (res != HAL_OK)
    {
        USB_DEBUG_MESSAGE("[DEBUG RS485] Ошибка передачи данных", DEBUG_RS485, DEBUG_LEVL_2);
        ERRCODE.STATUS |= STATUS_RS485_TX_ERROR;
    }
    else
    {
        ERRCODE.STATUS &= ~STATUS_RS485_TX_ERROR; // Сброс ошибки передачи
        USB_DEBUG_MESSAGE("[DEBUG RS485] Данные успешно отправлены", DEBUG_RS485, DEBUG_LEVL_3);
    }
}