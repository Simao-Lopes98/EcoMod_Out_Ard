#include "OTA.hpp"

namespace OTA
{
    esp_err_t ota_update_post_handler(httpd_req_t *req)
    {
        char buf[1000];
        esp_ota_handle_t ota_handle;
        int remaining = req->content_len;

        const esp_partition_t *ota_partition = esp_ota_get_next_update_partition(NULL);
        Serial.println("OTA: Partition to update: " + String(ota_partition->label));

        esp_ota_begin(ota_partition, 1000000, &ota_handle);// 

        Serial.println("Starting the uptade");
        int time_out_counter = 0;

        while (remaining > 0)
        {
            int recv_len = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));

            // Timeout Error: Just retry
            if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
            {
                if(time_out_counter < 5)
                {
                    Serial.println("OTA: Timeout");
                    time_out_counter++;
                    continue;
                }else{
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Validation / Activation Error");
                    return ESP_FAIL;
                }

                
                

                // Serious Error: Abort OTA
            }
            else if (recv_len <= 0)
            {
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Protocol Error");
                return ESP_FAIL;
            }

            // Successful Upload: Flash firmware chunk
            if (esp_ota_write(ota_handle, (const void *)buf, recv_len) != ESP_OK)
            {
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Flash Error");
                return ESP_FAIL;
            }

            remaining -= recv_len;
        }

        // Validate and switch to new OTA image and reboot
        if (esp_ota_end(ota_handle) != ESP_OK || esp_ota_set_boot_partition(ota_partition) != ESP_OK)
        {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Validation / Activation Error");
            return ESP_FAIL;
        }

        httpd_resp_sendstr(req, "Firmware update complete, rebooting now!\n");

        vTaskDelay(500 / portTICK_PERIOD_MS);
        esp_restart();

        return ESP_OK;
    }

    void taskOTA(void *pvParameters)
    {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

        /* Mark current app as valid */
        const esp_partition_t *partition = esp_ota_get_running_partition();
        Serial.println("OTA: Currently running partition: "+ String(partition->label));

        esp_ota_img_states_t ota_state;
        if (esp_ota_get_state_partition(partition, &ota_state) == ESP_OK)
        {
            if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
            {
                esp_ota_mark_app_valid_cancel_rollback();
            }
        }

        const esp_partition_t *ota_partition = esp_ota_get_next_update_partition(NULL);
        if (ota_partition == NULL)
        {
            Serial.println("OTA: Invalid OTA partition");
        }
        else
        {
            Serial.println("OTA: Partition to update "+ String(ota_partition->label));
        }
       Serial.println("OTA: Booted");

        while (true)
        {
            vTaskDelay(2500 / portTICK_PERIOD_MS);
        }
    }
}