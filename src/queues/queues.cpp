#include "queues.hpp"

namespace queues
{

    QueueHandle_t data; // Mailbox
    QueueHandle_t sta_cred;
    QueueHandle_t i2c_readings; // Mailbox
    QueueHandle_t modbus_readings;//Mailbox
    QueueHandle_t ncycles; // Mailbox
    QueueHandle_t msg_period;

    void setup()
    {
        data = xQueueCreate(1, sizeof(uint8_t)); // Example queue

        sta_cred = xQueueCreate(2, sizeof(STA_cred_t));
        i2c_readings = xQueueCreate(1, sizeof(I2C_readings_t));
        modbus_readings = xQueueCreate(1, sizeof(Modbus_readings_t));
        ncycles = xQueueCreate(1,sizeof(uint16_t));
        msg_period = xQueueCreate(2, sizeof(uint16_t));
    }
}
