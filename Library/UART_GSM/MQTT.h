#pragma once
#include "main.h"
#include "Parser.h"
#include <string.h>
#include <stdio.h>

// ====== јдрес брокера (без "http://") ======
#define MQTT_BROKER_ADDR   "5.35.102.166"
#define MQTT_BROKER_PORT   1883
#define MQTT_KEEPALIVE_S   120
#define MQTT_CLEAN_SESSION 1

// ====== ѕараметры обмена ======
#define MQTT_QOS_PUB   1     // QoS=1 Ч практичный выбор дл€ сотовых
#define MQTT_QOS_SUB   1
#define MQTT_RETAIN    0

// “опики (2 публикации)
#define TOPIC_PUB1_FMT "test"
#define TOPIC_PUB2_FMT "test"   // сюда отправл€ем пустую строку

// “опики дл€ настроек
#define TOPIC_SETTINGS "test"         // подписка на все команды

// ¬нешние хелперы вашего проекта
extern char *parseBuffer;   // общий буфер с последним ответом модема

#define STATUS_MQTT_CONN_ERROR        (1u<<20)
#define STATUS_MQTT_PUB_ERROR         (1u<<21)
#define STATUS_MQTT_SUB_ERROR         (1u<<22)
#define STATUS_MQTT_SERVER_COMM_ERROR (1u<<23)
#define STATUS_MQTT_AUTH_ERROR        (1u<<24)

extern Main_data_settings_item Main_data;
// Ћогин/пароль/ClientID
typedef struct {
  char clientId[32];
  char username[32];
  char password[32];
} MqttCredentials;

typedef enum {
  MQTT_ST_IDLE = 0,
  MQTT_ST_PDP_UP,
  MQTT_ST_CFG,              // AT+MCONFIG
  MQTT_ST_CONNECT,          // AT+MCONNECT (и разбор CONNECT OK)
  MQTT_ST_CONNECTED,
  MQTT_ST_ERROR
} MqttConnState;

typedef struct {
  MqttConnState st;
  uint8_t connected;
  char host[64];
  int  port;
  uint16_t keepalive_s;
  uint8_t clean_session;
  MqttCredentials cred;
  uint32_t t_last_stat_ms;   // последний опрос статуса
} MqttSm;

// ===== API =====
void MQTT_InitGlobal(void);                       // инициализаци€ SM и кредов
void MQTT_Process(uint32_t now_ms);               // вызывать регул€рно (100Ц500 мс)
HAL_StatusTypeDef MQTT_EnsureConnected(uint32_t deadline_ms); // подключить при необходимости

HAL_StatusTypeDef MQTT_Publish(const char *topic,
                               const char *payload,
                               int qos, int retain,
                               uint32_t timeout_ms);

HAL_StatusTypeDef MQTT_PublishSaveData(const char *save_data, uint32_t timeout_ms);
HAL_StatusTypeDef MQTT_PublishEmptySecond(uint32_t timeout_ms);

HAL_StatusTypeDef MQTT_Subscribe(const char *topic,
                                 int qos,
                                 uint32_t timeout_ms);

HAL_StatusTypeDef MQTT_Disconnect(uint32_t timeout_ms);
