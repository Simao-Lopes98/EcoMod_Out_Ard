#pragma once

#include <Arduino.h>
#include <Log.hpp>
#include "../env.hpp"
#include "../queues/queues.hpp"
#define SENSOR_TOP_GPIO 14
#define SENSOR_BOT_GPIO 27
#define DRAIN_PUMP_GPIO 12

namespace Drain
{
    extern bool travao;
    extern uint16_t ciclos;
    extern bool flagCiclo;
    void bot_isr();
    void taskDrain(void *pvParameters);//Processa dados do protocolo Modbuss
}
