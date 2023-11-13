#pragma once



/*
*   MQTT
*/
#define ENV_MQTT_BROKER "raspberrypi.local" // endereço a ligar mqtt.eclipseprojects.io
#define ENV_SEND_PERIOD_SEC 5

/*
*   Network
*/
#define ENV_SSID "TP-Link_24E3"
#define ENV_PASSWORD "35677529"

/*
*   I2C
*/
#define ENV_PH_SENS_ADDR 0x63
#define ENV_EC_SENS_ADDR 0x64

/*
*   ModBus
*/
#define ENV_PUMP_ID 2// ID da Bomba (Numero no mostrador da própria bomba)
#define ENV_TURB_ID 40 //ID do sensor de Tubr na entrada
#define ENV_COD_ID 1// ID da sensor de carga organica
#define ENV_EM_ID 3//ID da estação meterológica

/*
* Serial monitor debug
*/
#define ENV_MODBUS_DEBUG        0
#define ENV_I2C_DEBUG           0
#define ENV_MQTT_DEBUG          0
#define ENV_SOLO_AP_MODE        0
#define ENV_DRAIN_DEBUG         0


/*
*   Tasks
*/
#define ENV_TASK_NETWORK        1
#define ENV_TASK_MQTT           1
#define ENV_TASK_HTTPSERVER     1
#define ENV_TASK_OTA            1
#define ENV_TASK_I2C            1
#define ENV_TASK_MODBUS         1
#define ENV_TASK_DRAIN          1
