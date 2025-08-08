#include "Menu_data.h"
// Референсные токи
static const int refs_mA[2] = {4, 20};

// Чтение версии загрузчика из памяти
bool Boot_CopyVersion(Bootloader_data_item* out, size_t out_sz)
{
    if (!out || out_sz < sizeof(Bootloader_data_item)) return false;

    const Bootloader_data_item* src = BOOTVER_ADDR;

    // Быстрые проверки «стёрто/мусор»
    if (src->magic == 0xFFFFFFFFu || src->magic == 0x00000000u) return false;
    if (src->magic != BOOTLOADER_MAGIC) return false;

    // Версия должна иметь нуль-терминатор в пределах 32 байт
    size_t i = 0;
    for (; i < sizeof(src->version) && src->version[i] != '\0'; ++i) {}
    if (i == sizeof(src->version)) return false;

    // Копируем всю структуру в RAM
    memcpy(out, src, sizeof(*out));
    // На всякий — обеспечим нуль-терминатор
    out->version[sizeof(out->version) - 1] = '\0';
    return true;
}

// Функции аппаратного слоя (заглушки)
HAL_StatusTypeDef Collect_ADC_Data(uint8_t channel, int ref_mA)
{
    double ref_value[2][2] = {
        {0.0038, 0.0042}, // 4mA
        {0.018, 0.022} // 20mA
    }; // переводим в амперы
    //if ((ADC_data.ADC_Current[channel] < ref_value[ref_mA][0]) || (ADC_data.ADC_Current[channel] > ref_value[ref_mA][1])) {
    //    return HAL_ERROR;
    //}
    // Получение колибровочного коэффициента
    Main_data.real_current[ref_mA][channel] = Main_data.k_koeff[channel] ;

    return HAL_OK;
}

HAL_StatusTypeDef Colibrate_current_channel(uint8_t channel)
{
    Main_data.k_koeff[channel] = 0.016/(Main_data.real_current[1][channel] - Main_data.real_current[0][channel]);
    Main_data.b_koeff[channel] = 0.004 - Main_data.k_koeff[channel] * Main_data.real_current[0][channel];
    return HAL_OK;
}


const char DIS_PROTECT [2][40] = {"Выключить защиту?",  "Disable protection?"};
const char START_SETUP[2][40] = {"Начать настройку?",  "Start setup?"};

void Initial_setup(void)
{
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    
    // Проверка на защиту калибровочных данных
    if (Flash_IsCalibProtected())
    {
        OLED_Clear(0);
        Display_TopBar(selectedMenuItem);
        OLED_UpdateScreen();
        if (YES_OR_NO(DIS_PROTECT) == 0)
        {
            mode_redact = 0;
            Keyboard_press_code = 0xFF;
            return;
        }
        if (Flash_UnprotectCalibPage() == HAL_OK)
        {
            return;
        }
        else
        {
            OLED_Clear(0);
            Display_TopBar(selectedMenuItem);
            OLED_DrawCenteredString("Ошибка снятия защиты", 30);
            OLED_UpdateScreen();
            osDelay(1000);
            mode_redact = 0;
            Keyboard_press_code = 0xFF;
            return;
        }
    }


    if (YES_OR_NO(START_SETUP) == 0)
    {
        mode_redact = 0;
        Keyboard_press_code = 0xFF;
        return;
    }
    mode_redact = 5;

    InputString(Main_data.version.VERSION_PCB,
                strlen(Main_data.version.VERSION_PCB),
                "Серийный номер");
    InputString(Main_data.version.password,
                PASSWORD_LEN,
                "Пароль устройства");

    CalibrateVoltage();

    // 4) Калибровка тока по каналам
    CalibrateTable();

    // Финал: успешное завершение
    mode_redact = 2;
    OLED_Clear(0);
    Display_TopBar(selectedMenuItem);
    OLED_DrawCenteredString("УСПЕШНО", 30);
    OLED_UpdateScreen();
    osDelay(1000);
    Keyboard_press_code = 0xFF;
}

