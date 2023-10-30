#include "ModBus.hpp"

namespace ModBus
{
    ModbusRTU mb; // Class

    bool cb(Modbus::ResultCode event, uint16_t transactionId, void *data)
    { // Callback to monitor errors
        if (event != Modbus::EX_SUCCESS)
        { // Caso ocurrer um ERRO
            #if ENV_MODBUS_DEBUG
                Serial.print("Request result: 0x");
                Serial.print(event, HEX);
                Serial.println();
            #endif
        }
        return true;
    }

    float floatCOD(long int byte0, long int byte1, long int byte2, long int byte3)
    { // Recebe os registo e converte para float (sensor COD)
        long int realbyte0, realbyte1, realbyte2, realbyte3;
        char S;
        long int E, M;
        float D;
        realbyte0 = byte3;
        realbyte1 = byte2;
        realbyte2 = byte1;
        realbyte3 = byte0;
        if ((realbyte0 & 0x80) == 0)
        {
            S = 0; // Positivo
        }
        else
        {
            S = 1; // Negativo
        }
        E = ((realbyte0 << 1) | (realbyte1 & 0x80) >> 7) - 127;
        M = ((realbyte1 & 0x7f) << 16) | (realbyte2 << 8) | realbyte3;
        D = pow(-1, S) * (1.0 + M / pow(2, 23)) * pow(2, E);
        return D;
    }

    float floatTurb(uint16_t registo1, uint16_t registo2)
    { // Recebe 2 registos em inteiro e converte em float pelo protocolo IEEE754
        float valorFloat;
        int j = 0;
        uint32_t rg = 0;
        int expAux = 0;
        int e = 0;
        float mantissa = 0;
        int auxMantissa = 0;
        rg = (registo1 << 14) | registo2;
        expAux = (rg >> 21) & 255; // Apanha o exponencial
        e = expAux - 127;
        auxMantissa = rg << 11; // Apanha a mantissa
        auxMantissa = auxMantissa >> 11;
        for (int i = 23; i > 1; i--) // Recolhe cada casa decimal e muitiplca. Ex: 2^(-23)..
        {
            mantissa = mantissa + (((auxMantissa >> j) & 1) * powl(2, -i));
            j++;
        }
        valorFloat = (1 + mantissa) * pow(2, e); // Formula final
        return valorFloat;
    }

    float read_temperature()
    {
        float temp = 1.23;
        uint16_t temperature_reg[2];

        if (!mb.slave())
        {
            mb.writeHreg(ENV_TURB_ID, REG_TURB_START_MES, 3, cb); // ID, registo e evento a escrever. 3 = Medir temperatura e Tubr
            while (mb.slave())
            {
                mb.task();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        if (!mb.slave())
        {
            mb.readHreg(ENV_TURB_ID, REG_TURB_TEMP, temperature_reg, 2, cb);
            while (mb.slave())
            {
                mb.task();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        temp = floatTurb(temperature_reg[0], temperature_reg[1]); // Faz a conversao de INT para Float
        return temp;
    }

    float read_turb()
    {
        uint16_t turb_reg[2];
        float turb = 1.23;
        if (!mb.slave())
        {
            mb.writeHreg(ENV_TURB_ID, REG_TURB_START_MES, 3, cb); // ID, registo e evento a escrever. 3 = Medir temperatura e Tubr
            while (mb.slave())
            {
                mb.task();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        if (!mb.slave())
        {
            mb.readHreg(ENV_TURB_ID, REG_TURB_TURB, turb_reg, 2, cb);
            while (mb.slave())
            {
                mb.task();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        turb = floatTurb(turb_reg[0], turb_reg[1]); // Faz a conversao de INT para Float
        return turb;
    }

    float read_cod()
    {
        float cod = 1.23;
        uint16_t cod_reg[2];
        if (!mb.slave())
        {                                                         // Check if no transaction in progress
            mb.readHreg(ENV_COD_ID, REG_COD_COD, cod_reg, 2, cb); // ID, nº do Registo, var onde guardr, evento caso falhe
            while (mb.slave())
            { // Check if transaction is active
                mb.task();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        cod = floatCOD(0x00, 0x00, ((cod_reg[1] & 0xFF00) >> 8), cod_reg[1] & 0x00FF); // Operação logica MSB e LSB
        return cod;
    }


    void taskModbus(void *pvParameters)
    {
        queues::Modbus_readings_t readings;
        
        Serial1.begin(9600, SERIAL_8N2, 16, 17); // BaudRate, 1 Start Bit 8 Data e 2 StopBit, Rx,Tx
        mb.begin(&Serial1);
        mb.master();

        Serial.println("MobBus: Booted");
        while (true)
        {
            // Readings
            readings.temperature = read_temperature();
            readings.turbidity = read_turb();
            readings.COD = read_cod();

            #if ENV_MODBUS_DEBUG
                Serial.println("Temperature: " + String(readings.temperature) + " ,Turb: " + String( readings.turbidity) + " ,COD: " + String(readings.COD) + " ,RPM: " + String(readings.pump_RMP));
                Serial.printf("AWD:%.2f, AWS:%.2f, AT:%.2f, AH:%.2f, AP:%.2f, RF:%.2f, Rad:%.2f, UV:%.2f\n"
                ,readings.EM_readings[1],readings.EM_readings[4],readings.EM_readings[6],readings.EM_readings[7],readings.EM_readings[8],readings.EM_readings[9],readings.EM_readings[10],readings.EM_readings[11]);
            #endif

            xQueueOverwrite(queues::modbus_readings,&readings);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}