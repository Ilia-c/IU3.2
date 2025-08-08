#include "Flash_colibrate_data.h"
#include <string.h>

extern CRC_HandleTypeDef hcrc;  // HW CRC peripheral handle

/**
 * @brief  Подсчёт CRC32 аппаратным модулем (исключая поле crc32)
 */
static uint32_t CalcCRC_Main(const Main_data_settings_item *p)
{
    Main_data_settings_item tmp = *p;
    tmp.crc32 = 0u;
    uint32_t *w   = (uint32_t*)&tmp;
    uint32_t wlen = (uint32_t)(sizeof(tmp) / sizeof(uint32_t));
    __HAL_CRC_DR_RESET(&hcrc);
    return HAL_CRC_Calculate(&hcrc, w, wlen);
}

static uint32_t CalcCRC_Factory(const Factory_data_item *pf)
{
    Factory_data_item tmp;
    memcpy(&tmp, pf, sizeof(tmp));
    tmp.crc32 = 0u;
    uint32_t *w   = (uint32_t*)&tmp;
    uint32_t wlen = (uint32_t)(sizeof(tmp) / sizeof(uint32_t));
    __HAL_CRC_DR_RESET(&hcrc);
    return HAL_CRC_Calculate(&hcrc, w, wlen);
}
/**
 * @brief  Проверить, пуста ли область флеша (все байты == 0xFF)
 * @retval HAL_OK если область пуста, иначе HAL_ERROR
 */
uint8_t Flash_IsCalibEmpty(void)
{
    const uint8_t *p = (const uint8_t*)CALIB_DATA_ADDR;
    for (size_t i = 0; i < sizeof(Factory_data_item); ++i) {
        if (p[i] != 0xFFu) return 0; // что-то записано
    }
    return 1; // пусто
}

// Проверка защиты калибровочной страницы
bool Flash_IsCalibProtected(void)
{
    // Читаем зону B БАНКА 2
    const uint32_t wrp = FLASH->WRP2BR;

    const uint32_t start =
        (wrp & FLASH_WRP2BR_WRP2B_STRT_Msk) >> FLASH_WRP2BR_WRP2B_STRT_Pos;
    const uint32_t end =
        (wrp & FLASH_WRP2BR_WRP2B_END_Msk)  >> FLASH_WRP2BR_WRP2B_END_Pos;

    // «Пустая» конфигурация WRP для этой зоны: защиты нет
    if ((start == 0xFFU && end == 0x00U) || (start == 0xFFU && end == 0xFFU))
        return false;

    // Локальный номер страницы внутри Bank2 для 0x0807F800 — 127
    // (лучше заведи макрос CALIB_BANK_PAGE = 127)
    const uint32_t page = 127U;
    if (start <= end) 
        return (page >= start) && (page <= end);
    else 
        return (page >= start) || (page <= end);
}
/**
 * @brief  Включить защиту WRP на странице калибровки
 * @retval HAL_OK при успехе, HAL_ERROR при ошибке
 */
HAL_StatusTypeDef Flash_ProtectCalibPage(void)
{
    FLASH_OBProgramInitTypeDef obInit = {0};
    HAL_FLASH_Unlock();
    HAL_FLASH_OB_Unlock();

    // На всякий — подтянуть текущую конфигурацию
    HAL_FLASHEx_OBGetConfig(&obInit);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PROGERR | FLASH_FLAG_PGAERR |
                           FLASH_FLAG_PGSERR  | FLASH_FLAG_FASTERR |
                           FLASH_FLAG_MISERR  | FLASH_FLAG_SIZERR);

    obInit.OptionType     = OPTIONBYTE_WRP;
    obInit.WRPArea        = OB_WRPAREA_BANK2_AREAB;   // <-- Bank2 / Area B
    obInit.WRPStartOffset = 127;                      // <-- локальная страница в Bank2
    obInit.WRPEndOffset   = 127;

    if (HAL_FLASHEx_OBProgram(&obInit) != HAL_OK) goto error;
    if (HAL_FLASH_OB_Launch() != HAL_OK) goto error;
    // До этого места мы не дойдём (МК уже перезагрузился)

error:
    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
    return HAL_ERROR;
}

/**
 * @brief  Снять защиту WRP со всех страниц Bank1
 * @retval HAL_OK при успехе, HAL_ERROR при ошибке
 */
HAL_StatusTypeDef Flash_UnprotectCalibPage(void)
{
    FLASH_OBProgramInitTypeDef ob = {0};
    
    // __disable_irq(); // НЕ обязательно. Если оставляешь — см. error-ветку ниже.

    HAL_FLASH_Unlock();
    HAL_FLASH_OB_Unlock();

    // Подтянуть текущую конфигурацию (не обязательно, но полезно)
    HAL_FLASHEx_OBGetConfig(&ob);

    // Сбросить хвосты ошибок, иначе OBProgram может упасть сразу
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PROGERR | FLASH_FLAG_PGAERR |
                           FLASH_FLAG_PGSERR  | FLASH_FLAG_FASTERR |
                           FLASH_FLAG_MISERR  | FLASH_FLAG_SIZERR);

    ob.OptionType = OPTIONBYTE_WRP;

    // НУЖНА именно область банка 2, зона B
    ob.WRPArea = OB_WRPAREA_BANK2_AREAB;

