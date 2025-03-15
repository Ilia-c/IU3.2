#include "w25q128.h"
#include <string.h>
#include <stddef.h>

// Макросы управления CS (Chip Select)
#define cs_set()   HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, 0)
#define cs_reset() HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, 1)

extern SPI_HandleTypeDef hspi2;
uint32_t flash_end_ptr = 0;

// ------------------ Низкоуровневые SPI-функции ------------------
static int SPI2_Send(uint8_t *dt, uint16_t cnt) {
    return (HAL_SPI_Transmit(&hspi2, dt, cnt, 1000) == HAL_OK) ? 0 : -1;
}
static int SPI2_Recv(uint8_t *dt, uint16_t cnt) {
    return (HAL_SPI_Receive(&hspi2, dt, cnt, 1000) == HAL_OK) ? 0 : -1;
}

//------------------------------------------------------------------------------
// w25_init: Сброс флеша, проверка ID и обновление flash_end_ptr.
void w25_init(void) {
    if (W25_Reset() != 0) {
        return;
    }
    HAL_Delay(100);
    
    uint32_t id = 0;
    if (W25_Read_ID(&id) != 0) {
        return;
    }
    if (id != 0xEF4018) {
        // Можно установить код ошибки
        return;
    }
    update_flash_end_ptr();
}

//------------------------------------------------------------------------------
// Чтение ID флеша
int W25_Read_ID(uint32_t *id) {
    if (!id) return -1;
    uint8_t dt[3];
    uint8_t tx = W25_GET_JEDEC_ID;
    cs_set();
    HAL_Delay(2);
    if (SPI2_Send(&tx, 1) != 0) { cs_reset(); return -1; }
    if (SPI2_Recv(dt, 3) != 0) { cs_reset(); return -1; }
    cs_reset();
    *id = (dt[0] << 16) | (dt[1] << 8) | dt[2];
    return 0;
}

//------------------------------------------------------------------------------
// Сброс флеша
int W25_Reset(void) {
    uint8_t tx_buf[2] = {W25_ENABLE_RESET, W25_RESET};
    cs_set();
    HAL_Delay(2);
    if (SPI2_Send(tx_buf, 2) != 0) { cs_reset(); return -1; }
    cs_reset();
    return 0;
}

//------------------------------------------------------------------------------
// Чтение данных из флеша
int W25_Read_Data(uint32_t addr, uint8_t *data, uint32_t sz) {
    if (!data) return -1;
    uint8_t cmd[4];
    cmd[0] = W25_READ;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;
    cs_set();
    if (SPI2_Send(cmd, 4) != 0) { cs_reset(); return -1; }
    if (SPI2_Recv(data, sz) != 0) { cs_reset(); return -1; }
    cs_reset();
    return 0;
}

//------------------------------------------------------------------------------
// Разрешение записи
int W25_Write_Enable(void) {
    uint8_t cmd = W25_WRITE_ENABLE;
    cs_set();
    if (SPI2_Send(&cmd, 1) != 0) { cs_reset(); return -1; }
    cs_reset();
    return 0;
}