uint8_t InputString(char *buffer, uint8_t length, const char *title)
{
    uint8_t pos = 0, redraw = 1;
    Keyboard_press_code = 0xFF;

    while (Keyboard_press_code != 'O') {

        if (Keyboard_press_code == 'L') {
            do {
                pos = (pos == 0 ? length - 1 : pos - 1);
            } while (buffer[pos] == '.');
            redraw = 1;
        }
        if (Keyboard_press_code == 'R') {
            do {
                pos = (pos + 1) % length;
            } while (buffer[pos] == '.');
            redraw = 1;
        }
        if (Keyboard_press_code == 'U') {
            if (buffer[pos] < '~') buffer[pos] = buffer[pos]+1;
            else buffer[pos] = '/';
            redraw = 1;
        }
        if (Keyboard_press_code == 'D') {
            if (buffer[pos] > '/') buffer[pos] = buffer[pos]-1;
            else buffer[pos] = '~';
            redraw = 1;
        }

        if (Keyboard_press_code >= '0' && Keyboard_press_code <= '9') {
            // ввод нового символа (точка уже отфильтрована выше)
            buffer[pos] = Keyboard_press_code;
            do {
                pos = (pos + 1) % length;
            } while (buffer[pos] == '.');
            redraw = 1;
        }

        if (redraw) {
            redraw = 0;
            OLED_Clear(0);
            Display_TopBar(selectedMenuItem);

            // рисуем всю строку по центру
            uint16_t totalW = OLED_GetWidthStr(buffer);
            uint16_t x0     = (winth_display - totalW) / 2;
            uint16_t y      = 30;
            OLED_DrawStr(buffer, x0, y, 1);

            // вычисляем X подчёркивания:
            uint16_t cursorX = x0;
            for (uint8_t i = 0; i < pos; ++i) {
                char tmp[2] = { buffer[i], '\0' };
                cursorX += OLED_GetWidthStr(tmp);
            }

            // длина подчёркивания = ширина текущего символа
            char curCh[2] = { buffer[pos], '\0' };
            uint16_t w    = OLED_GetWidthStr(curCh);
            OLED_DrawHLine(cursorX, y + Font.height + 2, w, 1);

            OLED_DrawCenteredString(title, 20);
            OLED_UpdateScreen();
        }

        Keyboard_press_code = 0xFF;
        osDelay(10);
    }

    Keyboard_press_code = 0xFF;
    return 1;
}


// Калибровка напряжения 24 В
void CalibrateVoltage(void)
{
    Keyboard_press_code = 0xFF;
    uint8_t exit_code = 0;
    while ((Keyboard_press_code != 'O') || (exit_code == 0))
    {
        OLED_Clear(0);
        Display_TopBar(selectedMenuItem);
        OLED_DrawCenteredString("Калибровка 24 В", 20);
        uint16_t w = OLED_GetWidthStr(IntADC.ADC_AKB_volts_char);
        OLED_DrawStr(IntADC.ADC_AKB_volts_char, (winth_display - w) / 2, 30, 1);
        if (Keyboard_press_code == 'R'){
            int res = Read_ADC_Colibrate_24V();
            if (res == -1) OLED_DrawCenteredString("Ошибка", 40);
            else{ OLED_DrawCenteredString("Успешно", 40); exit_code=1; }
            OLED_UpdateScreen();
            osDelay(500);
        }
        OLED_UpdateScreen();
        osDelay(50);
    }
    OLED_DrawCenteredString("OK", 40);
    OLED_UpdateScreen();
    osDelay(1000);
    Keyboard_press_code = 0xFF;
}

