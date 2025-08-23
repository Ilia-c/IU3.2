#include "qr2xbm.h"

// �������� ����������� �������� � ���������� 1, ���� ������ ��������, ����� 0
static int isAllowedChar(char c) {
    if ((c >= '0' && c <= '9') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z') ||
        c == '.' || c == ';' || c == '-')
    {
        return 1;
    }
    return 0;
}

uint8_t qrcode[QR_BUFFER_LEN];
uint8_t temp[QR_BUFFER_LEN];
HAL_StatusTypeDef QR_create(char *text)
{
    size_t len = strlen(text);
    if (len == 0 || len > MAX_TEXT) {
        return HAL_ERROR;
    }
    for (size_t i = 0; i < len; i++) {
        if (!isAllowedChar(text[i])) {
            return HAL_ERROR;
        }
    }


    // �������� ����� � QR (�������� �����, ������ 4, ��������� LOW)
    int ok = qrcodegen_encodeText(
        text, temp, qrcode,
        qrcodegen_Ecc_MEDIUM,
        /*verMin=*/QR_VERSION, /*verMax=*/QR_VERSION,
        qrcodegen_Mask_AUTO, true  // ���� ��������� �������� � bool, ��������� �� ��������� � int
    );
    if (!ok) {
        return HAL_ERROR;
    }

    // ��������� ������
    int qrSize = qrcodegen_getSize(qrcode);  // ������ ���� 33
    if (qrSize != 4 * QR_VERSION + 17) {
        return HAL_ERROR;
    }

    // ��������� ��������� XBM
    QR_XBM[0] = CANVAS_SIZE;  // ������
    QR_XBM[1] = CANVAS_SIZE;  // ������

    // ����������� �� 8 ������� � ���� � ����� � ������
    int idx = 2;
    for (int y = 0; y < CANVAS_SIZE; y++) {
        for (int bx = 0; bx < BYTES_PER_ROW; bx++) {
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; bit++) {
                int x = bx * 8 + bit;
                int qrX = x - MARGIN_MODS;
                int qrY = y - MARGIN_MODS;
                if (qrX >= 0 && qrX < qrSize && qrY >= 0 && qrY < qrSize &&
                    qrcodegen_getModule(qrcode, qrX, qrY))
                {
                    byte |= (1 << bit);
                }
            }
            QR_XBM[idx++] = byte;
        }
    }

    return HAL_OK;
}
