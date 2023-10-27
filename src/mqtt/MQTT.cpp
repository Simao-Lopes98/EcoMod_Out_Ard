#include "MQTT.hpp"

namespace MQTT
{
    WiFiClient espClient;
    PubSubClient client(espClient);

    queues::I2C_readings_t rcv_i2c_readings;
    queues::Modbus_readings_t rcv_modbus_readings;
    char packet[512];
    char EMPacket[512];

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
                client.publish("sensors/input", packet);
                Serial.println("MQTT: Packet sent");
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
            xQueuePeek(queues::i2c_readings, &rcv_i2c_readings, 100 / portTICK_PERIOD_MS);
        }
        if(uxQueueMessagesWaiting(queues::modbus_readings))
        {
            xQueuePeek(queues::modbus_readings, &rcv_modbus_readings, 100 / portTICK_PERIOD_MS);
        }

        //Submerged sensors
        snprintf(packet,512, "{\"ref\":\"sensIN\", \"pH\":\"%s\", \"temperatura\":\"%.2f\", \"EC\":\"%.3s\", \"Turb\":\"%.2f\", \"COD\":\"%.2f\", \"RPM\":\"%d\" }"
        ,rcv_i2c_readings.ph, rcv_modbus_readings.temperature, rcv_i2c_readings.ec,rcv_modbus_readings.turbidity,rcv_modbus_readings.COD,rcv_modbus_readings.pump_RMP);
        
        //EM
        snprintf(EMPacket,512, "{\"ref\":\"estacaoM\", \"AWD\":\"%.2f\", \"AWS\":\"%.2f\", \"AT\":\"%.2f\", \"AH\":\"%.2f\", \"AP\":\"%.2f\", \"RF\":\"%.2f\", \"Rad\":\"%.2f\", \"UV\":\"%.2f\" }"
        ,rcv_modbus_readings.EM_readings[1],rcv_modbus_readings.EM_readings[4],rcv_modbus_readings.EM_readings[6],rcv_modbus_readings.EM_readings[7],rcv_modbus_readings.EM_readings[8],rcv_modbus_readings.EM_readings[9],rcv_modbus_readings.EM_readings[10],rcv_modbus_readings.EM_readings[11]);

    
    }
    void initialize_values()
    {
        strcpy(rcv_i2c_readings.ec,"0");
        strcpy(rcv_i2c_readings.ph,"0.0");
        rcv_modbus_readings.COD=0.0;
        rcv_modbus_readings.turbidity=0.0;
        rcv_modbus_readings.temperature=0.0;
        rcv_modbus_readings.pump_RMP=0;
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
            client.publish("sensors/input", packet);
            client.publish("sensors/EM", EMPacket);
            #if ENV_MQTT_DEBUG
                Serial.printf("MQTT: Packet sent: %s",packet);
                Serial.printf("MQTT: Packet sent: %s",EMPacket);
            #endif
            vTaskDelay(ENV_SEND_PERIOD_SEC * 1000 / portTICK_PERIOD_MS);
        }
    }
}
