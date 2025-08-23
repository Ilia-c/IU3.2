#include "w25q128.h"
#include "USB_FATFS_SAVE.h"   // Если вам нужна поддержка USB-файловой системы
#include "Settings.h"         // Если в вашем проекте EEPROM/version и т.д.

extern SPI_HandleTypeDef hspi2;
extern USBH_HandleTypeDef hUsbHostFS;
extern ApplicationTypeDef Appli_state;
extern FATFS USBFatFs;    /* Файловая система для USB */
extern FIL MyFile;        /* Объект файла */
extern char USBHPath[4];  /* Логический путь USB диска */

extern RTC_HandleTypeDef hrtc;
extern CRC_HandleTypeDef hcrc;

// Управление CS (Chip Select)
#define cs_set()   HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, GPIO_PIN_RESET)
#define cs_reset() HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, GPIO_PIN_SET)

uint32_t g_total_records_count = 0;

const char W25_SECTORS_NOT_EMPTY[2][40]   = {"Не все сектора пустые",    "Not all sectors are empty"};
const char W25_FILE_NOT_FOUND[2][40]      = {"Файл не найден",           "File not found"};
const char W25_INVALID_SIZE[2][40]        = {"Неверный размер",          "Invalid size"};
const char W25_FORMAT_ERROR[2][40]        = {"Ошибка формата",           "Format error"};
const char W25_SIZE_HEADER_MISMATCH[2][40]= {"Размер/заголовок",         "Header/size mismatch"};
const char W25_WRONG_BOARD_VERSION[2][40] = {"Неверная версия платы",    "Wrong board version"};
const char W25_NO_UPDATE[2][40]           = {"Нет обновления",           "No update available"};
const char W25_PREPARE_SPACE[2][40]       = {"Подготовка места",         "Preparing space"};
const char W25_ERASE_ERROR[2][40]         = {"Ошибка стирания",          "Erase error"};
const char W25_WRITE_ERROR[2][40]         = {"Ошибка записи",            "Write error"};
const char W25_COPYING[2][40]             = {"Копирование",              "Copying"};
const char W25_READ_ERROR[2][40]          = {"Ошибка чтения",            "Read error"};
const char W25_WRITE_ERROR_2[2][40]       = {"Ошибка записи 2",          "Write error 2"};
const char W25_COPY_ERROR[2][40]          = {"Ошибка копирования",       "Copy error"};
const char W25_COPY_SUCCESS[2][40]        = {"Успешно скопировано",      "Copied successfully"};
/* :contentReference[oaicite:0]{index=0} */


