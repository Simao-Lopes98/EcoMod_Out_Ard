#include "I2C.hpp"

namespace I2C
{
    void setup_i2c(int SDA, int SCL, int CLK)
    {
        Wire.begin(SDA, SCL, CLK); // SDA, SCL , CLK (I2C)
    }

    void read_i2c_sensor(int add, char *data)
    {                                               // Rece os valores dos sensores da Atlas via I2C
        TickType_t xDelay = 900 / portTICK_RATE_MS; // Tempo de espera 900 ms
        static char mensagem[6];

        char lixo;
        Wire.beginTransmission(add); // Envia
        Wire.write(0x52);            // R- para ler
        Wire.endTransmission();
        vTaskDelay(xDelay);

        Wire.requestFrom(add, 6);
        lixo = Wire.read(); // apanho o lixo SOH

        int i = 0;
        while (Wire.available())
        {
            mensagem[i] = Wire.read();
            i++;
        }
        if(strlen(mensagem))
        {
            strcpy(data,mensagem);
        }else{
            strcpy(data,"0.123");//Error reading
        }
        
    }

    void taskI2C(void *pvParameters)
    {
        queues::I2C_readings_t readings;
        setup_i2c(21, 22, 100000);
        Serial.println("I2C: Booted");

        while (true)
        {
            read_i2c_sensor(ENV_PH_SENS_ADDR, readings.ph);
            vTaskDelay(1000/portTICK_PERIOD_MS);
            read_i2c_sensor(ENV_EC_SENS_ADDR, readings.ec);
            #if ENV_I2C_DEBUG
                Serial.println("PH: " + String(readings.ph) + " EC: " + String(readings.ec));
            #endif
            xQueueOverwrite(queues::i2c_readings, &readings);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}