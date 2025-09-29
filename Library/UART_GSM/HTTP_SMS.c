#include "HTTP_SMS.h"

// ���������� M2M, ��������� ������ �� 5 ������
void M2M_disable()
{
    HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
    GSM_data.Status = 0;
    osDelay(5000);
}

// ���������� M2M, ��������� ������ �� 1,6 �������
void M2M_enable()
{
    // ���� GSM ��� ��������
    if (HAL_GPIO_ReadPin(EN_3P8V_GPIO_Port, EN_3P8V_Pin) == 0)
    {
        HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 1); // ��������� GSM
        osDelay(100);
        HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 1);
        osDelay(600);
        HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 0);
        osDelay(1000);
    }
}

// �������� ������� M2M � ���������/���������� ��� �������������
void M2M_check_POWER()
{
    // ���� GSM ������ ���� ��������
    if (EEPROM.Communication == M2M_DISABLE)
    {
        M2M_disable();
    }
    // ���� GSM ������ ���� �������
    if (EEPROM.Communication != M2M_DISABLE)
    {
        M2M_enable();
    }
}

// ������������� M2M � ��������� ������� ������ �����
void M2M_init(uint32_t startTick)
{

    // ���� ������ ����� �� �����
    if ((!(GSM_data.Status & GSM_RDY)) && ((EEPROM.DEBUG_Mode != USB_AT_DEBUG) || (EEPROM.USB_mode != USB_DEBUG)))
    {
        if (SendCommandAndParse("AT\r", waitForOKResponse, 2000) == HAL_OK)
        {
            USB_DEBUG_MESSAGE("[DEBUG AT] ������ ����� ���������, ������ ��������� �������� ������ �����", DEBUG_GSM, DEBUG_LEVL_1);
            if (SendCommandAndParse("AT+CFUN=0\r", waitForOKResponse, 2000) == HAL_OK)
            {
                if (SendCommandAndParse("AT+CFGDUALMODE=1,0\r", waitForOKResponse, 2000) == HAL_OK)
                {
                    if (SendCommandAndParse("AT+CFGRATPRIO=2\r", waitForOKResponse, 2000) == HAL_OK)
                    {
                        if (SendCommandAndParse("AT+CFUN=1\r", waitForOKResponse, 2000) == HAL_OK)
                        {
                            if (SendCommandAndParse("AT+CSCON=0\r", waitForOKResponse, 2000) == HAL_OK)
                            {
                                if (SendCommandAndParse("AT&W\r", waitForOKResponse, 2000) == HAL_OK)
                                {
                                    GSM_data.Status |= GSM_RDY;
                                    USB_DEBUG_MESSAGE("[DEBUG AT] ������ ����� ��������", DEBUG_GSM, DEBUG_LEVL_1);
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            USB_DEBUG_MESSAGE("[ERROR AT] ������ ����� �� ��������, ����", DEBUG_GSM, DEBUG_LEVL_1);
        }
        if (HAL_GetTick() - startTick > 10000) // ������� 10 ������
        {
            USB_DEBUG_MESSAGE("[ERROR AT] ������� 10� ��������, ������ ����� �� ������", DEBUG_GSM, DEBUG_LEVL_1);
            ERRCODE.STATUS |= STATUS_UART_NO_RESPONSE;
        }
    }

    if (GSM_data.Status & NETWORK_REGISTERED_SET_HTTP)
    {
        GSM_data.Status &= ~NETWORK_REGISTERED_SET_HTTP;
        USB_DEBUG_MESSAGE("[DEBUG AT] ��������� GPRS", DEBUG_GSM, DEBUG_LEVL_3);
        SendCommandAndParse("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r", waitForOKResponse, 1000);
        SendCommandAndParse("AT+CGACT=1,1\r", waitForOKResponse, 60000);
        SendCommandAndParse("AT+CDNSCFG=\"8.8.8.8\",\"77.88.8.8\"\r", waitForOKResponse, 1000);
    }
}

// ���������� ������� �������� ��� � HTTP �� ������
void M2M_status_Update()
{
    if (EEPROM.Mode == 0)
    {
        if (!(GSM_data.Status & SMS_SEND))
        {
            strcpy(GSM_data.GSM_sms_status, "");
        }
        if (!(GSM_data.Status & HTTP_READ))
        {
            strcpy(GSM_data.GSM_site_read_status, "");
        }
        if (!(GSM_data.Status & HTTP_SEND))
        {
            strcpy(GSM_data.GSM_site_status, "");
        }
    }

    if ((EEPROM.DEBUG_Mode != USB_AT_DEBUG) || (EEPROM.USB_mode != USB_DEBUG))
    {
        USB_DEBUG_MESSAGE("[DEBUG AT] ������ ���������� ������� GSM", DEBUG_GSM, DEBUG_LEVL_2);
        SendCommandAndParse("AT+CPIN?\r", waitForCPINResponse, 1000);
        SendCommandAndParse("AT+CSQ\r", waitForCSQResponse, 1000);
        SendCommandAndParse("AT+CEREG?\r", waitForCEREGResponse, 1000);
        SendCommandAndParse("AT+COPS?\r", waitForCOPSResponse, 1000);
        GSM_data.update_value();
        if (EEPROM.Mode == 0)
            xSemaphoreGive(Display_semaphore);
    }
}

// �������� ���, ���� ���������� ���� �� ��������
void SMS_send()
{
    // ���� ����� ��������� ���
    if (GSM_data.Status & SMS_SEND)
    {
        USB_DEBUG_MESSAGE("[DEBUG AT] ������ �������� ���", DEBUG_GSM, DEBUG_LEVL_2);
        GSM_data.Status &= ~SMS_SEND;
        Collect_DATA();
        if (sendSMS() == HAL_OK)
        {
            USB_DEBUG_MESSAGE("[DEBUG AT] ��� ����������", DEBUG_GSM, DEBUG_LEVL_2);
            if (EEPROM.Mode == 0)
                strcpy(GSM_data.GSM_sms_status, "OK");
            if (EEPROM.Mode == 0)
                xSemaphoreGive(Display_semaphore);
            GSM_data.Status |= SMS_SEND_Successfully;
        }
        else
        {
            USB_DEBUG_MESSAGE("[ERROR AT] ��� �� ����������", DEBUG_GSM, DEBUG_LEVL_1);
            if (EEPROM.Mode == 0)
                strcpy(GSM_data.GSM_sms_status, "ERR");
            if (EEPROM.Mode == 0)
                xSemaphoreGive(Display_semaphore);
            ERRCODE.STATUS |= STATUS_GSM_SMS_SEND_ERROR;
        }
    }
}

// �������� HTTP GET, ���� ���������� ���� �� ��������
void HTTP_send(){
    if (GSM_data.Status & HTTP_SEND)
    {
      GSM_data.Status &= ~HTTP_SEND;
      Collect_DATA();
      USB_DEBUG_MESSAGE("[DEBUG AT] ������ HTTP GET", DEBUG_GSM, DEBUG_LEVL_2);
      if (sendHTTP() == HAL_OK)
      {
        USB_DEBUG_MESSAGE("[DEBUG AT] ������� HTTP GET", DEBUG_GSM, DEBUG_LEVL_2);
        GSM_data.Status |= HTTP_SEND_Successfully;
        if (EEPROM.Mode == 0)
          strcpy(GSM_data.GSM_site_status, "OK");
        if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
        ERRCODE.STATUS &= ~STATUS_HTTP_SERVER_COMM_ERROR;
      }
      else
      {
        USB_DEBUG_MESSAGE("[ERROR AT] ������ HTTP GET", DEBUG_GSM, DEBUG_LEVL_1);
        if (EEPROM.Mode == 0)
          strcpy(GSM_data.GSM_site_status, "ERR");
        if (EEPROM.Mode == 0) xSemaphoreGive(Display_semaphore);
        ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
      }
    }
}

void HTTP_read(){
        if (GSM_data.Status & HTTP_READ)
    {
      USB_DEBUG_MESSAGE("[DEBUG AT] ������ HTTP READ", DEBUG_GSM, DEBUG_LEVL_1);
      // ���� ���� ��� ����� ��������� HTTP read
      GSM_data.Status &= ~HTTP_READ;
      SETTINGS_REQUEST_DATA();
      if (READ_Settings_sendHTTP() == HAL_OK)
      {
        USB_DEBUG_MESSAGE("[ERROR AT] ������� HTTP READ", DEBUG_GSM, DEBUG_LEVL_1);
        GSM_data.Status |= HTTP_READ_Successfully;
        if (EEPROM.Mode == 0){
          strcpy(GSM_data.GSM_site_read_status, "OK");
          xSemaphoreGive(Display_semaphore);
          ERRCODE.STATUS &= ~STATUS_HTTP_SERVER_COMM_ERROR;
        }
      }
      else
      {
        USB_DEBUG_MESSAGE("[ERROR AT] ������ HTTP READ", DEBUG_GSM, DEBUG_LEVL_1);
        if (EEPROM.Mode == 0)
        {
          strcpy(GSM_data.GSM_site_read_status, "ERR");
          xSemaphoreGive(Display_semaphore);
        }
        ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
      }
    }
}

// ����������� ������ - �������� ������ �� ������
HAL_StatusTypeDef SEND_DATA()
{
    // ���� ��������� ������ ����� � ����������� � ����
    if (EEPROM.Communication != M2M_DISABLE)
    {
        // 60 ������ �� ������� ������������������
        for (uint32_t i = HAL_GetTick(); HAL_GetTick() - i > 60000;)
        {
            if ((GSM_data.Status & NETWORK_REGISTERED) && (GSM_data.Status & SIGNAL_PRESENT))
            {
                break;
            }
            if (GSM_data.Status & STATUS_UART_NO_RESPONSE)
            {
                USB_DEBUG_MESSAGE("[DEBUG AT] ������ ����� ����", DEBUG_GSM, DEBUG_LEVL_2);
                HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
                osThreadSuspend(UART_PARSER_taskHandle);
                ERRCODE.STATUS |= STATUS_GSM_REG_ERROR;
                GSM_data.Status = 0;
                return HAL_ERROR;
            }
            osDelay(500);
        }

        if (ERRCODE.STATUS &= STATUS_GSM_REG_ERROR)
        {
            // ����� ��� � �� ����������� - ��������� GSM ������
            USB_DEBUG_MESSAGE("[DEBUG AT] ��� ������� ������������������ ��������� - ��������� �����", DEBUG_GSM, DEBUG_LEVL_2);
            HAL_GPIO_WritePin(EN_3P8V_GPIO_Port, EN_3P8V_Pin, 0);
            osThreadSuspend(UART_PARSER_taskHandle);
            GSM_data.Status = 0;
        }
        else{
            // ����� ���� - ���������� ������
            if (EEPROM.Communication_http_mqtt == HTTP) HTTP_or_SMS_send(); // �������� HTTP ��� ��� - ��������
            if (EEPROM.Communication_http_mqtt == MQTT){
                for (uint32_t i = HAL_GetTick(); HAL_GetTick() - i > 20000;) // ���� ����������� � MQTT 20 c�����
                {
                    if (ERRCODE.STATUS & STATUS_MQTT_CONN)
                    {
                        MQTT_PublishSaveData(save_data, 60000);  // �������� MQTT 
                        break;
                    }
                    osDelay(100);
                }
            }
        }
    }
}

HAL_StatusTypeDef HTTP_or_SMS_send()
{
    USB_DEBUG_MESSAGE("[DEBUG AT] ������ ��������� �������� HTTP � �����", DEBUG_GSM, DEBUG_LEVL_2);
    GSM_data.Status |= HTTP_SEND;
    // �������� �������, � �� �����. � ����� ����� ��������� ������ ��������� � ���������
    for (int i = 0; i < 240; i++)
    {
        if (GSM_data.Status & HTTP_SEND_Successfully)
        {
            return HAL_OK;
        }
        if (ERRCODE.STATUS & STATUS_HTTP_SERVER_COMM_ERROR)
        {
            break;
        }
        osDelay(1000);
    }

    // ���� ��� ������� HTTP ��������� - ���������� ���
    if (((ERRCODE.STATUS & STATUS_HTTP_SERVER_COMM_ERROR)) && !(ERRCODE.STATUS & STATUS_HTTP_NO_BINDING_ERROR))
    {
        _sendSMS();
    }
}

void _sendSMS()
{
    USB_DEBUG_MESSAGE("[DEBUG AT] ��� ������� HTTP ��������� - ������ �������� ���", DEBUG_GSM, DEBUG_LEVL_2);
    GSM_data.Status &= ~HTTP_SEND;
    GSM_data.Status &= ~SMS_SEND_Successfully;
    GSM_data.Status &= ~STATUS_GSM_SMS_SEND_ERROR;
    GSM_data.Status |= SMS_SEND;
    for (int timeout_sms = 0; timeout_sms < 120; timeout_sms++)
    {
        if (GSM_data.Status & SMS_SEND_Successfully)
        {
            USB_DEBUG_MESSAGE("[DEBUG AT] ��� ����������", DEBUG_GSM, DEBUG_LEVL_3);
            return HAL_BUSY; // �������� ��� HTTP �� ���������, �� ��� ����������
        }
        if (ERRCODE.STATUS & STATUS_GSM_SMS_SEND_ERROR)
        {
            USB_DEBUG_MESSAGE("[DEBUG AT] ������ �������� ���", DEBUG_GSM, DEBUG_LEVL_3);
            return HAL_ERROR;
        }
        osDelay(1000);
        char send[20] = "\0";
        sprintf(send, "[INFO] %d", timeout_sms);
        USB_DEBUG_MESSAGE(send, DEBUG_GSM, DEBUG_LEVL_3);
    }
}