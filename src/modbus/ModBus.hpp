#pragma once

#include <Arduino.h>
#include <ModbusRTU.h>
#include "../env.hpp"
#include "../queues/queues.hpp"

#define REG_PUMP_RPM 3100// Escrever/Ler RPM da bomba
#define REG_TURB_START_MES 0x0001 // Inicia a medição sensor tubr
#define REG_TURB_TEMP 0x0053//Le a mediçao de temperatura sensor turb
#define REG_TURB_TURB 0x0055//Lê a medição de temperatura
#define REG_COD_COD 0x2602//Lê o valor do COD mg/

namespace ModBus
{
    float floatCOD(long int byte0, long int byte1, long int byte2, long int byte3);
    float floatTurb(uint16_t registo1, uint16_t registo2);//Recebe 2 registos em inteiro e converte em float pelo protocolo IEEE754
    bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data); // Callback to monitor errors

    float read_temperature();
    float read_turb();
    float read_cod();

    int read_pump();
    void write_rpm_pump(uint16_t RPM);
    void read_EM(float *parametrosEstacaoMetero);

    void taskModbus(void *pvParameters);//Processa dados do protocolo Modbus
}
