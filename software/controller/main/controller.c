#include <stdio.h>
#include <time.h>
#include "nvs_flash.h"
#include "esp_log.h"

#include "controller.h"
#include "http_server.h"
#include "softap.h"

struct telemetry_struct telemetry = {102.6, 10.2, -40.6, 32.8, -64.2};

char* get_telemetry() {
	// Allocate memory for the string (approximately 256 bytes should be enough)
    char* result = malloc(512 * sizeof(char));
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }
    
    // Format the string with all values separated by commas
    int len = snprintf(result, 512, "%.10f,%.10f,%.10f,%.10f,%.10f",
                       telemetry.altitude,
                       telemetry.speed,
                       telemetry.heading,
                       telemetry.latitude,
                       telemetry.longitude);
    
    // Check if snprintf succeeded
    if (len < 0 || len >= 512) {
        free(result);
        return NULL;
    }
    
    return result;
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
	wifi_init_softap();

    static httpd_handle_t server = NULL;

	// ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
	// ESP_ERROR_CHECK(example_connect());

    /* Register event handlers to stop the server when Wi-Fi or Ethernet is disconnected,
     * and re-start it upon connection.
     */
#if !CONFIG_IDF_TARGET_LINUX
#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
#endif // CONFIG_EXAMPLE_CONNECT_WIFI
#ifdef CONFIG_EXAMPLE_CONNECT_ETHERNET
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_DISCONNECTED, &disconnect_handler, &server));
#endif // CONFIG_EXAMPLE_CONNECT_ETHERNET
#endif // !CONFIG_IDF_TARGET_LINUX

    /* Start the server for the first time */
    server = start_webserver();

    while (server) {
        sleep(5);
    }
}
