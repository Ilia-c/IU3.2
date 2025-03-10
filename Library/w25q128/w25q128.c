#include "w25q128.h"
#include <string.h>
#include <stddef.h>

// Макросы управления CS (Chip Select)
#define cs_set()   HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, 0)
#define cs_reset() HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, 1)

extern SPI_HandleTypeDef hspi2;
uint32_t flash_end_ptr = 0;

// Функции-обёртки для SPI (возвращают 0 при успехе, -1 при ошибке)
static int SPI2_Send(uint8_t *dt, uint16_t cnt) {
    return (HAL_SPI_Transmit(&hspi2, dt, cnt, 1000) == HAL_OK) ? 0 : -1;
}

static int SPI2_Recv(uint8_t *dt, uint16_t cnt) {
    return (HAL_SPI_Receive(&hspi2, dt, cnt, 1000) == HAL_OK) ? 0 : -1;
}

//------------------------------------------------------------------------------
// w25_init: Сброс флеша, проверка ID и обновление flash_end_ptr.
void w25_init(void) {
    // Сброс флеша
    if (W25_Reset() != 0) {
        // Можно установить код ошибки в Status_codes, если требуется.
        return;
    }
    HAL_Delay(100);
    
    // Чтение ID
    uint32_t id = 0;
    if (W25_Read_ID(&id) != 0) {
        // Отметить ошибку и выйти
        return;
    }
    // Проверяем ID (например, для W25Q128 должен быть 0xEF4018)
    if (id != 0xEF4018) {
        // Установить код ошибки в Status_codes.h (например, ERRCODE.STATUS |= STATUS_FLASH_INIT_ERROR)
        return;
    }
    
    // Обновляем указатель на свободный фрагмент
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
    HAL_Delay(2);
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
// Ожидание готовности флеша (с использованием FreeRTOS)
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
// Стирание сектора (адрес должен быть выровнен по SECTOR_SIZE)
int W25_Erase_Sector(uint32_t addr) {
    uint8_t cmd[4];
    cmd[0] = W25_SECTOR_ERASE;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;
    if (W25_Write_Enable() != 0) return -1;
    cs_set();
    HAL_Delay(2);
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
// Полное стирание флеша (Chip Erase)
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
// update_flash_end_ptr: Последовательное сканирование всех секторов для определения первого свободного фрагмента.
// Если заголовок сектора равен SECTOR_EMPTY (0xFFFF), устанавливаем flash_end_ptr на начало сектора.
// Если заголовок равен SECTOR_INUSE (0x00FF), сканируем фрагменты (начиная с SECTOR_HEADER_SIZE) и ищем первый свободный фрагмент.
// Если все фрагменты заполнены, помечаем сектор как полный (SECTOR_FULL) и продолжаем.
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
            // Сканируем фрагменты в секторе
            for (uint32_t i = 0; i < RECORDS_PER_SECTOR; i++) {
                uint32_t addr = sector_addr + SECTOR_HEADER_SIZE + i * RECORD_SIZE;
                uint8_t frag_status[2];
                if (W25_Read_Data(addr, frag_status, sizeof(frag_status)) != 0)
                    continue;
                // Если первый байт равен 0xFF, фрагмент свободен.
                // Если фрагмент имеет значение 0x00 в первом байте и второй 0xFF – запись была начата, считаем его заполненным.
                if (frag_status[0] == 0xFF)
                {
                    flash_end_ptr = sec * RECORDS_PER_SECTOR + i;
                    return 0;
                }
            }
            // Все фрагменты заполнены – помечаем сектор как полный.
            uint16_t full_val = SECTOR_FULL;
            if (W25_Write_Data(sector_addr, (uint8_t *)&full_val, sizeof(full_val)) != 0)
                return -1;
        }
    }
    // Если все сектора заполнены, начинаем с первого сектора.
    flash_end_ptr = 0;
    return 0;
}

