#include "HTTPServer.hpp"

namespace HTTPServer
{

    httpd_handle_t http_server = NULL;

    esp_err_t read_req_body(httpd_req_t *req, char *buf)
    {
        int total_len = req->content_len;
        int cur_len = 0;
        while (cur_len < total_len)
        {
            int received = httpd_req_recv(req, buf + cur_len, total_len);
            if (received <= 0)
            {
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive request data");
                free(buf);
                return ESP_FAIL;
            }
            cur_len += received;
        }
        buf[total_len] = '\0';
        return ESP_OK;
    }

    static esp_err_t index_get_handler(httpd_req_t *req)
    {
        extern const uint8_t index_html_start[] asm("_binary_src_web_index_html_start");
        extern const uint8_t index_html_end[] asm("_binary_src_web_index_html_end");
        httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
        return ESP_OK;
    }

    static esp_err_t update_get_handler(httpd_req_t *req)
    {
        queues::I2C_readings_t rcv_i2c_readings;
        queues::Modbus_readings_t rcv_modbus_readings;

        xQueuePeek(queues::i2c_readings, &rcv_i2c_readings, 10 / portTICK_PERIOD_MS);
        xQueuePeek(queues::modbus_readings, &rcv_modbus_readings, 10 / portTICK_PERIOD_MS);

        char packet[400];
        snprintf(packet, 400, "pH: %s \n Temperatura: %.2f\n EC: %.3s\n Turbidez: %.2f\n Carga Orgânica: %.2f\n RPM: %d\n", 
        rcv_i2c_readings.ph, rcv_modbus_readings.temperature, rcv_i2c_readings.ec, rcv_modbus_readings.turbidity, rcv_modbus_readings.COD, rcv_modbus_readings.pump_RMP);
        
        httpd_resp_send(req, (const char *)packet, strlen(packet));
        return ESP_OK;
    }

    static esp_err_t update_post_handler(httpd_req_t *req)
    {
        queues::STA_cred_t inc_cred;
        char *req_body = (char *)malloc((req->content_len + 1) * sizeof(char));
        if (esp_err_t res = read_req_body(req, req_body) != ESP_OK)
        {
            return res;
        }
        Serial.println(req_body);
        cJSON *obj = cJSON_Parse(req_body);
        strcpy(inc_cred.ssid, cJSON_GetObjectItem(obj, "ssid")->valuestring);
        strcpy(inc_cred.password, cJSON_GetObjectItem(obj, "password")->valuestring);
        xQueueSend(queues::sta_cred, &inc_cred, 15 / portTICK_PERIOD_MS);
        cJSON_Delete(obj);
        free(req_body);
        return ESP_OK;
    }

    void registerEndpoints()
    {
        const httpd_uri_t index_get = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = index_get_handler,
            .user_ctx = NULL};
        ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &index_get));

        const httpd_uri_t update_post_fw = {
            .uri = "/update/firmware",
            .method = HTTP_POST,
            .handler = OTA::ota_update_post_handler, // TODO: Edit
            .user_ctx = NULL};
        ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &update_post_fw));

        const httpd_uri_t update_post_cred = {
            .uri = "/update/credentials",
            .method = HTTP_POST,
            .handler = update_post_handler,
            .user_ctx = NULL};
        ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &update_post_cred));

        const httpd_uri_t update_get_param = {
            .uri = "/update/parameters",
            .method = HTTP_GET,
            .handler = update_get_handler,
            .user_ctx = NULL};
        ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &update_get_param));
    }

    void taskHTTPServer(void *pvParameters)
    {
        vTaskDelay(2 * 1000 / portTICK_PERIOD_MS);
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        ESP_ERROR_CHECK(httpd_start(&http_server, &config));
        registerEndpoints();

        Serial.println("HTTPServer: Booted");
        while (true)
        {
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}