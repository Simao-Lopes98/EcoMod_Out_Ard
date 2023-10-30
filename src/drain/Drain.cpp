#include "Drain.hpp"

namespace Drain
{
    bool travao = false;
    bool flagCiclo = true;
    uint16_t ciclos = 0;
    const char *TAG = "Drain";

    void bot_isr()
    {
        if (flagCiclo == 1)
        {
            if (travao == 1)
            {
                ciclos++;
                flagCiclo = 0;
            }
        }
        travao = 0;
    }

    void taskDrain(void *pvParameters)
    {
        pinMode(SENSOR_BOT_GPIO, INPUT);
        pinMode(SENSOR_TOP_GPIO, INPUT);
        pinMode(DRAIN_PUMP_GPIO, OUTPUT);
        attachInterrupt(SENSOR_BOT_GPIO, bot_isr, FALLING);
        bool start_pumping = false;
        uint16_t cycles = 0;
        service_log(TAG, "Booted");
        while (true)
        {

            if (digitalRead(SENSOR_TOP_GPIO) && digitalRead(SENSOR_BOT_GPIO) && !start_pumping)
            {
                start_pumping = true;
                cycles++;
                printf("%d\n", (int)cycles);
                xQueueOverwrite(queues::ncycles,&cycles);
            }
            if (!digitalRead(SENSOR_BOT_GPIO) && !digitalRead(SENSOR_TOP_GPIO) && start_pumping)
            {
                start_pumping = false;
            }
            if (start_pumping)
            {
                digitalWrite(DRAIN_PUMP_GPIO, HIGH);
                #if ENV_DRAIN_DEBUG
                    service_log(TAG, "Pump ON");
                #endif
            }
            else
            {
                digitalWrite(DRAIN_PUMP_GPIO, LOW);
                #if ENV_DRAIN_DEBUG
                    service_log(TAG, "Pump OFF");
                #endif
            }

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}