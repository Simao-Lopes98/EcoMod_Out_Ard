#include "Network.hpp"

namespace Network
{

    void setup_wifi()
    {
        #if !ENV_SOLO_AP_MODE
        Serial.println();
        Serial.print("Connecting to STA");
        Serial.println(ENV_SSID);

        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(ENV_SSID, ENV_PASSWORD);

        while (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Network: Trying to connect");
        }
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        #endif

        Serial.println("Starting AP");
        Serial.println("ESP_EcoModZHC");

        WiFi.softAP("ESP_EcoModZHC", "123456789");

        Serial.println("");
        Serial.println("Connected to WiFi");
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
    }

    void change_STA_cred(const char *newSsid, const char *newPassword)
    {
        WiFi.disconnect(true,false);
        while (WiFi.status() == WL_CONNECTED)
        {
            delay(1000);
            Serial.println("Disconnecting from current Wi-Fi network...");
        }

        WiFi.begin(newSsid, newPassword);

        Serial.print("Connecting to new Wi-Fi network ");
        Serial.println(newSsid);

        while (WiFi.status() != WL_CONNECTED)//! It can get stuck..
        {
            delay(1000);
            Serial.println("Connecting to new Wi-Fi network...");
        }

        Serial.println("Connected to new Wi-Fi network");
        Serial.print("New STA IP address: ");
        Serial.println(WiFi.localIP());
    }

    void taskNetwork(void *pvParameters)
    {
        setup_wifi();

        Serial.println("Network: Booted");
        while (true)
        {
            if (uxQueueMessagesWaiting(queues::sta_cred))
            {
                queues::STA_cred_t inc_cred;
                xQueueReceive(queues::sta_cred, &inc_cred, 10 / portTICK_PERIOD_MS);
                change_STA_cred(inc_cred.ssid,inc_cred.password);
            }
            vTaskDelay(2500 / portTICK_PERIOD_MS);
        }
    }
}