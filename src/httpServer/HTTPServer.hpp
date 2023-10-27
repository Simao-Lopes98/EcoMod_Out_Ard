
#pragma once
#include <Arduino.h>
#include "../queues/queues.hpp"
#include <freertos/FreeRTOS.h>
#include <esp_http_server.h>
#include <cJSON.h>
#include "../ota/OTA.hpp"

namespace HTTPServer
{

    esp_err_t read_req_body(httpd_req_t *req, char *buf);

    static esp_err_t index_get_handler(httpd_req_t *req);

    static esp_err_t update_post_handler(httpd_req_t *req);

    static esp_err_t update_get_handler(httpd_req_t *req);

    void registerEndpoints();

    void taskHTTPServer(void *pvParameters);
}