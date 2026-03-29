#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <esp_http_server.h>

// static httpd_handle_t start_webserver(void);
httpd_handle_t start_webserver(void);

esp_err_t stop_webserver(httpd_handle_t server);

void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);

void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data);

#endif // HTTP_SERVER_H
