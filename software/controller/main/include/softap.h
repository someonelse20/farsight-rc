#ifndef WIFI_SOFTAP_H
#define WIFI_SOFTAP_H

#include "esp_err.h"

/**
 * @brief Initialize and start Wi-Fi in softAP mode
 *
 * This function initializes the Wi-Fi driver, sets up the softAP configuration,
 * registers event handlers, and starts the Wi-Fi access point.
 */
void wifi_init_softap(void);

static const char *TAG = "wifi softAP";

#endif // WIFI_SOFTAP_H