// ------------------ Вспомогательные SPI-функции ------------------
static int SPI2_Send(uint8_t *dt, uint16_t cnt)
{
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi2, dt, cnt, 1000);
    if (status == HAL_TIMEOUT) {
        ERRCODE.STATUS |= STATUS_FLASH_TIEOUT_ERROR;
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_FLASH_TIEOUT_ERROR;
    if (status == HAL_ERROR) {
        ERRCODE.STATUS |= STATUS_FLASH_SEND_ERROR;
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_FLASH_SEND_ERROR;

    if (status == HAL_BUSY) {
        ERRCODE.STATUS |= STATUS_FLASH_READY_ERROR;
        return 2;
    }
    ERRCODE.STATUS &= ~STATUS_FLASH_READY_ERROR;
    return 0;
}

static int SPI2_Recv(uint8_t *dt, uint16_t cnt)
{
    HAL_StatusTypeDef status = HAL_SPI_Receive(&hspi2, dt, cnt, 1000);
    if (status == HAL_TIMEOUT) {
        ERRCODE.STATUS |= STATUS_FLASH_TIEOUT_ERROR;
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_FLASH_TIEOUT_ERROR;
    if (status == HAL_ERROR) {
        ERRCODE.STATUS |= STATUS_FLASH_RECV_ERROR;
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_FLASH_RECV_ERROR;

    if (status == HAL_BUSY) {
        ERRCODE.STATUS |= STATUS_FLASH_READY_ERROR;
        return 0;
    }
    ERRCODE.STATUS &= ~STATUS_FLASH_READY_ERROR;
    return 0;
}

//------------------------------------------------------------------------------
// Инициализация флеша: сброс, проверка ID
void w25_init(void)
{
    USB_DEBUG_MESSAGE("[DEBUG FLASH] Инициализация FLASH", DEBUG_FLASH, DEBUG_LEVL_1);
    if (W25_Reset() != 0) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] ОШИБКА СБРОСА", DEBUG_FLASH, DEBUG_LEVL_1);
        return;
    }
    HAL_Delay(100);

    uint32_t id = 0;
    USB_DEBUG_MESSAGE("[DEBUG FLASH] Чтение ID", DEBUG_FLASH, DEBUG_LEVL_3);
    W25_Read_ID(&id);
    if (id != 0xef4018)
    {
        USB_DEBUG_MESSAGE("[ERROR FLASH] Неверный ID", DEBUG_FLASH, DEBUG_LEVL_1);
        ERRCODE.STATUS |= STATUS_FLASH_INIT_ERROR;
    }
    else
    {
        USB_DEBUG_MESSAGE("[DEBUG FLASH] FLASH инициализирован", DEBUG_FLASH, DEBUG_LEVL_1);
        ERRCODE.STATUS &= ~STATUS_FLASH_INIT_ERROR;
    }
}

//------------------------------------------------------------------------------
// Чтение ID флеша
int W25_Read_ID(uint32_t *id)
{
    if (!id) return -1;
    uint8_t dt[3];
    uint8_t tx = W25_GET_JEDEC_ID;
    cs_set();
    HAL_Delay(2);
    if (SPI2_Send(&tx, 1) != 0) {
        cs_reset();
        return -1;
    }
    if (SPI2_Recv(dt, 3) != 0) {
        cs_reset();
        return -1;
    }
    cs_reset();
    *id = (dt[0] << 16) | (dt[1] << 8) | dt[2];
    return 0;
}

//------------------------------------------------------------------------------
// Сброс флеша
int W25_Reset(void)
{
    uint8_t tx_buf[2] = {W25_ENABLE_RESET, W25_RESET};
    cs_set();
    HAL_Delay(2);
    if (SPI2_Send(tx_buf, 2) != 0) {
        cs_reset();
        return -1;
    }
    cs_reset();
    return 0;
}

//------------------------------------------------------------------------------
// Чтение данных из флеша
int W25_Read_Data(uint32_t addr, uint8_t *data, uint32_t sz)
{
    if (!data) return -1;
    uint8_t cmd[4];
    cmd[0] = W25_READ;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    cs_set();
    if (SPI2_Send(cmd, 4) != 0) {
        cs_reset();
        return -1;
    }
    if (SPI2_Recv(data, sz) != 0) {
        cs_reset();
        return -1;
    }
    cs_reset();
    return 0;
}

//------------------------------------------------------------------------------
// Разрешение записи
int W25_Write_Enable(void)
{
    uint8_t cmd = W25_WRITE_ENABLE;
    cs_set();
    if (SPI2_Send(&cmd, 1) != 0) {
        cs_reset();
        return -1;
    }
    cs_reset();
    return 0;
}

//------------------------------------------------------------------------------
// Ожидание готовности флеша (FreeRTOS)
int W25_WaitForReady(uint32_t timeout_ms)
{
    uint32_t start_time = HAL_GetTick();
    
    while ((HAL_GetTick() - start_time) < timeout_ms)
    {
        uint8_t status = W25_Read_Status();
        if ((status & 0x01) == 0) {
            return 0; // Готово
        }
        osDelay(1); // Подождём 10 мс
    }
    
    // Превышен таймаут
    return -1;
}


//------------------------------------------------------------------------------
// Чтение статусного регистра
uint8_t W25_Read_Status(void)
{
    uint8_t cmd = W25_READ_STATUS_REG;
    uint8_t status = 0;
    cs_set();
    if (SPI2_Send(&cmd, 1) != 0) {
        cs_reset();
        return 0xFF;
    }
    if (SPI2_Recv(&status, 1) != 0) {
        cs_reset();
        return 0xFF;
    }
    cs_reset();
    return status;
}

//------------------------------------------------------------------------------
// Запись данных во флеш (Page Program)
int W25_Write_Data(uint32_t addr, uint8_t *data, uint32_t sz)
{
    if (!data) return -1;

    // Предполагаем, что sz <= 256 и не пересекает границу страницы
    // (т.к. вы пишете ровно 128 байт за раз).
    if (W25_Write_Enable() != 0) {
        return -1;
    }

    uint8_t cmd[4];
    cmd[0] = W25_PAGE_PROGRAM;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    cs_set();
    if (SPI2_Send(cmd, 4) != 0) {

        cs_reset();
        return -1;
    }
    if (SPI2_Send(data, sz) != 0) {
        cs_reset();
        return -1;
    }
    cs_reset();

    if (W25_WaitForReady(200) != 0) {
        return -1;
    }
    return 0;
}


//------------------------------------------------------------------------------
// Стирание сектора (4 КБ)
int W25_Erase_Sector(uint32_t addr)
{
    USB_DEBUG_MESSAGE("[DEBUG FLASH] Начато стирание сектора FLASH", DEBUG_FLASH, DEBUG_LEVL_3);
    uint8_t cmd[4];
    cmd[0] = W25_SECTOR_ERASE;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    if (W25_Write_Enable() != 0) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка разрешения записи перед стиранием", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }

    cs_set();
    if (SPI2_Send(cmd, 4) != 0) {
        cs_reset();
        USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка отправки команды стирания", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }
    cs_reset();

    // Ждём ~400 мс, пока сотрётся
    if (W25_WaitForReady(400) != 0) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка ожидания готовности после стирания", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }
    USB_DEBUG_MESSAGE("[DEBUG FLASH] Стирание FLASH завершено", DEBUG_FLASH, DEBUG_LEVL_3);
    return 0;
}

//------------------------------------------------------------------------------
// Полное стирание флеша
int W25_Chip_Erase(void)
{
    USB_DEBUG_MESSAGE("[DEBUG FLASH] Начато полное стирание FLASH", DEBUG_FLASH, DEBUG_LEVL_3);
    uint8_t cmd = W25_CHIP_ERASE;
    if (W25_Write_Enable() != 0){
        USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка разрешения записи перед полным стиранием", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }

    cs_set();
    if (SPI2_Send(&cmd, 1) != 0) {
        cs_reset();
        USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка отправки команды полного стирания", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }
    cs_reset();

    TickType_t start_time = xTaskGetTickCount();
    uint8_t status = 0;
    while ((status = W25_Read_Status()) & 0x01) {
        // Проверяем таймаут
        if (((xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS) >= TIMEOUT_CHIP_ERASE_MS) {
            USB_DEBUG_MESSAGE("[ERROR FLASH] Таймаут при ожидании полного стирания", DEBUG_FLASH, DEBUG_LEVL_2);
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    g_total_records_count = 0;
    if (status & 0x01) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка при полном стирании FLASH", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }
    USB_DEBUG_MESSAGE("[DEBUG FLASH] Полное стирание FLASH завершено", DEBUG_FLASH, DEBUG_LEVL_3);
    return 0;
    //return ((status & 0x01) ? -1 : 0);
}

uint32_t W25_Chip_test()
{
    // Подразумевается, что W25_Chip_Erase уже выполнен

    uint32_t id = 0;
    uint8_t step = 0;
    uint8_t Counter_addr_error = 0;
    if (W25_Read_ID(&id) != 0)
    {
        USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка чтения ID", DEBUG_FLASH, DEBUG_LEVL_2);
        return 0xFFFFFFFF; // Ошибка
    }
    // Проверим, что все сектора пустые
    uint8_t sector[RECORD_SIZE] = {0};
    PROGRESS_BAR(0);
    OLED_UpdateScreen();
    for (uint16_t sec = 0; sec < FLASH_SIZE_W25/SECTOR_SIZE; sec++)
    {
        uint32_t sector_addr = sec * SECTOR_SIZE;
        for (uint32_t addr = sector_addr; addr < sector_addr + SECTOR_SIZE; addr += RECORD_SIZE)
        {
            if (W25_Read_Data(addr, sector, RECORD_SIZE) != 0)
            {
                USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка чтения сектора", DEBUG_FLASH, DEBUG_LEVL_2);
                return 0xFFFFFFFF; // Ошибка
            }
            // Проверяем, что сектор пустой
            for (uint32_t i = 0; i < RECORD_SIZE; i++)
            {
                if (sector[i] != EMPTY)
                {
                    Counter_addr_error++;
                }
            }
        }
        uint8_t p = (uint8_t)(sec * 50 / (FLASH_SIZE_W25 / SECTOR_SIZE));
        if (p >= step + 5)
        {
            step = p - (p % 5);
            PROGRESS_BAR(step);
            OLED_UpdateScreen();
        }
    }
    if (Counter_addr_error != 0)
    {
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_SECTORS_NOT_EMPTY, 25);
        OLED_UpdateScreen();
        osDelay(10000);
        USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка: не все сектора пустые", DEBUG_FLASH, DEBUG_LEVL_2);
        return Counter_addr_error; // Ошибка
    }
    step = 0;
    for (uint16_t sec = 0; sec < FLASH_SIZE_W25/SECTOR_SIZE; sec++)
    {
        uint32_t sector_addr = sec * SECTOR_SIZE;
        for (uint32_t addr = sector_addr; addr < sector_addr + SECTOR_SIZE; addr += RECORD_SIZE)
        {
            for (uint8_t i = 0; i < RECORD_SIZE; i++)
            {
                sector[i] = SET; // Заполняем сектор данными 0x00
            }

            if (W25_Write_Data(addr, sector, RECORD_SIZE) != 0)
            {
                return 0xFFFFFFFF; // Ошибка
            }

            if (W25_Read_Data(addr, sector, RECORD_SIZE) != 0)
            {
                USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка чтения сектора", DEBUG_FLASH, DEBUG_LEVL_2);
                return 0xFFFFFFFF; // Ошибка
            }
            // Проверяем, что сектор пустой
            for (uint32_t i = 0; i < RECORD_SIZE; i++)
            {
                if (sector[i] != SET)
                {
                    Counter_addr_error++;
                }
            }
        }
        uint8_t p = (uint8_t)(sec * 50 / (FLASH_SIZE_W25 / SECTOR_SIZE));
        if (p >= step + 5)
        {
            step = p - (p % 5);
            PROGRESS_BAR(50+step);
            OLED_UpdateScreen();
        }
    }
    PROGRESS_BAR(100);
    osDelay(300);
    return Counter_addr_error;
}

//------------------------------------------------------------------------------
// Возвращает адрес свободного блока в "пустом" секторе или стирает сектор №0, если всё заполнено.
int32_t search_sector_empty(void)
{
    USB_DEBUG_MESSAGE("[DEBUG FLASH] Поиск свободного блока в секторах", DEBUG_FLASH, DEBUG_LEVL_3);
    uint8_t sector_header;  // первый байт сектора
    for (uint16_t sec = 0; sec < TOTAL_SECTORS; sec++) {
        uint32_t sector_addr = sec * SECTOR_SIZE;

        // Считываем 1 байт (Sector_mark)
        if (W25_Read_Data(sector_addr, &sector_header, 1) != 0) {
            // Ошибка чтения - пропускаем
            continue;
        }

        if (sector_header != SET) {
            // Считаем, что сектор "неполностью заполнен"
            // Ищем в секторе свободный блок
            for (uint32_t i = 0; i < RECORDS_PER_SECTOR; i++) {
                uint32_t addr = sector_addr + BLOCK_MARK_WRITE_START + i * RECORD_SIZE;
                uint8_t frag_status;
                if (W25_Read_Data(addr, &frag_status, 1) != 0) {
                    // Ошибка чтения - пропускаем
                    ERRCODE.STATUS |= STATUS_FLASH_CRC_ERROR;
                    continue;
                }
                if (frag_status == EMPTY) {
                    // нашли свободный блок
                    g_total_records_count = sec * RECORDS_PER_SECTOR + i;
                    USB_DEBUG_MESSAGE("[DEBUG FLASH] Найден свободный блок", DEBUG_FLASH, DEBUG_LEVL_3);
                    return (int32_t)(addr - BLOCK_MARK_WRITE_START);
                }
            }
            // Если весь сектор на самом деле заполнен, ставим 0x00 (SECTOR_MARK=SET)
            uint8_t full_val = SET; // 0x00
            if (W25_Write_Data(sector_addr, &full_val, 1) == 0) {
                // Записали, что сектор полный
            }
        }
    }

    // Если всё заполнено, стираем сектор №0 (кольцо), возвращаем адрес начала
    if (W25_Erase_Sector(0) != 0) {
        // Ошибка стирания - возвращаем -1
        USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка стирания сектора №0", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }

    // Теперь сектор №0 пуст, можно вернуть 0 (адрес начала)
    USB_DEBUG_MESSAGE("[DEBUG FLASH] Сектор №0 успешно стёрт", DEBUG_FLASH, DEBUG_LEVL_3);
    return 0;
}

//------------------------------------------------------------------------------
// Запись данных во флеш 
int flash_append_record(const char *record_data, uint8_t sector_mark_send_flag)
{
    USB_DEBUG_MESSAGE("[DEBUG FLASH] Запись данных во флеш", DEBUG_FLASH, DEBUG_LEVL_3);
    int32_t addr = search_sector_empty();
    if (addr < 0 || addr >= FLASH_TOTAL_SIZE){
        USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка поиска свободного блока", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }
    if (!record_data){
        USB_DEBUG_MESSAGE("[ERROR FLASH] Данные для записи не указаны", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }

    // Проверим длину данных, максимум (RECORD_SIZE - 6)
    size_t len = strlen(record_data);
    if (len > (RECORD_SIZE - 13)) {
        len = RECORD_SIZE - 13;
    }
    
    record_t new_rec;
    new_rec.Sector_mark       = WRITE_START; // [0] 0xF0 
    new_rec.Sector_mark_send  = EMPTY; // [1]
    new_rec.rec_status_start  = SET;   // [2] => 0x00
    new_rec.rec_status_end    = EMPTY; // [3] => 0xFF
    if (GSM_data.Status & HTTP_SEND_Successfully) new_rec.Sector_mark_send = HTTP_SEND_MARK; // HTTP успешен
    else if (GSM_data.Status & SMS_SEND_Successfully)  new_rec.block_mark_send   = SMS_SEND_MARK; // СМС ушла
    else new_rec.block_mark_send   = EMPTY;
    new_rec.length            = (uint8_t)len; // [5]
    new_rec.CRC32_calc        = 0; // [6-10] Инициализируем CRC
    memset(new_rec.data, 0x00, sizeof(new_rec.data));
    memcpy(new_rec.data, record_data, len);

    uint32_t word_count = (len + 3) / 4;
    uint32_t *p = (uint32_t *)new_rec.data;
    new_rec.CRC32_calc = HAL_CRC_Calculate(&hcrc, p, word_count);
    if (len == 0) new_rec.CRC32_calc = 0xFFFFFFFF;

    // Запишем 128 байт в блок
    if (W25_Write_Data(addr, (uint8_t *)&new_rec, sizeof(record_t)) != 0) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка записи данных во флеш", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }

    // Ставим rec_status_end = 0x00 => запись завершена
    {
        uint8_t complete_flag = SET; // 0x00
        uint32_t status_addr = addr + offsetof(record_t, rec_status_end);
        if (W25_Write_Data(status_addr, &complete_flag, 1) != 0) {
            USB_DEBUG_MESSAGE("[ERROR FLASH] Ошибка записи статуса завершения записи", DEBUG_FLASH, DEBUG_LEVL_2);
            return -1;
        }
    }

    // ---- Дополнительный шаг: если это последний блок сектора => стираем следующий сектор, если он не пустой ----

    // 1) выясним индекс сектора и блок внутри сектора
    uint32_t sector_index = (uint32_t)addr / SECTOR_SIZE;
    uint32_t block_index = ((uint32_t)addr % SECTOR_SIZE) / RECORD_SIZE;

    if (block_index == (RECORDS_PER_SECTOR - 1))
    {
        // это последний блок сектора
        uint32_t next_sector = (sector_index + 1) % TOTAL_SECTORS;
        uint32_t next_sector_addr = next_sector * SECTOR_SIZE;

        if (next_sector_addr >= FLASH_TOTAL_SIZE)
        {
            next_sector_addr = 0;
            ERRCODE.STATUS |= STATUS_FLASH_OVERFLOW_ERROR;
        }
        // читаем байт [0] в следующем секторе
        uint8_t next_sector_mark = 0xFF;
        if (W25_Read_Data(next_sector_addr, &next_sector_mark, 1) == 0)
        {
            // если != 0xFF, значит сектор не пустой => стираем
            if (next_sector_mark != EMPTY)
            {
                USB_DEBUG_MESSAGE("[DEBUG FLASH] Стираем следующий сектор, т.к. он не пустой", DEBUG_FLASH, DEBUG_LEVL_3);
                W25_Erase_Sector(next_sector_addr);
            }
        }
    }
    USB_DEBUG_MESSAGE("[DEBUG FLASH] Запись данных во флеш завершена", DEBUG_FLASH, DEBUG_LEVL_3);
    return 0;
}

//------------------------------------------------------------------------------
// Пометка указанного блока как "отправленного" + проверка, отправлен ли весь сектор
int mark_block_sent(int32_t addr_block)
{
    USB_DEBUG_MESSAGE("[DEBUG FLASH] Пометка блока как отправленного", DEBUG_FLASH, DEBUG_LEVL_4);
    // Проверка границ
    if (addr_block < 0 || addr_block > (int32_t)(FLASH_TOTAL_SIZE - RECORD_SIZE)) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] Адрес блока вне допустимого диапазона", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }
    // Проверка, что попадает в начало блока
    if ((addr_block % RECORD_SIZE) != 0) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] Адрес блока должен быть кратен размеру записи", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }

    // Проверяем, не пуст ли блок
    uint8_t read_byte = 0xFF;
    if (W25_Read_Data(addr_block, &read_byte, 1) != 0) {
        return -1;
    }
    if (read_byte == EMPTY) {
        // Пустой блок
        return 0;
    }

    // Ставим "block_mark_send" = 0x00
    uint8_t block_send = SET; // 0x00
    if (W25_Write_Data(addr_block + BLOCK_MARK_DATA_SEND, &block_send, 1) != 0) {
        return -1;
    }

    // Теперь проверяем, все ли блоки этого сектора отправлены
    uint32_t sector_idx  = (uint32_t)addr_block / SECTOR_SIZE;
    uint32_t sector_addr = sector_idx * SECTOR_SIZE;
    uint8_t marker = 0;  // 0 => все блоки отправлены, 0xFF => есть неотправленные
    
    for (uint32_t i = 0; i < RECORDS_PER_SECTOR; i++) {
        uint32_t cur_block_addr = sector_addr + BLOCK_MARK_DATA_SEND + i * RECORD_SIZE;
        uint8_t mark_val = 0xFF;
        if (W25_Read_Data(cur_block_addr, &mark_val, 1) != 0) {
            // Ошибка чтения - пропускаем
            continue;
        }
        // Если block_mark_send = 0xFF, значит не отправлен
        // Но надо ещё смотреть, пуст ли блок (чтоб не считать пустые)
        // Для простоты, если mark_val == SET => этот блок уже помечен как отправленный. 
        // Иначе => 0xFF = неотправленный
        if (mark_val == EMPTY) {
            // Может блок пуст? Пропустим. 
            continue;
        }
        // Если это 0xFF => блок есть, но не отправлен
        if (mark_val == EMPTY) {
            // ничего
        } else if (mark_val != SET) {
            // если что-то странное, тоже считаем неотправленным
            marker = 0xFF;
            break;
        } else {
            // mark_val == SET => OK, уже отправлен
        }
        if (mark_val == 0xFF) {
            marker = 0xFF;
            break;
        }
    }

    // Если marker == 0 => все блоки отправлены => ставим sector_mark_send = 0x00
    if (marker == 0) {
        uint32_t sectorMarkerAddr = sector_addr + SECTOR_MARK_SEND; 
        uint8_t val = SET; // 0x00
        if (W25_Write_Data(sectorMarkerAddr, &val, 1) != 0) {
            return -1;
        }
    }

    return 0;
}
int Save_one_to_USB(void)
{
    FRESULT res;
    UINT bw;
    FATFS *fs;
    DWORD fre_clust;
    
    ERRCODE.STATUS &= ~STATUS_USB_FULL_ERROR;
    ERRCODE.STATUS &= ~STATUS_USB_OPEN_ERROR;
    ERRCODE.STATUS &= ~STATUS_USB_LSEEK_ERROR;
    ERRCODE.STATUS &= ~STATUS_USB_FLASH_WRITE_ERROR;
    ERRCODE.STATUS &= ~STATUS_USB_FLASH_READ_ERROR;
    ERRCODE.STATUS &= ~STATUS_USB_FLASH_SYNC_ERROR;

    res = f_getfree(USBHPath, &fre_clust, &fs);
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_USB_FULL_ERROR;
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_USB_FULL_ERROR;
    
    char filename[16];
    createFilename(filename, sizeof(filename));
    res = f_open(&MyFile, filename, FA_OPEN_APPEND | FA_WRITE);
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_USB_OPEN_ERROR;
        f_close(&MyFile);
        f_mount(NULL, USBHPath, 0);
        memset(&USBFatFs, 0, sizeof(FATFS));
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_USB_OPEN_ERROR;

    res = f_lseek(&MyFile, f_size(&MyFile));
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_USB_LSEEK_ERROR;
        f_close(&MyFile);
        f_mount(NULL, USBHPath, 0);
        memset(&USBFatFs, 0, sizeof(FATFS));
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_USB_LSEEK_ERROR;
    Collect_DATA();


    uint16_t len_data = strlen(save_data);
    save_data[len_data-1] = '\n'; // обрезаем
    save_data[len_data] = '\0'; // обрезаем  
    char *data = save_data + 1; 
    res = f_write(&MyFile, data, strlen(data), &bw);
    if (res != FR_OK)
    {
        ERRCODE.STATUS |= STATUS_USB_FLASH_WRITE_ERROR;
        f_close(&MyFile);
        f_mount(NULL, USBHPath, 0);
        memset(&USBFatFs, 0, sizeof(FATFS));
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_USB_FLASH_WRITE_ERROR;

    res = f_sync(&MyFile);
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_USB_FLASH_SYNC_ERROR;
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_USB_FLASH_SYNC_ERROR;
    f_close(&MyFile);

    return 0;
}


//------------------------------------------------------------------------------
// Функция копирования данных во внешнее хранилище (USB)
int backup_records_to_external(void)
{
    FRESULT res;
    UINT bw;
    FATFS *fs;
    DWORD fre_clust;
    
    res = f_getfree(USBHPath, &fre_clust, &fs);
    if (res != FR_OK) {
        return -1;
        ERRCODE.STATUS |= STATUS_USB_FULL_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_USB_FULL_ERROR;

    char filename[16];
    createFilename(filename, sizeof(filename));
    res = f_open(&MyFile, filename, FA_OPEN_APPEND | FA_WRITE);
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_USB_OPEN_ERROR;
        f_close(&MyFile);
        f_mount(NULL, USBHPath, 0);
        memset(&USBFatFs, 0, sizeof(FATFS));
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_USB_OPEN_ERROR;
    res = f_lseek(&MyFile, f_size(&MyFile));
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_USB_LSEEK_ERROR;
        f_close(&MyFile);
        f_mount(NULL, USBHPath, 0);
        memset(&USBFatFs, 0, sizeof(FATFS));
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_USB_LSEEK_ERROR;

    // Сканируем все секторы
    record_t rec;
    osDelay(300);
    memset(&rec, 0, sizeof(rec));
    osDelay(100);

    for (uint16_t sec = 0; sec < TOTAL_SECTORS; sec++) {
        uint32_t sector_addr = sec * SECTOR_SIZE;
        uint8_t sector_header = 0xFF;
        if (W25_Read_Data(sector_addr, &sector_header, 1) != 0) {
            // Ошибка чтения - пропускаем
            continue;
        }
        // Если сектор "пустой" (0xFF) - пропускаем
        if (sector_header == EMPTY) {
            continue;
        }

        // иначе перебираем блоки
        for (uint32_t i = 0; i < RECORDS_PER_SECTOR; i++) {
            // адрес "rec_status_start" (смещение=2)
            uint32_t block_addr = sector_addr + (i * RECORD_SIZE) + BLOCK_MARK_WRITE_START;
            uint8_t block_header = 0xFF;
            if (W25_Read_Data(block_addr, &block_header, 1) != 0) {
                ERRCODE.STATUS |= STATUS_USB_FLASH_READ_ERROR;
                // ошибка чтения
                continue;
            }
            // Если блок тоже пустой, пропускаем
            if (block_header == EMPTY) {
                continue;
            }

            // Читаем весь блок
            // сместимся назад на 2 байта
            block_addr -= BLOCK_MARK_WRITE_START;
            if (W25_Read_Data(block_addr, (uint8_t *)&rec, sizeof(rec)) != 0)
            {
                ERRCODE.STATUS |= STATUS_USB_FLASH_READ_ERROR;
                continue;
            }
            size_t realLen = rec.length;
            if (realLen > 240)
            {
                rec.data[240] = '\n'; // обрезаем
                rec.data[241] = '\0'; // обрезаем
            }
            else
            {
                rec.data[realLen - 1] = '\n'; // обрезаем
                rec.data[realLen] = '\0';     // обрезаем
            }
            // Если данные не записыны - повреждение data+";data_error\n"
            if (rec.rec_status_end == EMPTY)
            {
                strcpy(&rec.data[realLen - 1], ";ERROR\n");
            }
            else
            {

                if (rec.block_mark_send == EMPTY)
                {
                    strcpy(&rec.data[realLen - 1], ";NO_SEND\n");
                }
                if (rec.block_mark_send == SMS_SEND_MARK)
                {
                    strcpy(&rec.data[realLen - 1], ";SMS\n");
                }
                // Проверка CRC32
                uint32_t word_count = (rec.length + 3) / 4;
                uint32_t *p = (uint32_t *)rec.data;
                uint32_t CRC32_clac = HAL_CRC_Calculate(&hcrc, p, word_count);
                if (rec.length == 0)
                    CRC32_clac = 0xFFFFFFFF;
                if (CRC32_clac != rec.CRC32_calc)
                {
                    strcpy(&rec.data[realLen - 1], ";CRC\n");
                }
            }

            // Записываем в файл
            char *data = rec.data + 1; 
            res = f_write(&MyFile, data, strlen(data), &bw);
            if (res != FR_OK)
            {
                ERRCODE.STATUS |= STATUS_USB_FLASH_WRITE_ERROR;
                f_close(&MyFile);
                f_mount(NULL, USBHPath, 0);
                memset(&USBFatFs, 0, sizeof(FATFS));
                return -1;
            }
        }
    }
    
    res = f_sync(&MyFile);
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_USB_FLASH_SYNC_ERROR;
        // ошибка синхронизации
    }
    ERRCODE.STATUS &= ~STATUS_USB_FLASH_SYNC_ERROR;
    f_close(&MyFile);
    // f_mount(NULL, USBHPath, 0); // Если нужно
    return 1;
}


// Копирование ошибки на USB
int backup_fault_to_external(void) {
    FRESULT res;
    DWORD fre_clust;
    FATFS *fs;
    UINT bw;

    // 4) Берём указатель на дамп в Flash
    volatile FaultLog_t *log = FlashBackup_GetLog();
    if (log->magic != FAULTLOG_MAGIC) {
        // Нет валидного дампа — просто закрываем и выходим
        f_close(&MyFile);
        return 0;
    }


    // 1) Проверяем, что на USB есть место и диск смонтирован
    res = f_getfree(USBHPath, &fre_clust, &fs);
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_USB_FULL_ERROR;
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_USB_FULL_ERROR;

    char filename[16];
    createFilename_HARD_FAULT(filename, sizeof(filename));
    res = f_open(&MyFile, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_USB_OPEN_ERROR;
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_USB_OPEN_ERROR;



    // 5) Записываем «сырой» бинарный дамп структуры
    res = f_write(&MyFile, log, sizeof(*log), &bw);
    if (res != FR_OK || bw != sizeof(*log)) {
        ERRCODE.STATUS |= STATUS_USB_FLASH_WRITE_ERROR;
        f_close(&MyFile);
        return -1;
    }
    ERRCODE.STATUS &= ~STATUS_USB_FLASH_WRITE_ERROR;

    // 6) Синхронизируем и закрываем
    res = f_sync(&MyFile);
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_USB_FLASH_SYNC_ERROR;
        // продолжаем — файл всё равно закроем
    } else {
        ERRCODE.STATUS &= ~STATUS_USB_FLASH_SYNC_ERROR;
    }
    f_close(&MyFile);


    return 1;
}


//------------------------------------------------------------------------------
// Пример создания имени файла
void createFilename(char *dest, size_t destSize)
{
    // Предполагаем, что EEPROM.version.VERSION_PCB существует
    char Version[20] = {0};
    strncpy(Version, Main_data.version.VERSION_PCB, sizeof(Version)-1);
    remove_braces_inplace(Version);

    char tmp[32];
    int j = 0;

    for (int i = 0; Version[i] != '\0' && j < 8; i++) {
        if (Version[i] != '.' && Version[i] != '-') {
            tmp[j++] = Version[i];
        }
    }
    tmp[j] = '\0';
    snprintf(dest, destSize, "%s%s.csv", USBHPath, tmp);
}

void createFilename_HARD_FAULT(char *dest, size_t destSize)
{
    // Предполагаем, что EEPROM.version.VERSION_PCB существует
    char Version[20] = {0};
    strncpy(Version, Main_data.version.VERSION_PCB, sizeof(Version)-1);
    remove_braces_inplace(Version);

    char tmp[32];
    int j = 0;

    for (int i = 0; Version[i] != '\0' && j < 8; i++) {
        if (Version[i] != '.' && Version[i] != '-') {
            tmp[j++] = Version[i];
        }
    }
    tmp[j] = '\0';
    snprintf(dest, destSize, "%s%s.err", USBHPath, tmp);
}


//---------------------------------------//
//             ОБНОВЛЕНИЕ ПО             //
//---------------------------------------//
#define UPDATE_BUFFER_SIZE  256
#define EXTERNAL_HEADER_SIZE 36U
#define EXT_FW_AREA_SIZE TARGET_FLASH_SIZE
#define EXTERNAL_FW_START FLASH_TOTAL_SIZE
#define APP_SIZE (446U * 1024U)
#define EXTERNAL_MAX_FW_SIZE (APP_SIZE)
#define APP_ADDRESS 0x08010000U
static uint32_t gFirmwareSize = 0;


int W25_Write_PageAligned(uint32_t addr,
                                 const uint8_t *src,
                                 uint32_t len)
{
  while (len)
  {
    uint32_t pageRest = 256U - (addr & 0xFFU);
    uint32_t chunk = (len < pageRest) ? len : pageRest;

    if (W25_Write_Data(addr, src, chunk) != 0)
      return -1;
    if (W25_WaitForReady(500U) != 0)
      return -1;

    addr += chunk;
    src += chunk;
    len -= chunk;
  }
  return 0;
}
static uint32_t CRC32_Update(uint32_t crc, uint8_t data)
{
  crc ^= data;
  for (uint8_t i = 0; i < 8; i++)
    crc = (crc & 1U) ? (crc >> 1) ^ 0xEDB88320U : (crc >> 1);
  return crc & 0xFFFFFFFFU; /* <?? маска на всякий случай */
}

/* ===== Функция очистки внешней флеш области ================= */
static int EraseExternalFirmwareArea(void)
{
  const uint32_t sectorSize = 4096;
  uint32_t totalSectors = EXT_FW_AREA_SIZE / sectorSize;
  uint8_t step = 0;
  for (uint32_t i = 0; i < totalSectors; ++i)
  {
    uint32_t addr = EXTERNAL_FW_START + i * sectorSize;
    // Стираем сектор и ждём готовности
    if (W25_Erase_Sector(addr) != 0)
      return -1;
    if (W25_WaitForReady(500) != 0)
      return -1;
    // Обновляем прогресс каждые 10%
    uint8_t p = (uint8_t)(((i + 1) * 100) / totalSectors);
    if (p >= step + 10)
    {
      step = p - (p % 10);
      PROGRESS_BAR(step);
      OLED_UpdateScreen();
    }
  }
  return 0;
}

FRESULT FindAndOpenFirstBin(FIL *pFile, const char *USBHPath)
{
  DIR dir;
  FILINFO fno;
  FRESULT res;
  char fullpath[64];

  // Откроем корень
  res = f_opendir(&dir, USBHPath);
  if (res != FR_OK)
    return res;

  // Лучший кандидат
  int found = 0;
  uint16_t best_fw_ver = 0;
  WORD best_date = 0; // FAT дата
  WORD best_time = 0; // FAT время
  char best_path[sizeof(fullpath)];

  for (;;)
  {
    res = f_readdir(&dir, &fno);
    if (res != FR_OK)
    {
      f_closedir(&dir);
      return res;
    }
    if (fno.fname[0] == 0)
      break; // конец списка
    if (fno.fattrib & AM_DIR)
      continue; // пропускаем папки

    // Проверка расширения .BIN (без LFN у FatFs обычно уже верхний регистр)
    const char *name = fno.fname;
    size_t len = strlen(name);
    if (len < 4 || name[len - 4] != '.')
      continue;
    char e0 = (char)toupper((unsigned char)name[len - 3]);
    char e1 = (char)toupper((unsigned char)name[len - 2]);
    char e2 = (char)toupper((unsigned char)name[len - 1]);
    if (!(e0 == 'B' && e1 == 'I' && e2 == 'N'))
      continue;

    // Полный путь вида "0:/filename.BIN" — USBHPath ДОЛЖЕН оканчиваться '/'
    snprintf(fullpath, sizeof(fullpath), "%s%s", USBHPath, name);

    // Откроем кандидат
    FIL f;
    if (f_open(&f, fullpath, FA_READ) != FR_OK)
      continue;

    // Грубые проверки размеров
    DWORD fsz = f_size(&f);
    if (fsz < EXTERNAL_HEADER_SIZE || fsz > EXTERNAL_HEADER_SIZE + EXTERNAL_MAX_FW_SIZE)
    {
      f_close(&f);
      continue;
    }

    // Чтение 16-байтного заголовка
    uint8_t hdr[16];
    UINT br = 0;
    if (f_read(&f, hdr, sizeof(hdr), &br) != FR_OK || br != sizeof(hdr))
    {
      f_close(&f);
      continue;
    }

    // Парсинг LE: [size8][crc4][fw_ver2][board_ver2]
    uint64_t size = 0;
    for (int i = 0; i < 8; ++i)
      size |= ((uint64_t)hdr[i]) << (8 * i);

    if (size == 0 || size > EXTERNAL_MAX_FW_SIZE || (EXTERNAL_HEADER_SIZE + size) != fsz)
    {
      f_close(&f);
      continue;
    }

    uint16_t fw_ver = (uint16_t)(hdr[12] | (hdr[13] << 8));
    uint16_t board_ver = (uint16_t)(hdr[14] | (hdr[15] << 8));

    // Фильтр по плате
    if (board_ver != (uint16_t)BOARD_VERSION)
    {
      f_close(&f);
      continue;
    }

    // Кандидат подходит — выберем лучший (по fw_ver, затем по дате/времени)
    if (!found ||
        fw_ver > best_fw_ver ||
        (fw_ver == best_fw_ver && (fno.fdate > best_date || (fno.fdate == best_date && fno.ftime > best_time))))
    {
      found = 1;
      best_fw_ver = fw_ver;
      best_date = fno.fdate;
      best_time = fno.ftime;
      strncpy(best_path, fullpath, sizeof(best_path));
      best_path[sizeof(best_path) - 1] = '\0';
    }

    f_close(&f);
  }

  f_closedir(&dir);

  if (!found)
    return FR_NO_FILE;
  // Открываем лучший найденный файл
  return f_open(pFile, best_path, FA_READ);
}

void Update_PO(void)
{
    FRESULT res;
    char path[32];

    if ((res = FindAndOpenFirstBin(&MyFile, USBHPath)) != FR_OK) {
         OLED_DrawCenteredString(W25_FILE_NOT_FOUND, 30);
         OLED_UpdateScreen();
         return;
     }

    // 2) Проверяем размер файла
    DWORD fullSize = f_size(&MyFile);
    if (fullSize <= EXTERNAL_HEADER_SIZE || fullSize > EXTERNAL_HEADER_SIZE + EXTERNAL_MAX_FW_SIZE) {
        f_close(&MyFile);
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_INVALID_SIZE, 10);
        OLED_UpdateScreen();
        osDelay(1000);
        return;
    }

    // 3) Читаем 8 байт размера + 4 байта CRC32

    UINT br;
    uint8_t size_hdr[8], crc_hdr[4], ver_hdr[4], iv_hdr[16], crc_enc[4];
    // Читаем 8 байт размера прошивки
    if (f_read(&MyFile, size_hdr, 8, &br) != FR_OK || br != 8) { 
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_FORMAT_ERROR, 10);
        OLED_UpdateScreen();
        f_close(&MyFile); 
        return; 
    }
    // Читаем 4 байта CRC32
    if (f_read(&MyFile, crc_hdr, 4, &br) != FR_OK || br != 4) { 
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_FORMAT_ERROR, 10);
        OLED_UpdateScreen();
        f_close(&MyFile); 
        return;  
    }

    // Читаем 4 байта версии прошивки и версии платы
    if (f_read(&MyFile, ver_hdr, 4, &br) != FR_OK || br != 4) {
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_FORMAT_ERROR, 10);
        OLED_UpdateScreen();
        f_close(&MyFile);
        return;
    }

    // читаем 16 байт ключа шифрования
    if (f_read(&MyFile, iv_hdr, 16, &br) != FR_OK || br != 16) { 
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_FORMAT_ERROR, 10);
        OLED_UpdateScreen();
        f_close(&MyFile); 
        return; 
    }

    // Запись 4 байт CRC32 зашифрованных для проверки расшифровки
    if (f_read(&MyFile, crc_enc, 4, &br) != FR_OK || br != 4) { 
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_FORMAT_ERROR, 10);
        OLED_UpdateScreen();
        f_close(&MyFile); 
        return; 
    }

    // Вычисляем размер прошивки из первых 8 байт (little endian)
    gFirmwareSize = (uint64_t)size_hdr[0] | ((uint64_t)size_hdr[1] << 8) | ((uint64_t)size_hdr[2] << 16) | ((uint64_t)size_hdr[3] << 24) | ((uint64_t)size_hdr[4] << 32) | ((uint64_t)size_hdr[5] << 40) | ((uint64_t)size_hdr[6] << 48) | ((uint64_t)size_hdr[7] << 56);
    // Ожидаемый CRC32
    uint32_t expectedCRC = (uint32_t)crc_hdr[0] | ((uint32_t)crc_hdr[1] << 8) | ((uint32_t)crc_hdr[2] << 16) | ((uint32_t)crc_hdr[3] << 24);
    uint32_t expectedCRC_enc =
      (uint32_t)crc_enc[0] |
      ((uint32_t)crc_enc[1] << 8) |
      ((uint32_t)crc_enc[2] << 16) |
      ((uint32_t)crc_enc[3] << 24);

    // Версия ПО (LE, 2 байта) и версия платы (LE, 2 байта)
    uint16_t new_fw_ver = (uint16_t)(ver_hdr[0] | (ver_hdr[1] << 8));
    uint16_t new_boardver = (uint16_t)(ver_hdr[2] | (ver_hdr[3] << 8));

    // sanity-check: длина из заголовка должна совпасть с длиной файла
    if (gFirmwareSize == 0 ||
        gFirmwareSize > EXTERNAL_MAX_FW_SIZE ||
        (uint64_t)fullSize != (uint64_t)EXTERNAL_HEADER_SIZE + gFirmwareSize)
    {
        f_close(&MyFile);
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_SIZE_HEADER_MISMATCH, 10);
        OLED_UpdateScreen();
        osDelay(1000);
        return;
    }
    if (new_boardver != BOARD_VERSION) {
        f_close(&MyFile);
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_WRONG_BOARD_VERSION, 10);
        OLED_UpdateScreen();
        osDelay(1000);
        return;
    }

    if (new_fw_ver < DEFAULT_VERSION_PROGRAMM_UINT16_t) {
        f_close(&MyFile);
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_NO_UPDATE, 10);
        OLED_UpdateScreen();
        osDelay(1000);
        return;
    }


    // Очистка внешнего хранилища
    OLED_Clear(0);
    OLED_DrawCenteredString(W25_PREPARE_SPACE, 10);
    PROGRESS_BAR(0);
    OLED_UpdateScreen();
    if (EraseExternalFirmwareArea() != 0) { 
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_ERASE_ERROR, 10);
        OLED_UpdateScreen();
        f_close(&MyFile); 
        return; 
    }


    // Записываем в W25Q128 все 16 байт заголовка
    if (W25_Write_Data(EXTERNAL_FW_START,        size_hdr, 8) != 0) { 
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_WRITE_ERROR, 10);
        OLED_UpdateScreen();
        f_close(&MyFile); 
        return; 
    }
    if (W25_Write_Data(EXTERNAL_FW_START + 8, crc_hdr, 4) != 0)      { 
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_WRITE_ERROR, 10);
        OLED_UpdateScreen();
        f_close(&MyFile); 
        return; 
    }
    if (W25_Write_Data(EXTERNAL_FW_START + 12, ver_hdr, 4) != 0) {
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_WRITE_ERROR, 10);
        OLED_UpdateScreen();
        f_close(&MyFile);
        return;
    }
    if (W25_Write_Data(EXTERNAL_FW_START + 16, iv_hdr, 16) != 0) {
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_WRITE_ERROR, 10);
        OLED_UpdateScreen();
        f_close(&MyFile);
        return;
    }
    
    if (W25_Write_Data(EXTERNAL_FW_START + 32, crc_enc, 4) != 0) {
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_WRITE_ERROR, 10);
        OLED_UpdateScreen();
        f_close(&MyFile);
        return;
    }
    
    // Копируем остальной бинарник с прогрессом и проверяем CRC на лету
    OLED_Clear(0);
    OLED_DrawCenteredString(W25_COPYING, 10);
    PROGRESS_BAR(0);
    OLED_UpdateScreen();

    uint32_t total = 0;
    uint32_t addr  = EXTERNAL_FW_START + EXTERNAL_HEADER_SIZE;
    uint32_t crc   = 0xFFFFFFFFU;
    uint8_t  buf[512];
    uint8_t  step  = 0;

    while (total < gFirmwareSize) {
        UINT need = (gFirmwareSize - total > sizeof(buf)) ? sizeof(buf) : (gFirmwareSize - total);
        if (f_read(&MyFile, buf, need, &br) != FR_OK || br == 0) { 
            OLED_Clear(0);
            OLED_DrawCenteredString(W25_READ_ERROR, 30);
            OLED_UpdateScreen();
            f_close(&MyFile); 
            return; 
        }
        taskENTER_CRITICAL();
        for (UINT i = 0; i < br; ++i) crc = CRC32_Update(crc, buf[i]);
        if (W25_Write_PageAligned(addr, buf, br) != 0) { 
            OLED_Clear(0);
            OLED_DrawCenteredString(W25_WRITE_ERROR_2, 30);
            OLED_UpdateScreen();
            f_close(&MyFile); 
            taskEXIT_CRITICAL();
            return;  
        }
        taskEXIT_CRITICAL();

        addr  += br;
        total += br;

        uint8_t p = (uint8_t)(100UL * total / gFirmwareSize);
        if (p >= step + 10) {
            step = p - (p % 10);
            PROGRESS_BAR(step);
            OLED_UpdateScreen();
        }
    }

    f_close(&MyFile);
    crc = ~crc;
    if (crc != expectedCRC_enc) {
        OLED_Clear(0);
        OLED_DrawCenteredString(W25_COPY_ERROR, 10);
        OLED_UpdateScreen();
        HAL_Delay(1000);
        EraseExternalFirmwareArea();
        return;
    }

    OLED_Clear(0);
    OLED_DrawCenteredString(W25_COPY_SUCCESS, 10);
    OLED_UpdateScreen();
    HAL_Delay(1000);

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_RTC_ENABLE();
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, (uint16_t)(gFirmwareSize >> 16));
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, (uint16_t)(gFirmwareSize & 0xFFFF));
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, BKP_UPDATE_FLAG);
    NVIC_SystemReset();
}