#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "../env.hpp"
#include "../queues/queues.hpp"

namespace MQTT
{
    void reconnect();

    void callback(char *topic, byte *payload, unsigned int length);
    void process_data();
    void initialize_values();
    
    void taskMQTT(void *pvParameters);//Task de envio de parametros para o broker
}
