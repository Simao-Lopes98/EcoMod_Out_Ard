#include "Drain.hpp"


namespace Drain
{
    bool travao = false;
    bool flagCiclo = true;
    uint16_t ciclos = 0;
    const char* TAG = "Drain";
    
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

        service_log(TAG,"Booted");
        while (true)
        {
            if (digitalRead(SENSOR_TOP_GPIO) == HIGH)
            {                                  // Liquido no sensor de cima detetado com a interrupt
                travao = 1;                    // Permite incrementar o n de ciclos
                digitalWrite(DRAIN_PUMP_GPIO, HIGH); // Liga a bomba
                service_log(TAG,"Pump ON");
            }
            if (digitalRead(SENSOR_BOT_GPIO) == LOW)
            {
                digitalWrite(DRAIN_PUMP_GPIO, LOW);
                service_log(TAG,"Pump OFF");
                if (flagCiclo == 0)
                {
                    // if (!client.connected())
                    // {
                    //     reconnect();
                    // }
                    // snprintf(msg2, MSG_BUFFER_SIZE2, "{\"ref\": \"Ciclos\", \"Ncic\":\"%d\" }", ciclos);
                    // client.publish("QTopic", msg2);
                    // Serial.println(msg2);
                    flagCiclo = 1; // RESET
                }
            }
        }
    }
}