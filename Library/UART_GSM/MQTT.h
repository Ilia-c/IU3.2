#pragma once
#include "main.h"
#include "Parser.h"
#include <string.h>
#include <stdio.h>

// ====== Адрес брокера (без "http://") ======
#define MQTT_BROKER_ADDR   "5.35.102.166"
#define MQTT_BROKER_PORT   1883
#define MQTT_KEEPALIVE_S   120
#define MQTT_CLEAN_SESSION 1

// ====== Параметры обмена ======
#define MQTT_QOS_PUB   1     // QoS=1 — практичный выбор для сотовых
#define MQTT_QOS_SUB   1
#define MQTT_RETAIN    0

// Топик для передачи кпоказаний
#define TOPIC_PUB1_FMT "aus/%s/data/publish"

// Топик для запроса настроек
#define TOPIC_PUB2_FMT "aus/%s/settings/get" 

// Топик для чтения настроек
#define TOPIC_SETTINGS_FMT "aus/%s/settings/get/responseh"
// Внешние хелперы вашего проекта
extern char *parseBuffer;   // общий буфер с последним ответом модема

extern Main_data_settings_item Main_data;
// Логин/пароль/ClientID
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
void MQTT_InitGlobal(void);                       // инициализация SM и кредов
void MQTT_Process(uint32_t now_ms);               // вызывать регулярно (100–500 мс)
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
