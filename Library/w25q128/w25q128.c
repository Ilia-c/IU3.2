#include "w25q128.h"
#include "USB_FATFS_SAVE.h"   // ���� ��� ����� ��������� USB-�������� �������
#include "Settings.h"         // ���� � ����� ������� EEPROM/version � �.�.

extern SPI_HandleTypeDef hspi2;
extern USBH_HandleTypeDef hUsbHostFS;
extern ApplicationTypeDef Appli_state;
extern FATFS USBFatFs;    /* �������� ������� ��� USB */
extern FIL MyFile;        /* ������ ����� */
extern char USBHPath[4];  /* ���������� ���� USB ����� */
extern EEPROM_Settings_item EEPROM;
extern RTC_HandleTypeDef hrtc;
extern CRC_HandleTypeDef hcrc;

// ���������� CS (Chip Select)
#define cs_set()   HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, GPIO_PIN_RESET)
#define cs_reset() HAL_GPIO_WritePin(SPI2_CS_ROM_GPIO_Port, SPI2_CS_ROM_Pin, GPIO_PIN_SET)

uint32_t g_total_records_count = 0;

// ------------------ ��������������� SPI-������� ------------------
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
// ������������� �����: �����, �������� ID
void w25_init(void)
{
    USB_DEBUG_MESSAGE("[DEBUG FLASH] ������������� FLASH", DEBUG_FLASH, DEBUG_LEVL_1);
    if (W25_Reset() != 0) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������ ������", DEBUG_FLASH, DEBUG_LEVL_1);
        return;
    }
    HAL_Delay(100);

    uint32_t id = 0;
    USB_DEBUG_MESSAGE("[DEBUG FLASH] ������ ID", DEBUG_FLASH, DEBUG_LEVL_3);
    W25_Read_ID(&id);
    if (id != 0xef4018)
    {
        USB_DEBUG_MESSAGE("[ERROR FLASH] �������� ID", DEBUG_FLASH, DEBUG_LEVL_1);
        ERRCODE.STATUS |= STATUS_FLASH_INIT_ERROR;
    }
    else
    {
        USB_DEBUG_MESSAGE("[DEBUG FLASH] FLASH ���������������", DEBUG_FLASH, DEBUG_LEVL_1);
        ERRCODE.STATUS &= ~STATUS_FLASH_INIT_ERROR;
    }
}

