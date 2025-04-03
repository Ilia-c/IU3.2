#include "w25q128.h"
#include "USB_FATFS_SAVE.h"   // Если вам нужна поддержка USB-файловой системы
#include "Settings.h"         // Если в вашем проекте EEPROM/version и т.д.

extern SPI_HandleTypeDef hspi2;
extern USBH_HandleTypeDef hUsbHostFS;
extern ApplicationTypeDef Appli_state;
extern FATFS USBFatFs;    /* Файловая система для USB */
extern FIL MyFile;        /* Объект файла */
extern char USBHPath[4];  /* Логический путь USB диска */
extern EEPROM_Settings_item EEPROM;

// Управление CS (Chip Select)
#define cs_set()   HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, GPIO_PIN_RESET)
#define cs_reset() HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, GPIO_PIN_SET)

uint32_t g_total_records_count = 0;

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
        return 0;
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
    if (W25_Reset() != 0) {
        return;
    }
    HAL_Delay(100);

    uint32_t id = 0;
    W25_Read_ID(&id);
    if (id != 0xef4018)
    {
        ERRCODE.STATUS |= STATUS_FLASH_INIT_ERROR;
    }
    else
    {
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
    TickType_t start_tick = xTaskGetTickCount();
    while (((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS) < timeout_ms) {
        uint8_t status = W25_Read_Status();
        if ((status & 0x01) == 0) {
            return 0;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
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

    if (W25_WaitForReady(100) != 0) {
        return -1;
    }
    return 0;
}

//------------------------------------------------------------------------------
// Стирание сектора (4 КБ)
int W25_Erase_Sector(uint32_t addr)
{
    uint8_t cmd[4];
    cmd[0] = W25_SECTOR_ERASE;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    if (W25_Write_Enable() != 0) {
        return -1;
    }

    cs_set();
    if (SPI2_Send(cmd, 4) != 0) {
        cs_reset();
        return -1;
    }
    cs_reset();

    // Ждём ~400 мс, пока сотрётся
    if (W25_WaitForReady(400) != 0) {
        return -1;
    }

    return 0;
}

//------------------------------------------------------------------------------
// Полное стирание флеша
int W25_Chip_Erase(void)
{
    uint8_t cmd = W25_CHIP_ERASE;
    if (W25_Write_Enable() != 0) return -1;

    cs_set();
    if (SPI2_Send(&cmd, 1) != 0) {
        cs_reset();
        return -1;
    }
    cs_reset();

    TickType_t start_time = xTaskGetTickCount();
    uint8_t status = 0;
    while ((status = W25_Read_Status()) & 0x01) {
        // Проверяем таймаут
        if (((xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS) >= TIMEOUT_CHIP_ERASE_MS) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    g_total_records_count = 0;
    return ((status & 0x01) ? -1 : 0);
}

//------------------------------------------------------------------------------
// Возвращает адрес свободного блока в "пустом" секторе или стирает сектор №0, если всё заполнено.
int32_t search_sector_empty(void)
{
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
        return -1;
    }

    // Теперь сектор №0 пуст, можно вернуть 0 (адрес начала)
    return 0;
}

//------------------------------------------------------------------------------
// Запись данных во флеш 
int flash_append_record(const char *record_data, uint8_t sector_mark_send_flag)
{
    int32_t addr = search_sector_empty();
    if (addr < 0 || addr >= FLASH_TOTAL_SIZE) return -1;
    if (!record_data) return -1;

    // Проверим длину данных, максимум (RECORD_SIZE - 6)
    size_t len = strlen(record_data);
    if (len > (RECORD_SIZE - 6)) {
        len = RECORD_SIZE - 6;
    }

    // Сформируем структуру
    record_t new_rec;
    // По вашей логике:
    new_rec.Sector_mark       = WRITE_START; // [0] 0xF0 
    new_rec.Sector_mark_send  = EMPTY; // [1]
    new_rec.rec_status_start  = SET;   // [2] => 0x00
    new_rec.rec_status_end    = EMPTY; // [3] => 0xFF
    if (sector_mark_send_flag) new_rec.Sector_mark_send = SET; // 0x00
    else  new_rec.block_mark_send   = EMPTY; // [4] => 0xFF
    new_rec.length            = (uint8_t)len; // [5]
    memset(new_rec.data, 0xFF, sizeof(new_rec.data));
    memcpy(new_rec.data, record_data, len);

    // Запишем 128 байт в блок
    if (W25_Write_Data(addr, (uint8_t *)&new_rec, sizeof(record_t)) != 0) {
        return -1;
    }

    // Ставим rec_status_end = 0x00 => запись завершена
    {
        uint8_t complete_flag = SET; // 0x00
        uint32_t status_addr = addr + offsetof(record_t, rec_status_end);
        if (W25_Write_Data(status_addr, &complete_flag, 1) != 0) {
            return -1;
        }
    }

    // ---- Дополнительный шаг: если это последний блок сектора => стираем следующий сектор, если он не пустой ----
    {
        // 1) выясним индекс сектора и блок внутри сектора
        uint32_t sector_index = (uint32_t)addr / SECTOR_SIZE;
        uint32_t block_index  = ((uint32_t)addr % SECTOR_SIZE) / RECORD_SIZE;

        if (block_index == (RECORDS_PER_SECTOR - 1)) {
            // это последний блок сектора
            uint32_t next_sector = (sector_index + 1) % TOTAL_SECTORS;
            uint32_t next_sector_addr = next_sector * SECTOR_SIZE;

            if (next_sector_addr >= FLASH_TOTAL_SIZE){
                next_sector_addr = 0;
                ERRCODE.STATUS |= STATUS_FLASH_OVERFLOW_ERROR;
            }
            // читаем байт [0] в следующем секторе
            uint8_t next_sector_mark = 0xFF;
            if (W25_Read_Data(next_sector_addr, &next_sector_mark, 1) == 0) {
                // если != 0xFF, значит сектор не пустой => стираем
                if (next_sector_mark != EMPTY) {
                    W25_Erase_Sector(next_sector_addr);
                }
            }
        }
    }

    return 0;
}

//------------------------------------------------------------------------------
// Пометка указанного блока как "отправленного" + проверка, отправлен ли весь сектор
int mark_block_sent(int32_t addr_block)
{
    // Проверка границ
    if (addr_block < 0 || addr_block > (int32_t)(FLASH_TOTAL_SIZE - RECORD_SIZE)) {
        return -1;
    }
    // Проверка, что попадает в начало блока
    if ((addr_block % RECORD_SIZE) != 0) {
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
// Функция резервного копирования данных во внешнее хранилище (USB)
int backup_records_to_external(void)
{
    // Пример вашего кода с минимальными правками

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
    // Допустим, flash_end_ptr не используется, тогда можно убрать всё с last_written / start_sector.

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
            if (W25_Read_Data(block_addr, (uint8_t *)&rec, sizeof(rec)) != 0) {
                ERRCODE.STATUS |= STATUS_USB_FLASH_READ_ERROR;
                continue;
            }
            size_t realLen = rec.length;
            if (realLen > 110){
                rec.data[110] = '\n'; // обрезаем
                rec.data[111] = '\0'; // обрезаем  
            } 
            else{
                rec.data[realLen-1] = '\n'; // обрезаем
                rec.data[realLen] = '\0'; // обрезаем  
            }
            // Если данные не записыны - повреждение data+";data_error\n"
            if (rec.rec_status_end == EMPTY){
                rec.data[realLen-1] = ';';
                rec.data[realLen] = 'E';
                rec.data[realLen+1] = 'R';
                rec.data[realLen+2] = 'R';
                rec.data[realLen+3] = 'O';
                rec.data[realLen+4] = 'R';
                rec.data[realLen+5] = '\n';
                rec.data[realLen+6] = '\0';
            }

            if (rec.block_mark_send == EMPTY){
                rec.data[realLen-1] = ';';
                rec.data[realLen] = 'N';
                rec.data[realLen+1] = 'O';
                rec.data[realLen+2] = '_';
                rec.data[realLen+3] = 'S';
                rec.data[realLen+4] = 'E';
                rec.data[realLen+5] = 'N';
                rec.data[realLen+6] = 'D';
                rec.data[realLen+7] = '\n';
                rec.data[realLen+8] = '\0';
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

//------------------------------------------------------------------------------
// Пример создания имени файла
void createFilename(char *dest, size_t destSize)
{
    // Предполагаем, что EEPROM.version.VERSION_PCB существует
    const char *version = EEPROM.version.VERSION_PCB; // напр. "3.75-A001V"
    char tmp[32];
    int j = 0;

    for (int i = 0; version[i] != '\0' && j < 8; i++) {
        if (version[i] != '.' && version[i] != '-') {
            tmp[j++] = version[i];
        }
    }
    tmp[j] = '\0';

    snprintf(dest, destSize, "%s%s.csv", USBHPath, tmp);
}