//------------------------------------------------------------------------------
// Ожидание готовности флеша (FreeRTOS)
int W25_WaitForReady(uint32_t timeout_ms) {
    TickType_t start_tick = xTaskGetTickCount();
    while (((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS) < timeout_ms) {
        uint8_t status = W25_Read_Status();
        if ((status & 0x01) == 0)
            return 0;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return -1;
}

//------------------------------------------------------------------------------
// Запись данных во флеш (Page Program)
int W25_Write_Data(uint32_t addr, uint8_t *data, uint32_t sz) {
    if (!data) return -1;
    uint8_t cmd[4];
    cmd[0] = W25_PAGE_PROGRAM;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;
    if (W25_Write_Enable() != 0) return -1;
    cs_set();
    if (SPI2_Send(cmd, 4) != 0) { cs_reset(); return -1; }
    if (SPI2_Send(data, sz) != 0) { cs_reset(); return -1; }
    cs_reset();
    if (W25_WaitForReady(100) != 0) return -1;
    return 0;
}

//------------------------------------------------------------------------------
// Стирание сектора
int W25_Erase_Sector(uint32_t addr) {
    uint8_t cmd[4];
    cmd[0] = W25_SECTOR_ERASE;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;
    if (W25_Write_Enable() != 0) return -1;
    cs_set();
    if (SPI2_Send(cmd, 4) != 0) { cs_reset(); return -1; }
    cs_reset();
    if (W25_WaitForReady(400) != 0) return -1;
    return 0;
}

//------------------------------------------------------------------------------
// Чтение статусного регистра
uint8_t W25_Read_Status(void) {
    uint8_t cmd = W25_READ_STATUS_REG;
    uint8_t status = 0;
    cs_set();
    if (SPI2_Send(&cmd, 1) != 0) { cs_reset(); return 0xFF; }
    if (SPI2_Recv(&status, 1) != 0) { cs_reset(); return 0xFF; }
    cs_reset();
    return status;
}

//------------------------------------------------------------------------------
// Полное стирание флеша
int W25_Chip_Erase(void) {
    uint8_t cmd = W25_CHIP_ERASE;
    if (W25_Write_Enable() != 0) return -1;
    cs_set();
    if (SPI2_Send(&cmd, 1) != 0) { cs_reset(); return -1; }
    cs_reset();
    TickType_t start_time = xTaskGetTickCount();
    uint8_t status = 0;
    while ((status = W25_Read_Status()) & 0x01) {
        if (((xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS) >= TIMEOUT_CHIP_ERASE_MS)
            break;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    return (status & 0x01) ? -1 : 0;
}

//------------------------------------------------------------------------------
// update_flash_end_ptr
int update_flash_end_ptr(void) {
    uint16_t sector_header;
    for (uint32_t sec = 0; sec < TOTAL_SECTORS; sec++) {
        uint32_t sector_addr = sec * SECTOR_SIZE;
        if (W25_Read_Data(sector_addr, (uint8_t *)&sector_header, sizeof(sector_header)) != 0)
            continue;
        if (sector_header == SECTOR_EMPTY) {
            flash_end_ptr = sec * RECORDS_PER_SECTOR;
            return 0;
        } else if (sector_header == SECTOR_INUSE) {
            for (uint32_t i = 0; i < RECORDS_PER_SECTOR; i++) {
                uint32_t addr = sector_addr + SECTOR_HEADER_SIZE + i * RECORD_SIZE;
                // Читаем первые 2 байта
                uint8_t frag_status[2];
                if (W25_Read_Data(addr, frag_status, sizeof(frag_status)) != 0)
                    continue;
                // Если первый байт = 0xFF => свободно
                if (frag_status[0] == 0xFF) {
                    flash_end_ptr = sec * RECORDS_PER_SECTOR + i;
                    return 0;
                }
            }
            // Все фрагменты заполнены => SECTOR_FULL
            uint16_t full_val = SECTOR_FULL;
            if (W25_Write_Data(sector_addr, (uint8_t *)&full_val, sizeof(full_val)) != 0)
                return -1;
        }
    }
    // Если всё заполнено, wrap-around
    flash_end_ptr = 0;
    return 0;
}

//------------------------------------------------------------------------------
// flash_append_record: теперь учитываем, что data[] меньше на 2 байта (RECORD_SIZE - 10).
int flash_append_record(const char *record_data) {
    if (!record_data) return -1;
    size_t len = strlen(record_data);
    if (len > (RECORD_SIZE - 10))  // <-- здесь -10, а не -8
        len = RECORD_SIZE - 10;
    
    uint32_t record_index = flash_end_ptr;
    uint32_t sector = record_index / RECORDS_PER_SECTOR;
    uint32_t rec_in_sector = record_index % RECORDS_PER_SECTOR;
    uint32_t sector_addr = sector * SECTOR_SIZE;
    uint32_t record_addr = sector_addr + SECTOR_HEADER_SIZE + rec_in_sector * RECORD_SIZE;
    
    uint16_t sector_header;
    if (W25_Read_Data(sector_addr, (uint8_t *)&sector_header, sizeof(sector_header)) != 0)
        return -1;
    
    if (sector_header == SECTOR_EMPTY) {
        uint16_t new_header = SECTOR_INUSE;
        if (W25_Write_Data(sector_addr, (uint8_t *)&new_header, sizeof(new_header)) != 0)
            return -1;
    }
    
    int found_fragment = 0;
    for (uint32_t i = rec_in_sector; i < RECORDS_PER_SECTOR; i++) {
        uint32_t candidate_addr = sector_addr + SECTOR_HEADER_SIZE + i * RECORD_SIZE;
        uint8_t frag_status[2];
        if (W25_Read_Data(candidate_addr, frag_status, sizeof(frag_status)) != 0)
            continue;
        if (frag_status[0] == 0xFF && frag_status[1] == 0xFF) {
            flash_end_ptr = sector * RECORDS_PER_SECTOR + i;
            record_addr   = candidate_addr;
            found_fragment= 1;
            break;
        }
    }
    if (!found_fragment) {
        // Переход к следующему сектору
        uint32_t next_sector = (sector + 1) % TOTAL_SECTORS;
        uint32_t next_sector_addr = next_sector * SECTOR_SIZE;
        if (W25_Erase_Sector(next_sector_addr) != 0)
            return -1;
        uint16_t full_val = SECTOR_FULL;
        if (W25_Write_Data(sector_addr, (uint8_t *)&full_val, sizeof(full_val)) != 0)
            return -1;
        flash_end_ptr = next_sector * RECORDS_PER_SECTOR;
        sector = next_sector;
        rec_in_sector = 0;
        sector_addr = sector * SECTOR_SIZE;
        record_addr= sector_addr + SECTOR_HEADER_SIZE;
    }
    
    record_t new_rec;
    new_rec.length          = (uint16_t)len;
    new_rec.rec_status_start= 0x00;
    new_rec.rec_status_end  = 0xFF;
    // Добавленные поля: sent_marker_sector, sent_marker_block
    // По умолчанию = 0xFF (не отправлено)
    new_rec.sent_marker_sector = 0xFF;
    new_rec.sent_marker_block  = 0xFF;

    memset(new_rec.data, 0xFF, sizeof(new_rec.data));
    memcpy(new_rec.data, record_data, len);
    
    // Запись полностью (128 байт)
    if (W25_Write_Data(record_addr, (uint8_t *)&new_rec, sizeof(record_t)) != 0)
        return -1;
    
    // Обновляем rec_status_end -> RECORD_COMPLETE_FLAG
    uint8_t complete_flag = RECORD_COMPLETE_FLAG;
    uint32_t status_addr  = record_addr + offsetof(record_t, rec_status_end);
    if (W25_Write_Data(status_addr, &complete_flag, 1) != 0)
        return -1;
    
    flash_end_ptr++;
    return 0;
}

//------------------------------------------------------------------------------
// Чтение записи
int flash_read_record_by_index(uint32_t record_block, char *buffer) {
    if (!buffer || record_block >= TOTAL_RECORDS)
        return -1;
    
    uint32_t sector = record_block / RECORDS_PER_SECTOR;
    uint32_t rec_in_sector = record_block % RECORDS_PER_SECTOR;
    uint32_t addr = sector * SECTOR_SIZE + SECTOR_HEADER_SIZE + rec_in_sector * RECORD_SIZE;
    
    record_t rec;
    if (W25_Read_Data(addr, (uint8_t *)&rec, sizeof(record_t)) != 0)
        return -1;
    // Если первый байт == 0xFF => пусто
    if (((uint8_t *)&rec)[0] == 0xFF)
        return -1;
    
    // Длина не должна превосходить (RECORD_SIZE-10)
    uint16_t len = rec.length;
    if (len > (RECORD_SIZE - 10))
        len = RECORD_SIZE - 10;
    
        memcpy(buffer, rec.data, len);
        buffer[len] = '\0'; // завершаем строку
        
        // Флаги отсутствия скобок
        int no_open_bracket  = 0; 
        int no_close_bracket = 0;
        
        // 1) Проверяем/удаляем ведущий '['
        size_t l = strlen(buffer);
        if (l == 0 || buffer[0] != '[') {
            no_open_bracket = 1;
        } else {
            // если найдено '[', удаляем его, сдвигая строку
            memmove(buffer, buffer + 1, l); 
        }
        
        // 2) Проверяем/удаляем завершающий ']'
        l = strlen(buffer);
        if (l == 0 || buffer[l - 1] != ']') {
            no_close_bracket = 1;
        } else {
            // если найдено ']', заменяем на '\0'
            buffer[l - 1] = '\0';
        }
        
        // 3) Если отсутствует либо ведущая '[', либо завершающая ']', добавляем ";NO_WRITE"
        if (no_open_bracket || no_close_bracket) {
            strncat(buffer, ";NO_WRITE", RECORD_OUTPUT_SIZE - strlen(buffer) - 1);
        }
        
        if (rec.sent_marker_block != 0x00) {
            strncat(buffer, ";NO_SEND", RECORD_OUTPUT_SIZE - strlen(buffer) - 1);
        }
        // 5) В конце – добавляем перевод строки
        strncat(buffer, "\n", strlen(buffer));
        
        
    return 0;
}
void createFilename(char *dest, size_t destSize) {
    const char *version = EEPROM.version.VERSION_PCB;  // например, "3.75-A001V"
    char tmp[32];
    int j = 0;
    // Проходим по версии и копируем, если символ не '.' и не '-'
    for (int i = 0; version[i] != '\0' && j < 8; i++) {
        if (version[i] != '.' && version[i] != '-') {
            tmp[j++] = version[i];
        }
    }
    tmp[j] = '\0';

    // Формируем имя файла: префикс диска + полученная строка + ".csv"
    // Предполагаем, что destSize достаточен (например, 16 байт)
    snprintf(dest, destSize, "%s%s.csv", USBHPath, tmp);
}


//------------------------------------------------------------------------------
// backup_records_to_external – без изменений
int backup_records_to_external(void) {
    // Обновляем flash_end_ptr (если требуется)
    update_flash_end_ptr();
    // Если запись успешно считана, вызываем резервное копирование (например, выводим в консоль).
    FRESULT res;
    UINT bw;
    FATFS *fs;
    DWORD fre_clust;

    
    res = f_getfree(USBHPath, &fre_clust, &fs);
    if (res != FR_OK)
    {
        return -1;
    }
    

    char filename[16];
    createFilename(filename, sizeof(filename));
    res = f_open(&MyFile, filename, FA_OPEN_APPEND | FA_WRITE);
    if (res != FR_OK)
    {
        f_close(&MyFile);
        f_mount(NULL, USBHPath, 0);
        memset(&USBFatFs, 0, sizeof(FATFS));
        return -1;
    }



    // Определяем последний записанный фрагмент.
    uint32_t last_written;
    if (flash_end_ptr == 0)
        last_written = TOTAL_RECORDS - 1;
    else
        last_written = flash_end_ptr - 1;
    
    // Номер сектора, содержащего последнюю запись.
    uint32_t last_written_sector = last_written / RECORDS_PER_SECTOR;
    
    // Начинаем копирование с сектора, следующего за last_written_sector
    uint32_t start_sector = (last_written_sector + 1) % TOTAL_SECTORS;
    uint32_t current_sector = start_sector;
    
    // Буфер фиксированного размера для чтения записи.
    char save_data[RECORD_OUTPUT_SIZE] = {0};
    
    // Продолжаем копирование до того момента, когда пройдём сектор с последней записью.
    do {
        uint32_t sector_addr = current_sector * SECTOR_SIZE;
        uint16_t sector_header;
        if (W25_Read_Data(sector_addr, (uint8_t *)&sector_header, sizeof(sector_header)) != 0) {
            // Если ошибка чтения – пропускаем сектор
            goto next_sector;
        }
        // Если сектор помечен как пустой, пропускаем его.
        if (sector_header == SECTOR_EMPTY)
            goto next_sector;
        
        // Сектор содержит данные – сканируем все фрагменты этого сектора.
        for (uint32_t i = 0; i < RECORDS_PER_SECTOR; i++) {
            // Вычисляем глобальный индекс фрагмента
            uint32_t record_index = current_sector * RECORDS_PER_SECTOR + i;
            // Если мы находимся в секторе с последней записью, копировать только записи до last_written.
            if (current_sector == last_written_sector && record_index > last_written)
                break;
            
            // Считываем запись по индексу (функция flash_read_record_by_index не требует параметр размера)
            if (flash_read_record_by_index(record_index, save_data) == 0)
            {

                res = f_write(&MyFile, save_data, strlen(save_data), &bw);
                if(res != FR_OK)
                {
                    f_close(&MyFile);
                    f_mount(NULL, USBHPath, 0);
                    memset(&USBFatFs, 0, sizeof(FATFS));
                    return -1;
                }
            }
        }
        // ! Подозрительно
    next_sector:
        current_sector = (current_sector + 1) % TOTAL_SECTORS;
    } while (current_sector != start_sector);

    res = f_sync(&MyFile);
    if (res != FR_OK) {
        // Обработка ошибки синхронизации
    }
    //osDelay(200);
    f_close(&MyFile);
    //f_mount(NULL, USBHPath, 0);
    return 1;
}


//------------------------------------------------------------------------------
// Новая функция: mark_block_sent_and_check_sector
int mark_block_sent_and_check_sector(uint32_t record_block) {
    if (record_block >= TOTAL_RECORDS)
        return -1;
    
    // 1) Ставим sent_marker_block=0x00 для заданного блока
    uint32_t sector = record_block / RECORDS_PER_SECTOR;
    uint32_t blk_in_sec = record_block % RECORDS_PER_SECTOR;
    uint32_t block_addr = sector * SECTOR_SIZE + SECTOR_HEADER_SIZE + blk_in_sec * RECORD_SIZE;
    
    uint8_t zeroByte = 0x00;
    // offset = offsetof(record_t, sent_marker_block)
    uint32_t offset_sent_block = offsetof(record_t, sent_marker_block); // == 7
    if (W25_Write_Data(block_addr + offset_sent_block, &zeroByte, 1) != 0)
        return -1;
    
    // 2) Проверяем все блоки сектора: если все (не пустые) тоже имеют sent_marker_block=0x00,
    //    то в нулевом блоке сектора => sent_marker_sector=0x00
    uint32_t sector_addr = sector * SECTOR_SIZE;
    for (uint32_t i = 0; i < RECORDS_PER_SECTOR; i++) {
        uint32_t cur_block_addr = sector_addr + SECTOR_HEADER_SIZE + i * RECORD_SIZE;
        record_t rec;
        if (W25_Read_Data(cur_block_addr, (uint8_t *)&rec, sizeof(record_t)) != 0)
            continue;
        // Если первый байт=0xFF => блок пуст => пропускаем
        if (((uint8_t *)&rec)[0] == 0xFF)
            continue;
        // Если sent_marker_block != 0x00 => есть неотправленный
        if (rec.sent_marker_block != 0x00)
            return 0; 
    }
    // Если дошли сюда, все не-пустые блоки = sent_marker_block=0x00 => выставляем sent_marker_sector=0x00
    // Нулевой блок => offset= sector_addr+0*RECORD_SIZE +offsetof(record_t,sent_marker_sector)
    // Но учтите, sector_header=2 байта. 0-й блок => sector_addr +0?
    uint32_t zero_block_addr = sector_addr + SECTOR_HEADER_SIZE; // Адрес 0-го блока
    uint32_t offset_sent_sector = offsetof(record_t, sent_marker_sector); // == 6
    if (W25_Write_Data(zero_block_addr + offset_sent_sector, &zeroByte, 1) != 0)
        return -1;
    
    return 0;
}

int find_first_unsent_block_by_sector(uint32_t *record_block)
{
    if (!record_block) return -1;
    
    for (uint32_t sec = 0; sec < TOTAL_SECTORS; sec++) {
        // Начало сектора
        uint32_t sector_addr = sec * SECTOR_SIZE;
        
        // Считываем sector_header (2 байта в начале сектора)
        uint16_t sector_header;
        if (W25_Read_Data(sector_addr, (uint8_t *)&sector_header, sizeof(sector_header)) != 0) {
            // Ошибка чтения – пропускаем
            continue;
        }
        // Пропускаем пустые и полные сектора
        if (sector_header == SECTOR_EMPTY || sector_header == SECTOR_FULL) {
            // Сектор пустой (нет данных) или полный (но может быть неотправленный?
            // Логика у вас такая, что FULL обычно значит "все блоки заняты". 
            // Если "FULL" неотправленный, это case не рассмотрен – поправьте при необходимости.
            continue;
        }
        
        // Сектор "INUSE" (0x00FF) – проверим sent_marker_sector в **нулевом блоке** – байт[6].
        // Адрес нулевого блока = sector_addr + SECTOR_HEADER_SIZE
        uint32_t zero_block_addr = sector_addr + SECTOR_HEADER_SIZE;
        
        // Считываем sent_marker_sector (offset=6, т.к. offsetof(record_t, sent_marker_sector)==6)
        uint8_t sectorSentMarker;
        if (W25_Read_Data(zero_block_addr + offsetof(record_t, sent_marker_sector), &sectorSentMarker, 1) != 0) {
            continue; // ошибка чтения
        }
        // Если весь сектор уже отправлен, пропускаем
        if (sectorSentMarker == 0x00) {
            // весь сектор = отправлен
            continue;
        }
        
        // sectorSentMarker=0xFF => не отправлен целиком. Теперь перебираем блоки
        // ищем первый неотправленный (sent_marker_block==0xFF)
        int found_unsent_in_sector = 0; // флаг
        for (uint32_t i = 0; i < RECORDS_PER_SECTOR; i++) {
            uint32_t block_addr = sector_addr + SECTOR_HEADER_SIZE + i * RECORD_SIZE;
            // Проверим пуст ли блок (первый байт структуры, rec[0] == 0xFF)
            uint8_t firstByte;
            if (W25_Read_Data(block_addr, &firstByte, 1) != 0) {
                continue; // ошибка чтения – пропускаем
            }
            if (firstByte == 0xFF) {
                // блок пуст
                continue;
            }
            // Блок не пуст – считываем sent_marker_block
            uint8_t blockSentMarker;
            if (W25_Read_Data(block_addr + offsetof(record_t, sent_marker_block),
                              &blockSentMarker, 1) != 0) {
                continue;
            }
            if (blockSentMarker == 0xFF) {
                // Н найден – неотправленный блок
                *record_block = sec * RECORDS_PER_SECTOR + i;
                return 0; // возвращаем успех
            }
        }
        
        // Если цикл прошёл, но не найден ни один блок, имеющий blockSentMarker==0xFF,
        // значит все заполненные блоки уже отправлены => ставим sectorSentMarker=0x00
        {
            uint8_t zeroByte = 0x00;
            if (W25_Write_Data(zero_block_addr + offsetof(record_t, sent_marker_sector),
                               &zeroByte, 1) != 0) {
                continue;
            }
        }
        // И идём к следующему сектору
    }
    
    // Если дошли сюда, неотправленных блоков не найдено
    return -1;
}
