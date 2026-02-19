#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdlib.h>
#include <unistd.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include "esp_tls.h"
#include <sys/time.h>
#include "http_server.h"
#if !CONFIG_IDF_TARGET_LINUX
#include <esp_wifi.h>
#include <esp_system.h>
#include "esp_eth.h"
#endif  // !CONFIG_IDF_TARGET_LINUX

// static httpd_handle_t start_webserver(void);
httpd_handle_t start_webserver(void);

esp_err_t stop_webserver(httpd_handle_t server);

void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);

void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data);

#endif // HTTP_SERVER_H