#if defined(OB_WRPSTATE_DISABLE)
    // Если в твоей версии HAL есть поле WRPState — самый чистый способ:
    ob.WRPState = OB_WRPSTATE_DISABLE;
    // WRPStartOffset/WRPEndOffset в этом случае можно не трогать.
#else
    // Универсальный способ для L4: задать «пустой» диапазон (нет защиты)
    ob.WRPStartOffset = 0xFF;   // пусто
    ob.WRPEndOffset   = 0x00;   // пусто
#endif

    if (HAL_FLASHEx_OBProgram(&ob) != HAL_OK) goto error;

    OLED_Clear(0);
    OLED_DrawCenteredString("Защита снята", 30);
    OLED_DrawCenteredString("Перезагрузка", 40);
    OLED_UpdateScreen();
    osDelay(1000);
    // Применяем Option Bytes -> будет аппаратный reset прямо здесь
    if (HAL_FLASH_OB_Launch() != HAL_OK) goto error;

    // До этого места мы не дойдём (МК уже перезагрузился)
    return HAL_OK;

error:
    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
    // __enable_irq(); // если отключал IRQ выше — верни их тут
    return HAL_ERROR;
}



/**
 * @brief  Записать калибровочные данные во Flash
 * @retval HAL_OK при успехе, HAL_ERROR при ошибке
 */
HAL_StatusTypeDef Flash_WriteCalib(const Main_data_settings_item *pData)
{
    // Сформировать контейнер в RAM
    Factory_data_item f = {0};
    f.magic   = FACTORY_SETTINGS_MAGIC;
    f.version = 1u;

    for (int i = 0; i < 3; ++i) {
        f.data[i] = *pData;
        f.data[i].crc32 = CalcCRC_Main(&f.data[i]);
    }
    f.crc32 = CalcCRC_Factory(&f);

    // Стереть одну страницу (Bank2 / page 127)
    FLASH_EraseInitTypeDef erase = {0};
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.Banks     = FLASH_BANK_2;
    erase.Page      = CALIB_BANK_PAGE; // 127
    erase.NbPages   = 1;

    uint32_t pageError = 0xFFFFFFFFu;

    __HAL_FLASH_CLEAR_FLAG( FLASH_FLAG_OPTVERR | FLASH_FLAG_WRPERR |
                            FLASH_FLAG_PROGERR | FLASH_FLAG_PGAERR |
                            FLASH_FLAG_PGSERR | FLASH_FLAG_FASTERR |
                            FLASH_FLAG_MISERR  | FLASH_FLAG_SIZERR );

    HAL_FLASH_Unlock();
    if (HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }

    // Прошить контейнер 64-битными словами
    const uint64_t *dw = (const uint64_t*)&f;
    const uint32_t  n  = (uint32_t)(sizeof(f) / sizeof(uint64_t));
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t addr = CALIB_DATA_ADDR + (i * sizeof(uint64_t));
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, dw[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return HAL_ERROR;
        }
    }

    HAL_FLASH_Lock();
    return Flash_ProtectCalibPage();
}

/**
 * @brief  Прочитать калибровочные данные из Flash и проверить CRC
 * @retval HAL_OK если данные корректны, HAL_ERROR при CRC mismatch
 */
HAL_StatusTypeDef Flash_ReadCalib(Main_data_settings_item *out)
{
    // Быстрая проверка «пусто»
    if (Flash_IsCalibEmpty()) return HAL_ERROR;

    // Считать контейнер в RAM
    Factory_data_item f;
    memcpy(&f, (const void*)CALIB_DATA_ADDR, sizeof(f));

    // Проверка контейнерного MAGIC (если пусто/мусор — будет не совпадать)
    bool magic_ok = (f.magic == FACTORY_SETTINGS_MAGIC);

    // Проверка CRC контейнера (не блокирующая)
    bool factory_crc_ok = (CalcCRC_Factory(&f) == f.crc32);

    // Проверить 3 копии по их CRC
    bool valid[3] = {false,false,false};
    for (int i = 0; i < 3; ++i) {
        uint32_t calc = CalcCRC_Main(&f.data[i]);
        valid[i] = (calc == f.data[i].crc32);
    }

    // Выбор лучшей копии: первая валидная
    int best = -1;
    for (int i = 0; i < 3; ++i) if (valid[i]) { best = i; break; }

    if (best >= 0)
    {
        *out = f.data[best];
        return HAL_OK;
    }
    return HAL_ERROR;
}