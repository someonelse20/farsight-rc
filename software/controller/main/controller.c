#include "controller.h"
#include <stdio.h>
#include <ssd1306.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/i2c_types.h"
#include "driver/i2c_master.h"

#include "softap.h"
#include "http_server.h"

static const char *TAG = "Controller Main";

telemetry_t telemetry;
controls_t controls;

static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle);
static void poll_inputs(void);

void app_main(void)
{
	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Start I2C
	i2c_master_bus_handle_t i2c0_bus_hdl;
	i2c_master_dev_handle_t i2c0_dev_hdl;
	i2c_master_init(&i2c0_bus_hdl, &i2c0_dev_hdl);
	ESP_LOGI(TAG, "I2C initialized successfully");

	TickType_t last_wake_time   = xTaskGetTickCount ();

	ssd1306_config_t dev_cfg         = I2C_SSD1306_128x64_CONFIG_DEFAULT;
	ssd1306_handle_t dev_hdl;

	// Start WIFI AP
	ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
	wifi_init_softap();

	// Start HTTP server
	static httpd_handle_t server = NULL;
	server = start_webserver();

	// Start OLED display
	ssd1306_init(i2c0_bus_hdl, &dev_cfg, &dev_hdl);
	if (dev_hdl == NULL) {
		ESP_LOGE(TAG, "ssd1306 handle init failed");
		// assert(dev_hdl);
	}
}

char* get_telemetry() {
	char* result = malloc(128 * sizeof(char));

	int len = snprintf(result, 128, "%.2i,%.2i,%.2i,%.8f,%.8f",
	                   telemetry.altitude,
	                   telemetry.speed,
	                   telemetry.heading,
	                   telemetry.latitude,
	                   telemetry.longitude);

	// Check if snprintf succeeded
	if (len < 0 || len >= 128) {
		free(result);
		return NULL;
	}

	return result;
}
static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle)
{
	i2c_master_bus_config_t bus_config = {
		.i2c_port = I2C_MASTER_NUM,
		.sda_io_num = I2C_MASTER_SDA_IO,
		.scl_io_num = I2C_MASTER_SCL_IO,
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.glitch_ignore_cnt = 7,
		.flags.enable_internal_pullup = true,
	};
	ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));

	i2c_device_config_t dev_config = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = SSD1306_ADDR,
		.scl_speed_hz = I2C_MASTER_FREQ_HZ,
	};
	ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
}

static void poll_inputs(void) {

}