//------------------------------------------------------------------------------
// flash_append_record: Запись новой 128-байтной записи в последнюю свободную ячейку.
// Алгоритм:
// 1. Определяем текущий фрагмент по flash_end_ptr.
// 2. Считываем заголовок сектора. Если сектор пустой, обновляем его до SECTOR_INUSE.
// 3. Считываем первые 2 байта целевого фрагмента. Если фрагмент занят, переходим к следующему сектору:
//    - Определяем следующий сектор (с wrap-around).
//    - Стираем следующий сектор, чтобы он стал пустым.
//    - Помечаем текущий сектор как заполненный (SECTOR_FULL).
//    - Обновляем flash_end_ptr на начало нового сектора.
// 4. Формируем запись: заполняем length, rec_status_start (0x00), rec_status_end (0xFF) и data.
// 5. Записываем запись в фрагмент, затем обновляем rec_status_end до RECORD_COMPLETE_FLAG (0xAA).
// 6. Обновляем flash_end_ptr.
int flash_append_record(const char *record_data) {
    if (!record_data)
        return -1;
    size_t len = strlen(record_data);
    if (len > (RECORD_SIZE - 8))
        len = RECORD_SIZE - 8;
    
    // Определяем текущую позицию по flash_end_ptr
    uint32_t record_index = flash_end_ptr;
    uint32_t sector = record_index / RECORDS_PER_SECTOR;
    uint32_t rec_in_sector = record_index % RECORDS_PER_SECTOR;
    uint32_t sector_addr = sector * SECTOR_SIZE;
    uint32_t record_addr = sector_addr + SECTOR_HEADER_SIZE + rec_in_sector * RECORD_SIZE;
    
    // Считываем 2-байтовый заголовок сектора
    uint16_t sector_header;
    if (W25_Read_Data(sector_addr, (uint8_t *)&sector_header, sizeof(sector_header)) != 0)
        return -1;
    
    // Если сектор пустой, обновляем его заголовок до SECTOR_INUSE (0x00FF)
    if (sector_header == SECTOR_EMPTY) {
        uint16_t new_header = SECTOR_INUSE;
        if (W25_Write_Data(sector_addr, (uint8_t *)&new_header, sizeof(new_header)) != 0)
            return -1;
    }
    
    // Вместо проверки только текущего фрагмента, ищем свободный фрагмент в текущем секторе,
    // начиная с позиции rec_in_sector.
    int found_fragment = 0;
    for (uint32_t i = rec_in_sector; i < RECORDS_PER_SECTOR; i++) {
        uint32_t candidate_addr = sector_addr + SECTOR_HEADER_SIZE + i * RECORD_SIZE;
        uint8_t frag_status[2];
        if (W25_Read_Data(candidate_addr, frag_status, sizeof(frag_status)) != 0)
            continue;
        // Если первые 2 байта равны 0xFF, фрагмент считается свободным.
        if (frag_status[0] == 0xFF && frag_status[1] == 0xFF) {
            flash_end_ptr = sector * RECORDS_PER_SECTOR + i;
            record_addr = candidate_addr;
            found_fragment = 1;
            break;
        }
    }
    
    // Если ни один фрагмент в текущем секторе свободен, сектор считается заполненным.
    if (!found_fragment) {
        // Перед переходом к следующему сектору очищаем следующий сектор, чтобы гарантировать, 
        // что всегда будет хотя бы один пустой сектор.
        uint32_t next_sector = (sector + 1) % TOTAL_SECTORS;
        uint32_t next_sector_addr = next_sector * SECTOR_SIZE;
        if (W25_Erase_Sector(next_sector_addr) != 0)
            return -1;
        // Помечаем текущий сектор как заполненный
        uint16_t full_val = SECTOR_FULL;
        if (W25_Write_Data(sector_addr, (uint8_t *)&full_val, sizeof(full_val)) != 0)
            return -1;
        // Обновляем flash_end_ptr на начало нового сектора
        flash_end_ptr = next_sector * RECORDS_PER_SECTOR;
        sector = flash_end_ptr / RECORDS_PER_SECTOR;
        rec_in_sector = 0;
        sector_addr = sector * SECTOR_SIZE;
        record_addr = sector_addr + SECTOR_HEADER_SIZE;
    }
    
    // Формируем новую запись (фиксированный размер 128 байт)
    record_t new_rec;
    // Поле length – длина полезных данных (максимум 122 байта)
    new_rec.length = (uint16_t)len;
    // rec_status_start устанавливается в 0x00 (запись начата)
    new_rec.rec_status_start = 0x00;
    // rec_status_end изначально 0xFF (запись не завершена)
    new_rec.rec_status_end = 0xFF;
    // Если это первая запись в секторе, первый фрагмент уже содержит заголовок сектора,
    // поэтому никаких дополнительных действий не требуются.
    memset(new_rec.data, 0xFF, sizeof(new_rec.data));
    memcpy(new_rec.data, record_data, len);
    
    // Записываем новую 128-байтную запись в найденный фрагмент
    if (W25_Write_Data(record_addr, (uint8_t *)&new_rec, sizeof(record_t)) != 0)
        return -1;
    
    // После успешной записи обновляем поле rec_status_end до RECORD_COMPLETE_FLAG (0xAA)
    uint8_t complete_flag = RECORD_COMPLETE_FLAG;
    uint32_t status_addr = record_addr + offsetof(record_t, rec_status_end);
    if (W25_Write_Data(status_addr, &complete_flag, 1) != 0)
        return -1;
    
    // Обновляем глобальный указатель: переходим к следующему фрагменту
    flash_end_ptr++;
    return 0;
}


