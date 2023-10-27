#pragma once

#include <Arduino.h>
#include "env.hpp"
#include "mqtt/MQTT.hpp"
#include "network/Network.hpp"
#include "httpServer/HTTPServer.hpp"
#include "ota/OTA.hpp"
#include "i2c/I2C.hpp"
#include "modbus/ModBus.hpp"
#include "drain/Drain.hpp"


TaskHandle_t TaskMQTT;
TaskHandle_t TaskNetwork;
TaskHandle_t TaskHTTPSever;
TaskHandle_t TaskOTA;
TaskHandle_t TaskI2C;
TaskHandle_t TaskModBus;
TaskHandle_t TaskDrain;

void setup()
{
  Serial.begin(115200);
  
  queues::setup();

  #if ENV_TASK_NETWORK
  xTaskCreate(Network::taskNetwork, "TaskNetwork", 5 * 1024, NULL, 1, &TaskNetwork);
  #endif

  #if ENV_TASK_MQTT
  xTaskCreate(MQTT::taskMQTT, "TaskMQTT", 5 * 1024, NULL, 2, &TaskMQTT);
  #endif

  #if ENV_TASK_HTTPSERVER
  xTaskCreate(HTTPServer::taskHTTPServer, "TaskHTTPServer", 5 * 1024, NULL, 1, &TaskHTTPSever);
  #endif

  #if ENV_TASK_OTA
  xTaskCreate(OTA::taskOTA, "TaskOTA", 5 * 1024, NULL, 1, &TaskOTA);
  #endif

  #if ENV_TASK_I2C
  xTaskCreate(I2C::taskI2C, "TaskI2C", 5 * 1024, NULL, 1, &TaskI2C);
  #endif

  #if ENV_TASK_MODBUS
  xTaskCreate(ModBus::taskModbus, "TaskModbus", 5 * 1024, NULL, 1, &TaskModBus);
  #endif

  #if ENV_TASK_DRAIN
  xTaskCreate(Drain::taskDrain, "TaskDrain", 3 * 1024, NULL, 1, &TaskModBus);
  #endif
}

void loop()
{
  // put your main code here, to run repeatedly:
  vTaskDelay(2500 / portTICK_PERIOD_MS);
}