// Калибровка тока по трём каналам на одной странице
void CalibrateTable(void)
{
    enum { COUNT = 3 };
    uint8_t sel_chan = 0;
    uint8_t ok4[COUNT] = {0};
    uint8_t ok20[COUNT] = {0};
    uint8_t sel_ref[COUNT] = {0};
    Keyboard_press_code = 0xFF;
    uint8_t done_counter = 0;
    while (1)
    {


        OLED_Clear(0);
        Display_TopBar(selectedMenuItem);

        uint16_t colW = winth_display / COUNT;
        uint16_t y0 = 16;
        uint16_t rectH = 63;

        for (uint8_t ch = 0; ch < COUNT; ++ch)
        {
            uint16_t x = ch * colW + 2;
            // рамка
            if (ch == sel_chan){
                if (ch!=0) x--;
                OLED_DrawRectangle(x-2, y0-2, x+colW - 3, rectH);
                if (ch!=0) x++;
            }

            char buf[20];
            // заголовок
            sprintf(buf, "Канал %d", ch + 1);
            OLED_DrawCenteredString_OFFSETX(buf, y0, colW, colW*ch);
            // текущее измеренное
            OLED_DrawCenteredString_OFFSETX(ADC_data.ADC_Current_char[ch], y0 + 14, colW, colW*ch);
            // референс
            sprintf(buf, "R:%d мА", refs_mA[sel_ref[ch]]);
            OLED_DrawCenteredString_OFFSETX(buf, y0 + 28, colW, colW*ch);
            // статус
            char status[6];
            if (ok4[ch] && ok20[ch])      sprintf(status, "4/20");
            else if (ok4[ch])              sprintf(status, "4/x");
            else if (ok20[ch])             sprintf(status, "x/20");
            else                           sprintf(status, "x/x");
            OLED_DrawStr(status, x + colW/2 - OLED_GetWidthStr(status)/2, y0 + 38, 1);
        }
        OLED_UpdateScreen();

        if (Keyboard_press_code == 'L')
            sel_chan = (sel_chan == 0 ? COUNT - 1 : sel_chan - 1);
        else if (Keyboard_press_code == 'R')
            sel_chan = (sel_chan + 1) % COUNT;
        else if (Keyboard_press_code == 'U' || Keyboard_press_code == 'D')
            sel_ref[sel_chan] ^= 1;
        else if (Keyboard_press_code == 'O')
        {
            if (Collect_ADC_Data(sel_chan + 1, refs_mA[sel_ref[sel_chan]]) == HAL_OK)
            {
                if (sel_ref[sel_chan] == 0) ok4[sel_chan] = 1;
                else                         ok20[sel_chan] = 1;
            }
            osDelay(100);
        }
        uint8_t all_done = 1;
        for (uint8_t i = 0; i < COUNT; ++i)
            if (!(ok4[i] && ok20[i])) { all_done = 0; break; }
        if (all_done) {
            if (Keyboard_press_code == 'O') done_counter++;
            else done_counter = 0;
            if (done_counter > 10){
                Colibrate_current_channel(0);
                Colibrate_current_channel(1);
                Colibrate_current_channel(2);
                OLED_Clear(0);
                Display_TopBar(selectedMenuItem);
                OLED_DrawCenteredString("Калибровка", 30);
                OLED_DrawCenteredString("успешна", 40);
                OLED_UpdateScreen();
                osDelay(1000);

                OLED_Clear(0);
                Display_TopBar(selectedMenuItem);
                HAL_StatusTypeDef res = Flash_WriteCalib(&Main_data);
                if (res == HAL_OK) {
                    OLED_DrawCenteredString("Запись успешна", 30);
                } else {
                    OLED_DrawCenteredString("Ошибка записи", 30);
                    OLED_UpdateScreen();
                    osDelay(5000);
                    NVIC_SystemReset(); // Значит повреждена флэш память, перезагружаем устройство
                }
                OLED_UpdateScreen();
                break;
            }
        }

        Keyboard_press_code = 0xFF;
        osDelay(20);
    }
    Keyboard_press_code = 0xFF;
}