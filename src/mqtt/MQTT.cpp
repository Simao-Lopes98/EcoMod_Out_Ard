#include "MQTT.hpp"

namespace MQTT
{
    WiFiClient espClient;
    PubSubClient client(espClient);

    queues::I2C_readings_t rcv_i2c_readings;
    queues::Modbus_readings_t rcv_modbus_readings;
    uint16_t ncyc = 0;
    char packet[512];
    char pumpPacket[256];
    char EmPacket[512];
    uint16_t msg_time_period = ENV_SEND_PERIOD_SEC;

    void callback(char *topic, byte *payload, unsigned int length)
    {
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        for (int i = 0; i < length; i++)
        {
            Serial.print((char)payload[i]);
        }
        Serial.println();
    }

    void reconnect()
    {
        while (!client.connected())
        {
            Serial.print("Attempting MQTT connection...");
            // Create a random client ID
            String clientId = "ESP32-Simao";
            clientId += String(random(0xffff), HEX);
            // Attempt to connect
            if (client.connect(clientId.c_str()))
            {
                process_data();
                client.publish(ENV_SENS_OUT_TOPIC, packet);
                #if ENV_MQTT_DEBUG
                    Serial.println("MQTT: Sensors Topic sent");
                #endif

                client.publish(ENV_PUMP_TOPIC, pumpPacket);
                #if ENV_MQTT_DEBUG
                    Serial.println("MQTT: Pump Topic sent");
                #endif

                client.publish(ENV_EM_TOPIC, EmPacket);
                #if ENV_MQTT_DEBUG
                    Serial.println("MQTT: EM Topic sent");
                #endif
            }
            else
            {
                Serial.print("failed, rc=");
                Serial.print(client.state());
                Serial.println(" try again in 5 seconds");
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }
        }
    }

    void process_data()
    {
        if(uxQueueMessagesWaiting(queues::i2c_readings))
        {
            xQueuePeek(queues::i2c_readings, &rcv_i2c_readings, 5 / portTICK_PERIOD_MS);
        }
        if(uxQueueMessagesWaiting(queues::modbus_readings))
        {
            xQueuePeek(queues::modbus_readings, &rcv_modbus_readings, 5 / portTICK_PERIOD_MS);
        }
        if(uxQueueMessagesWaiting(queues::ncycles))
        {
            xQueuePeek(queues::ncycles,&ncyc,5/portTICK_PERIOD_MS);
        }

        //Submerged sensors
        snprintf(packet,sizeof(packet), "{\"ref\":\"sensOUT\", \"pH\":\"%s\", \"temperatura\":\"%.2f\", \"EC\":\"%.3s\", \"Turb\":\"%.2f\", \"COD\":\"%.2f\", \"Cic\":\"%d\" }"
        ,rcv_i2c_readings.ph, rcv_modbus_readings.temperature, rcv_i2c_readings.ec,rcv_modbus_readings.turbidity,rcv_modbus_readings.COD, (int)ncyc);

        snprintf(pumpPacket,sizeof(pumpPacket),"{\"ref\":\"pumpIN\", \"RPM\":\"%d\" }", rcv_modbus_readings.pump_RMP);
        
        snprintf(EmPacket,sizeof(EmPacket), "{\"ref\":\"EM\", \"AWD\":\"%.2f\", \"AWS\":\"%.2f\", \"AT\":\"%.2f\", \"AH\":\"%.2f\", \"AP\":\"%.2f\", \"RF\":\"%.2f\", \"UV\":\"%.2f\", \"Rad\":\"%.2f\" }"
        ,rcv_modbus_readings.EM_readings[1],rcv_modbus_readings.EM_readings[4],rcv_modbus_readings.EM_readings[6],rcv_modbus_readings.EM_readings[7],rcv_modbus_readings.EM_readings[8],rcv_modbus_readings.EM_readings[9],rcv_modbus_readings.EM_readings[11],rcv_modbus_readings.EM_readings[10]);
    
    }
    
    void initialize_values()
    {
        strcpy(rcv_i2c_readings.ec,"0");
        strcpy(rcv_i2c_readings.ph,"0.0");
        rcv_modbus_readings.COD = 0.0;
        rcv_modbus_readings.turbidity = 0.0;
        rcv_modbus_readings.temperature = 0.0;
        rcv_modbus_readings.pump_RMP = 0;
        rcv_modbus_readings.EM_readings[0] = 0.0;
        rcv_modbus_readings.EM_readings[1] = 0.0;
        rcv_modbus_readings.EM_readings[2] = 0.0;
        rcv_modbus_readings.EM_readings[3] = 0.0;
        rcv_modbus_readings.EM_readings[4] = 0.0;
        rcv_modbus_readings.EM_readings[5] = 0.0;
        rcv_modbus_readings.EM_readings[6] = 0.0;
        rcv_modbus_readings.EM_readings[7] = 0.0;
        rcv_modbus_readings.EM_readings[8] = 0.0;
        rcv_modbus_readings.EM_readings[9] = 0.0;
        rcv_modbus_readings.EM_readings[10] = 0.0;
        rcv_modbus_readings.EM_readings[11] = 0.0;
    }

    void taskMQTT(void *pvParameters) // Task de envio de parametros para o broker
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        client.setServer(ENV_MQTT_BROKER, 1883);
        client.setCallback(callback);
        initialize_values();
        Serial.println("MQTT: Booted");
        while (true)
        {
            if (!client.connected())
            {
                reconnect();
            }
            client.loop();

            process_data();
            client.publish(ENV_SENS_OUT_TOPIC, packet);
            #if ENV_MQTT_DEBUG
                Serial.printf("MQTT: Sensors packet sent: %s",packet);
            #endif
            vTaskDelay(250 / portTICK_PERIOD_MS);

            client.publish(ENV_PUMP_TOPIC, pumpPacket);
            #if ENV_MQTT_DEBUG
                Serial.printf("MQTT: Pump packet sent: %s",pumpPacket);
            #endif
            vTaskDelay(250 / portTICK_PERIOD_MS);

            client.publish(ENV_EM_TOPIC, EmPacket);
            #if ENV_MQTT_DEBUG
                Serial.printf("MQTT: EM packet sent: %s",EmPacket);
            #endif

            if(uxQueueMessagesWaiting(queues::msg_period))
            {
                xQueueReceive(queues::msg_period,&msg_time_period,10 / portTICK_PERIOD_MS);
                #if ENV_MQTT_DEBUG
                    Serial.printf("MQTT: Message sending waiting time updated to: %"PRIu16"\n",msg_time_period);
                #endif
            }

            vTaskDelay(msg_time_period * 1000 / portTICK_PERIOD_MS);
        }
    }
}
