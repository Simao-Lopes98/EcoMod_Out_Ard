#pragma once

#include <Arduino.h>
#include <esp_ota_ops.h>
#include "esp_log.h"
#include <esp_http_server.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>

namespace OTA
{
    esp_err_t ota_update_post_handler(httpd_req_t *req);

    void taskOTA(void *pvParameters);

}