//------------------------------------------------------------------------------
// Чтение записи по физическому номеру фрагмента (от 0 до TOTAL_RECORDS-1)
int flash_read_record_by_index(uint32_t record_block, char *buffer) {
    if (!buffer || record_block >= TOTAL_RECORDS)
        return -1;
    
    uint32_t sector = record_block / RECORDS_PER_SECTOR;
    uint32_t rec_in_sector = record_block % RECORDS_PER_SECTOR;
    uint32_t addr = sector * SECTOR_SIZE + SECTOR_HEADER_SIZE + rec_in_sector * RECORD_SIZE;
    record_t rec;
    if (W25_Read_Data(addr, (uint8_t *)&rec, sizeof(record_t)) != 0)
        return -1;
    if (((uint8_t *)&rec)[0] == 0xFF)
        return -1;
    
    // Количество байт данных не должно превышать размер полезной области.
    uint16_t len = rec.length;
    if (len > (RECORD_SIZE - 8))
        len = RECORD_SIZE - 8;
    
    memcpy(buffer, rec.data, len);
    buffer[len] = '\0';
    
    // Если запись не завершена, дописываем метку ошибки.
    if (rec.rec_status_end != RECORD_COMPLETE_FLAG) {
        strncat(buffer, ";err flasg\n", RECORD_OUTPUT_SIZE - strlen(buffer) - 1);
    }
    
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

// Возвраты: -1 - ошибка USB,  0 - Ошибка чтения 1 - все ок
int backup_records_to_external(void) {
    // Обновляем flash_end_ptr (если требуется)
    update_flash_end_ptr();
    // Если запись успешно считана, вызываем резервное копирование (например, выводим в консоль).
    FRESULT res;
    UINT bw;
    f_mount(NULL, USBHPath, 0);
    osDelay(100);
    if(disk_initialize(0) != RES_OK) {
        // Если диск не инициализируется, возвращаем ошибку
        return -1;
    }
    //f_mount(NULL, (TCHAR const* )"", 0);
    res = f_mount(&USBFatFs, USBHPath, 0);
    if (res != FR_OK)
    {
        // Если файловую систему не смонтировать, выходим
        // ! Вывести сообщение об ошибке
        f_mount(NULL, "", 0);
        return -1;
    }

    char filename[16];
    createFilename(filename, sizeof(filename));
    res = f_open(&MyFile, filename, FA_OPEN_APPEND | FA_WRITE);
    if (res != FR_OK)
    {
        f_close(&MyFile);
        f_mount(NULL, USBHPath, 0);
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
                char *ptr = save_data;
                // Если первый символ равен '[', пропускаем его, сдвигая указатель
                if (ptr[0] == '[') {
                    ptr++;
                }
                
                // Если строка пустая после сдвига, пропускаем запись
                if (strlen(ptr) == 0) continue;
                // Определяем текущую длину строки, на которую указывает ptr
                size_t len = strlen(ptr);
                
                // Если последний символ равен ']', заменяем его на перевод строки '\n'
                if (len > 0 && ptr[len - 1] == ']') {
                    ptr[len - 1] = '\n';
                    // Если длина строки меньше размера буфера, гарантируем корректное завершение строки
                    if (len < RECORD_OUTPUT_SIZE) {
                        ptr[len] = '\0';
                    }
                    // На всякий случай, чтобы гарантировать, что последний символ в буфере – '\0'
                    ptr[RECORD_OUTPUT_SIZE - 1] = '\0';
                }
                
                // Пишем обработанную строку в файл
                res = f_write(&MyFile, ptr, strlen(ptr), &bw);
                if(res != FR_OK)
                {
                    f_close(&MyFile);
                    f_mount(NULL, USBHPath, 0);
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
    osDelay(200);
    f_close(&MyFile);
    f_mount(NULL, USBHPath, 0);
    return 1;
}