//------------------------------------------------------------------------------
// ������ ID �����
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
// ����� �����
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
// ������ ������ �� �����
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
// ���������� ������
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
// �������� ���������� ����� (FreeRTOS)
int W25_WaitForReady(uint32_t timeout_ms)
{
    TickType_t start_tick = xTaskGetTickCount();
    while (((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS) < timeout_ms) {
        uint8_t status = W25_Read_Status();
        if ((status & 0x01) == 0) {
            return 0;
        }
    }
    return -1;
}

//------------------------------------------------------------------------------
// ������ ���������� ��������
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
// ������ ������ �� ���� (Page Program)
int W25_Write_Data(uint32_t addr, uint8_t *data, uint32_t sz)
{
    if (!data) 
    return -1;

    // ������������, ��� sz <= 256 � �� ���������� ������� ��������
    // (�.�. �� ������ ����� 128 ���� �� ���).
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

    if (W25_WaitForReady(500) != 0) {
        return -1;
    }
    return 0;
}

//------------------------------------------------------------------------------
// �������� ������� (4 ��)
int W25_Erase_Sector(uint32_t addr)
{
    USB_DEBUG_MESSAGE("[DEBUG FLASH] ������ �������� ������� FLASH", DEBUG_FLASH, DEBUG_LEVL_3);
    uint8_t cmd[4];
    cmd[0] = W25_SECTOR_ERASE;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    if (W25_Write_Enable() != 0) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������ ���������� ������ ����� ���������", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }

    cs_set();
    if (SPI2_Send(cmd, 4) != 0) {
        cs_reset();
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������ �������� ������� ��������", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }
    cs_reset();

    // ��� ~400 ��, ���� �������
    if (W25_WaitForReady(400) != 0) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������ �������� ���������� ����� ��������", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }
    USB_DEBUG_MESSAGE("[DEBUG FLASH] �������� FLASH ���������", DEBUG_FLASH, DEBUG_LEVL_3);
    return 0;
}

//------------------------------------------------------------------------------
// ������ �������� �����
int W25_Chip_Erase(void)
{
    USB_DEBUG_MESSAGE("[DEBUG FLASH] ������ ������ �������� FLASH", DEBUG_FLASH, DEBUG_LEVL_3);
    uint8_t cmd = W25_CHIP_ERASE;
    if (W25_Write_Enable() != 0){
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������ ���������� ������ ����� ������ ���������", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }

    cs_set();
    if (SPI2_Send(&cmd, 1) != 0) {
        cs_reset();
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������ �������� ������� ������� ��������", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }
    cs_reset();

    TickType_t start_time = xTaskGetTickCount();
    uint8_t status = 0;
    while ((status = W25_Read_Status()) & 0x01) {
        // ��������� �������
        if (((xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS) >= TIMEOUT_CHIP_ERASE_MS) {
            USB_DEBUG_MESSAGE("[ERROR FLASH] ������� ��� �������� ������� ��������", DEBUG_FLASH, DEBUG_LEVL_2);
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    g_total_records_count = 0;
    if (status & 0x01) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������ ��� ������ �������� FLASH", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }
    USB_DEBUG_MESSAGE("[DEBUG FLASH] ������ �������� FLASH ���������", DEBUG_FLASH, DEBUG_LEVL_3);
    return 0;
    //return ((status & 0x01) ? -1 : 0);
}

uint32_t W25_Chip_test()
{
    // ���������������, ��� W25_Chip_Erase ��� ��������

    uint32_t id = 0;
    uint8_t step = 0;
    uint8_t Counter_addr_error = 0;
    if (W25_Read_ID(&id) != 0)
    {
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������ ������ ID", DEBUG_FLASH, DEBUG_LEVL_2);
        return 0xFFFFFFFF; // ������
    }
    // ��������, ��� ��� ������� ������
    uint8_t sector[RECORD_SIZE] = {0};
    PROGRESS_BAR(0);
    OLED_UpdateScreen();
    for (uint16_t sec = 0; sec < FLASH_SIZE/SECTOR_SIZE; sec++)
    {
        uint32_t sector_addr = sec * SECTOR_SIZE;
        for (uint32_t addr = sector_addr; addr < sector_addr + SECTOR_SIZE; addr += RECORD_SIZE)
        {
            if (W25_Read_Data(addr, sector, RECORD_SIZE) != 0)
            {
                USB_DEBUG_MESSAGE("[ERROR FLASH] ������ ������ �������", DEBUG_FLASH, DEBUG_LEVL_2);
                return 0xFFFFFFFF; // ������
            }
            // ���������, ��� ������ ������
            for (uint32_t i = 0; i < RECORD_SIZE; i++)
            {
                if (sector[i] != EMPTY)
                {
                    Counter_addr_error++;
                }
            }
        }
        uint8_t p = (uint8_t)(sec * 50 / (FLASH_SIZE / SECTOR_SIZE));
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
        OLED_DrawCenteredString("�� ��� ������� ������", 25);
        OLED_UpdateScreen();
        osDelay(10000);
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������: �� ��� ������� ������", DEBUG_FLASH, DEBUG_LEVL_2);
        return Counter_addr_error; // ������
    }
    step = 0;
    for (uint16_t sec = 0; sec < FLASH_SIZE/SECTOR_SIZE; sec++)
    {
        uint32_t sector_addr = sec * SECTOR_SIZE;
        for (uint32_t addr = sector_addr; addr < sector_addr + SECTOR_SIZE; addr += RECORD_SIZE)
        {
            for (uint8_t i = 0; i < RECORD_SIZE; i++)
            {
                sector[i] = SET; // ��������� ������ ������� 0x00
            }

            if (W25_Write_Data(addr, &sector, RECORD_SIZE) != 0)
            {
                return 0xFFFFFFFF; // ������
            }

            if (W25_Read_Data(addr, sector, RECORD_SIZE) != 0)
            {
                USB_DEBUG_MESSAGE("[ERROR FLASH] ������ ������ �������", DEBUG_FLASH, DEBUG_LEVL_2);
                return 0xFFFFFFFF; // ������
            }
            // ���������, ��� ������ ������
            for (uint32_t i = 0; i < RECORD_SIZE; i++)
            {
                if (sector[i] != SET)
                {
                    Counter_addr_error++;
                }
            }
        }
        uint8_t p = (uint8_t)(sec * 50 / (FLASH_SIZE / SECTOR_SIZE));
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
// ���������� ����� ���������� ����� � "������" ������� ��� ������� ������ �0, ���� �� ���������.
int32_t search_sector_empty(void)
{
    USB_DEBUG_MESSAGE("[DEBUG FLASH] ����� ���������� ����� � ��������", DEBUG_FLASH, DEBUG_LEVL_3);
    uint8_t sector_header;  // ������ ���� �������
    for (uint16_t sec = 0; sec < TOTAL_SECTORS; sec++) {
        uint32_t sector_addr = sec * SECTOR_SIZE;

        // ��������� 1 ���� (Sector_mark)
        if (W25_Read_Data(sector_addr, &sector_header, 1) != 0) {
            // ������ ������ - ����������
            continue;
        }

        if (sector_header != SET) {
            // �������, ��� ������ "����������� ��������"
            // ���� � ������� ��������� ����
            for (uint32_t i = 0; i < RECORDS_PER_SECTOR; i++) {
                uint32_t addr = sector_addr + BLOCK_MARK_WRITE_START + i * RECORD_SIZE;
                uint8_t frag_status;
                if (W25_Read_Data(addr, &frag_status, 1) != 0) {
                    // ������ ������ - ����������
                    ERRCODE.STATUS |= STATUS_FLASH_CRC_ERROR;
                    continue;
                }
                if (frag_status == EMPTY) {
                    // ����� ��������� ����
                    g_total_records_count = sec * RECORDS_PER_SECTOR + i;
                    USB_DEBUG_MESSAGE("[DEBUG FLASH] ������ ��������� ����", DEBUG_FLASH, DEBUG_LEVL_3);
                    return (int32_t)(addr - BLOCK_MARK_WRITE_START);
                }
            }
            // ���� ���� ������ �� ����� ���� ��������, ������ 0x00 (SECTOR_MARK=SET)
            uint8_t full_val = SET; // 0x00
            if (W25_Write_Data(sector_addr, &full_val, 1) == 0) {
                // ��������, ��� ������ ������
            }
        }
    }

    // ���� �� ���������, ������� ������ �0 (������), ���������� ����� ������
    if (W25_Erase_Sector(0) != 0) {
        // ������ �������� - ���������� -1
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������ �������� ������� �0", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }

    // ������ ������ �0 ����, ����� ������� 0 (����� ������)
    USB_DEBUG_MESSAGE("[DEBUG FLASH] ������ �0 ������� ����", DEBUG_FLASH, DEBUG_LEVL_3);
    return 0;
}

//------------------------------------------------------------------------------
// ������ ������ �� ���� 
int flash_append_record(const char *record_data, uint8_t sector_mark_send_flag)
{
    USB_DEBUG_MESSAGE("[DEBUG FLASH] ������ ������ �� ����", DEBUG_FLASH, DEBUG_LEVL_3);
    int32_t addr = search_sector_empty();
    if (addr < 0 || addr >= FLASH_TOTAL_SIZE){
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������ ������ ���������� �����", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }
    if (!record_data){
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������ ��� ������ �� �������", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }

    // �������� ����� ������, �������� (RECORD_SIZE - 6)
    size_t len = strlen(record_data);
    if (len > (RECORD_SIZE - 6)) {
        len = RECORD_SIZE - 6;
    }
    
    // ���������� ���������
    record_t new_rec;
    // �� ����� ������:
    new_rec.Sector_mark       = WRITE_START; // [0] 0xF0 
    new_rec.Sector_mark_send  = EMPTY; // [1]
    new_rec.rec_status_start  = SET;   // [2] => 0x00
    new_rec.rec_status_end    = EMPTY; // [3] => 0xFF
    if (sector_mark_send_flag) new_rec.Sector_mark_send = SET; // 0x00
    else  new_rec.block_mark_send   = EMPTY; // [4] => 0xFF
    new_rec.length            = (uint8_t)len; // [5]
    new_rec.CRC32_calc        = 0; // [6-10] �������������� CRC
    memset(new_rec.data, 0x00, sizeof(new_rec.data));
    memcpy(new_rec.data, record_data, len);

    uint32_t word_count = (len + 3) / 4;
    uint32_t *p = (uint32_t *)new_rec.data;
    new_rec.CRC32_calc = HAL_CRC_Calculate(&hcrc, p, word_count);
    if (len == 0) new_rec.CRC32_calc = 0xFFFFFFFF;

    // ������� 128 ���� � ����
    if (W25_Write_Data(addr, (uint8_t *)&new_rec, sizeof(record_t)) != 0) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] ������ ������ ������ �� ����", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }

    // ������ rec_status_end = 0x00 => ������ ���������
    {
        uint8_t complete_flag = SET; // 0x00
        uint32_t status_addr = addr + offsetof(record_t, rec_status_end);
        if (W25_Write_Data(status_addr, &complete_flag, 1) != 0) {
            USB_DEBUG_MESSAGE("[ERROR FLASH] ������ ������ ������� ���������� ������", DEBUG_FLASH, DEBUG_LEVL_2);
            return -1;
        }
    }

    // ---- �������������� ���: ���� ��� ��������� ���� ������� => ������� ��������� ������, ���� �� �� ������ ----

    // 1) ������� ������ ������� � ���� ������ �������
    uint32_t sector_index = (uint32_t)addr / SECTOR_SIZE;
    uint32_t block_index = ((uint32_t)addr % SECTOR_SIZE) / RECORD_SIZE;

    if (block_index == (RECORDS_PER_SECTOR - 1))
    {
        // ��� ��������� ���� �������
        uint32_t next_sector = (sector_index + 1) % TOTAL_SECTORS;
        uint32_t next_sector_addr = next_sector * SECTOR_SIZE;

        if (next_sector_addr >= FLASH_TOTAL_SIZE)
        {
            next_sector_addr = 0;
            ERRCODE.STATUS |= STATUS_FLASH_OVERFLOW_ERROR;
        }
        // ������ ���� [0] � ��������� �������
        uint8_t next_sector_mark = 0xFF;
        if (W25_Read_Data(next_sector_addr, &next_sector_mark, 1) == 0)
        {
            // ���� != 0xFF, ������ ������ �� ������ => �������
            if (next_sector_mark != EMPTY)
            {
                USB_DEBUG_MESSAGE("[DEBUG FLASH] ������� ��������� ������, �.�. �� �� ������", DEBUG_FLASH, DEBUG_LEVL_3);
                W25_Erase_Sector(next_sector_addr);
            }
        }
    }
    USB_DEBUG_MESSAGE("[DEBUG FLASH] ������ ������ �� ���� ���������", DEBUG_FLASH, DEBUG_LEVL_3);
    return 0;
}

//------------------------------------------------------------------------------
// ������� ���������� ����� ��� "�������������" + ��������, ��������� �� ���� ������
int mark_block_sent(int32_t addr_block)
{
    USB_DEBUG_MESSAGE("[DEBUG FLASH] ������� ����� ��� �������������", DEBUG_FLASH, DEBUG_LEVL_4);
    // �������� ������
    if (addr_block < 0 || addr_block > (int32_t)(FLASH_TOTAL_SIZE - RECORD_SIZE)) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] ����� ����� ��� ����������� ���������", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }
    // ��������, ��� �������� � ������ �����
    if ((addr_block % RECORD_SIZE) != 0) {
        USB_DEBUG_MESSAGE("[ERROR FLASH] ����� ����� ������ ���� ������ ������� ������", DEBUG_FLASH, DEBUG_LEVL_2);
        return -1;
    }

    // ���������, �� ���� �� ����
    uint8_t read_byte = 0xFF;
    if (W25_Read_Data(addr_block, &read_byte, 1) != 0) {
        return -1;
    }
    if (read_byte == EMPTY) {
        // ������ ����
        return 0;
    }

    // ������ "block_mark_send" = 0x00
    uint8_t block_send = SET; // 0x00
    if (W25_Write_Data(addr_block + BLOCK_MARK_DATA_SEND, &block_send, 1) != 0) {
        return -1;
    }

    // ������ ���������, ��� �� ����� ����� ������� ����������
    uint32_t sector_idx  = (uint32_t)addr_block / SECTOR_SIZE;
    uint32_t sector_addr = sector_idx * SECTOR_SIZE;
    uint8_t marker = 0;  // 0 => ��� ����� ����������, 0xFF => ���� ��������������

    for (uint32_t i = 0; i < RECORDS_PER_SECTOR; i++) {
        uint32_t cur_block_addr = sector_addr + BLOCK_MARK_DATA_SEND + i * RECORD_SIZE;
        uint8_t mark_val = 0xFF;
        if (W25_Read_Data(cur_block_addr, &mark_val, 1) != 0) {
            // ������ ������ - ����������
            continue;
        }
        // ���� block_mark_send = 0xFF, ������ �� ���������
        // �� ���� ��� ��������, ���� �� ���� (���� �� ������� ������)
        // ��� ��������, ���� mark_val == SET => ���� ���� ��� ������� ��� ������������. 
        // ����� => 0xFF = ��������������
        if (mark_val == EMPTY) {
            // ����� ���� ����? ���������. 
            continue;
        }
        // ���� ��� 0xFF => ���� ����, �� �� ���������
        if (mark_val == EMPTY) {
            // ������
        } else if (mark_val != SET) {
            // ���� ���-�� ��������, ���� ������� ��������������
            marker = 0xFF;
            break;
        } else {
            // mark_val == SET => OK, ��� ���������
        }
        if (mark_val == 0xFF) {
            marker = 0xFF;
            break;
        }
    }

    // ���� marker == 0 => ��� ����� ���������� => ������ sector_mark_send = 0x00
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
    save_data[len_data-1] = '\n'; // ��������
    save_data[len_data] = '\0'; // ��������  
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
// ������� ����������� ������ �� ������� ��������� (USB)
int backup_records_to_external(void)
{
    // ������ ������ ���� � ������������ ��������

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
    // ��������, flash_end_ptr �� ������������, ����� ����� ������ �� � last_written / start_sector.

    // ��������� ��� �������
    record_t rec;
    osDelay(300);
    memset(&rec, 0, sizeof(rec));
    osDelay(100);

    for (uint16_t sec = 0; sec < TOTAL_SECTORS; sec++) {
        uint32_t sector_addr = sec * SECTOR_SIZE;
        uint8_t sector_header = 0xFF;
        if (W25_Read_Data(sector_addr, &sector_header, 1) != 0) {
            // ������ ������ - ����������
            continue;
        }
        // ���� ������ "������" (0xFF) - ����������
        if (sector_header == EMPTY) {
            continue;
        }

        // ����� ���������� �����
        for (uint32_t i = 0; i < RECORDS_PER_SECTOR; i++) {
            // ����� "rec_status_start" (��������=2)
            uint32_t block_addr = sector_addr + (i * RECORD_SIZE) + BLOCK_MARK_WRITE_START;
            uint8_t block_header = 0xFF;
            if (W25_Read_Data(block_addr, &block_header, 1) != 0) {
                ERRCODE.STATUS |= STATUS_USB_FLASH_READ_ERROR;
                // ������ ������
                continue;
            }
            // ���� ���� ���� ������, ����������
            if (block_header == EMPTY) {
                continue;
            }

            // ������ ���� ����
            // ��������� ����� �� 2 �����
            block_addr -= BLOCK_MARK_WRITE_START;
            if (W25_Read_Data(block_addr, (uint8_t *)&rec, sizeof(rec)) != 0)
            {
                ERRCODE.STATUS |= STATUS_USB_FLASH_READ_ERROR;
                continue;
            }
            size_t realLen = rec.length;
            if (realLen > 110)
            {
                rec.data[110] = '\n'; // ��������
                rec.data[111] = '\0'; // ��������
            }
            else
            {
                rec.data[realLen - 1] = '\n'; // ��������
                rec.data[realLen] = '\0';     // ��������
            }
            // ���� ������ �� �������� - ����������� data+";data_error\n"
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
                else
                {
                    // �������� CRC32
                    uint32_t word_count = (rec.length + 3) / 4;
                    uint32_t *p = (uint32_t *)rec.data;
                    uint32_t CRC32_clac = HAL_CRC_Calculate(&hcrc, p, word_count);
                    if (rec.length == 0)
                        CRC32_clac = 0xFFFFFFFF;
                    if (CRC32_clac != rec.CRC32_calc)
                    {
                        strcpy(&rec.data[realLen - 1], ";CRC_ERR\n");
                    }
                }
            }

            // ���������� � ����
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
        // ������ �������������
    }
    ERRCODE.STATUS &= ~STATUS_USB_FLASH_SYNC_ERROR;
    f_close(&MyFile);
    // f_mount(NULL, USBHPath, 0); // ���� �����
    return 1;
}

//------------------------------------------------------------------------------
// ������ �������� ����� �����
void createFilename(char *dest, size_t destSize)
{
    // ������������, ��� EEPROM.version.VERSION_PCB ����������
    char Version[20] = {0};
    strncpy(Version, EEPROM.version.VERSION_PCB, sizeof(Version)-1);
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


//---------------------------------------//
//             ���������� ��             //
//---------------------------------------//
#define UPDATE_BUFFER_SIZE  256
#define EXTERNAL_HEADER_SIZE 12U
#define EXT_FW_AREA_SIZE TARGET_FLASH_SIZE
#define EXTERNAL_FW_START FLASH_TOTAL_SIZE
#define APP_SIZE (448U * 1024U)
#define EXTERNAL_MAX_FW_SIZE (APP_SIZE)
#define APP_ADDRESS 0x08010000U
static uint32_t gFirmwareSize = 0;


static int W25_Write_PageAligned(uint32_t addr,
                                 const uint8_t *src,
                                 uint32_t len)
{
    while (len)
    {
        uint32_t pageRest = 256U - (addr & 0xFFU); 
        uint32_t chunk = (len < pageRest) ? len : pageRest; /* ? 256?page */

        if (W25_Write_Data(addr, src, chunk) != 0) /* PP   */
            return -1;
        if (W25_WaitForReady(500U) != 0) /* busy */
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
  return crc & 0xFFFFFFFFU; /* <?? ����� �� ������ ������ */
}

/* ===== ������� ������� ������� ���� ������� ================= */
static int EraseExternalFirmwareArea(void)
{
  const uint32_t sectorSize = 4096;
  uint32_t totalSectors = EXT_FW_AREA_SIZE / sectorSize;
  uint8_t step = 0;
  for (uint32_t i = 0; i < totalSectors; ++i)
  {
    uint32_t addr = EXTERNAL_FW_START + i * sectorSize;
    // ������� ������ � ��� ����������
    if (W25_Erase_Sector(addr) != 0)
      return -1;
    if (W25_WaitForReady(500) != 0)
      return -1;
    // ��������� �������� ������ 10%
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

FRESULT FindAndOpenFirstBin(FIL *pFile, const char *USBHPath) {
    DIR dir;
    FILINFO fno;
    FRESULT res;
    char fullpath[64];

    // ���� � ����� USBHPath ���� �� ����� *.BIN
    res = f_findfirst(&dir, &fno, USBHPath, "*.BIN");
    if (res != FR_OK) {
        f_closedir(&dir);
        return res;
    }

    if (fno.fname[0] == 0) {
        // �� ���� ���� �� ������
        f_closedir(&dir);
        return FR_NO_FILE;
    }

    // �������� ������ ����: USBHPath + SFN-���
    // USBHPath ������ ������������ ������, �������� "0:/" ��� "0:/DIR/"
    snprintf(fullpath, sizeof(fullpath), "%s%s", USBHPath, fno.fname);

    // �������� �������
    res = f_open(pFile, fullpath, FA_READ);
    f_closedir(&dir);
    return res;
}

void Update_PO(void)
{
    FRESULT res;
    char path[32];

    // 1) ��������� UPDATE.bin
    /*
    snprintf(path, sizeof(path), "%sUPDATE.bin", USBHPath);
    if ((res = f_open(&MyFile, path, FA_READ)) != FR_OK) {
        OLED_DrawCenteredString("���� �� ������", 30);
        OLED_UpdateScreen();
        return;
    }*/
    if ((res = FindAndOpenFirstBin(&MyFile, USBHPath)) != FR_OK) {
         OLED_DrawCenteredString("���� �� ������", 30);
         OLED_UpdateScreen();
         return;
     }

    // 2) ��������� ������ �����
    DWORD fullSize = f_size(&MyFile);
    if (fullSize <= EXTERNAL_HEADER_SIZE || fullSize > EXTERNAL_HEADER_SIZE + EXTERNAL_MAX_FW_SIZE) {
        f_close(&MyFile);
        OLED_Clear(0);
        OLED_DrawCenteredString("�������� ������", 10);
        OLED_UpdateScreen();
        osDelay(1000);
        return;
    }

    // 3) ������ 8 ���� ������� + 4 ����� CRC32

    UINT br;
    uint8_t size_hdr[8], crc_hdr[4];
    // ������ 8 ���� ������� ��������
    if (f_read(&MyFile, size_hdr, 8, &br) != FR_OK || br != 8) { 
        OLED_Clear(0);
        OLED_DrawCenteredString("������ �������", 10);
        OLED_UpdateScreen();
        f_close(&MyFile); 
        return -1; 
    }
    // ������ 4 ����� CRC32
    if (f_read(&MyFile, crc_hdr, 4, &br) != FR_OK || br != 4) { 
        OLED_Clear(0);
        OLED_DrawCenteredString("������ �������", 10);
        OLED_UpdateScreen();
        f_close(&MyFile); 
        return -1;  
    }
    // ��������� ������ �������� �� ������ 8 ���� (little endian)
    gFirmwareSize =  (uint64_t)size_hdr[0]
                   | ((uint64_t)size_hdr[1] <<  8)
                   | ((uint64_t)size_hdr[2] << 16)
                   | ((uint64_t)size_hdr[3] << 24)
                   | ((uint64_t)size_hdr[4] << 32)
                   | ((uint64_t)size_hdr[5] << 40)
                   | ((uint64_t)size_hdr[6] << 48)
                   | ((uint64_t)size_hdr[7] << 56);

    // ��������� CRC32
    uint32_t expectedCRC =  (uint32_t)crc_hdr[0]
                          | ((uint32_t)crc_hdr[1] <<  8)
                          | ((uint32_t)crc_hdr[2] << 16)
                          | ((uint32_t)crc_hdr[3] << 24);

    // ������� �������� ���������
    OLED_Clear(0);
    OLED_DrawCenteredString("���������� �����", 10);
    PROGRESS_BAR(0);
    OLED_UpdateScreen();
    if (EraseExternalFirmwareArea() != 0) { 
        OLED_Clear(0);
        OLED_DrawCenteredString("������ ��������", 10);
        OLED_UpdateScreen();
        f_close(&MyFile); 
        return -1; 
    }


    // ���������� � W25Q128 ��� 12 ���� ���������
    if (W25_Write_Data(EXTERNAL_FW_START,        size_hdr, 8) != 0) { 
        OLED_Clear(0);
        OLED_DrawCenteredString("������ ������", 10);
        OLED_UpdateScreen();
        f_close(&MyFile); 
        return -1; 
    }
    if (W25_Write_Data(EXTERNAL_FW_START + 8, crc_hdr, 4) != 0)      { 
        OLED_Clear(0);
        OLED_DrawCenteredString("������ ������", 10);
        OLED_UpdateScreen();
        f_close(&MyFile); 
        return -1; 
    }

    // �������� ��������� �������� � ���������� � ��������� CRC �� ����
    OLED_Clear(0);
    OLED_DrawCenteredString("�����������", 10);
    PROGRESS_BAR(0);
    OLED_UpdateScreen();

    uint32_t total = 0;
    uint32_t addr  = EXTERNAL_FW_START + EXTERNAL_HEADER_SIZE;
    uint32_t crc   = 0xFFFFFFFFU;
    uint8_t  buf[256];
    uint8_t  step  = 0;

    while (total < gFirmwareSize) {
        UINT need = (gFirmwareSize - total > sizeof(buf)) ? sizeof(buf) : (gFirmwareSize - total);
        if (f_read(&MyFile, buf, need, &br) != FR_OK || br == 0) { 
            OLED_Clear(0);
            OLED_DrawCenteredString("������ ������", 10);
            OLED_UpdateScreen();
            f_close(&MyFile); 
            return -1; 
        }

        for (UINT i = 0; i < br; ++i) crc = CRC32_Update(crc, buf[i]);
        if (W25_Write_PageAligned(addr, buf, br) != 0) { 
            OLED_Clear(0);
            OLED_DrawCenteredString("������ ������ 2", 10);
            OLED_UpdateScreen();
            f_close(&MyFile); 
            return -1;  
        }

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
    if (crc != expectedCRC) {
        OLED_Clear(0);
        OLED_DrawCenteredString("������ �����������", 10);
        OLED_UpdateScreen();
        HAL_Delay(1000);
        EraseExternalFirmwareArea();
        return -1;
    }

    OLED_Clear(0);
    OLED_DrawCenteredString("������� �����������", 10);
    OLED_UpdateScreen();
    HAL_Delay(1000);

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_RTC_ENABLE();
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, BKP_UPDATE_FLAG);
    NVIC_SystemReset();
}