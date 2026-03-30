#include "controller.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <ssd1306.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hal/adc_types.h"
#include "nvs_flash.h"
#include "driver/i2c_types.h"
#include "driver/i2c_master.h"
#include "esp_adc/adc_continuous.h"

#include "soc/soc_caps.h"
#include "softap.h"
#include "http_server.h"

#define ADC_SAMPLE_LEN 64

static const char *TAG = "Controller Main";

telemetry_t telemetry;
controls_t controls;

double joy_1_x;
double joy_1_y;
double joy_2_x;
double joy_2_y;
double bat_v;

static void adc_master_init(adc_continuous_handle_t adc_handle);
static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle);

static double convert_adc_data(uint32_t raw_data);
static void poll_adc(adc_continuous_handle_t handle);
static void poll_buttons(void);

void app_main(void)
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Start ADC
	adc_continuous_handle_t adc_handle = NULL;
	adc_master_init(adc_handle);

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

	// Start ADC measurment
	ESP_ERROR_CHECK(adc_continuous_start(adc_handle));

	while (1) {

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

static void adc_master_init(adc_continuous_handle_t adc_handle) {
	adc_continuous_handle_cfg_t adc_handle_config = {
		.max_store_buf_size = 1024,
		.conv_frame_size = 256,
	};
	ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_handle_config, &adc_handle));

	adc_channel_t adc_channel[5] = {
		ADC_CHANNEL_0,
		ADC_CHANNEL_1,
		ADC_CHANNEL_2,
		ADC_CHANNEL_3,
		ADC_CHANNEL_6
	};

	adc_continuous_config_t dig_cfg = {
		.sample_freq_hz = 20 * 1000,
		.conv_mode = ADC_CONV_SINGLE_UNIT_1,
	};

	uint8_t channel_num = 5;

	adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
	dig_cfg.pattern_num = channel_num;
	for (int i = 0; i < channel_num; i++) {
		adc_pattern[i].atten = ADC_ATTEN_DB_0;
		adc_pattern[i].channel = adc_channel[i]; // & 0x7;
		adc_pattern[i].unit = ADC_UNIT_1;
		adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
	}
	dig_cfg.adc_pattern = adc_pattern;
	ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &dig_cfg));
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

static double convert_adc_data(uint32_t raw_data) {
	return raw_data * 3.3 / pow(2, SOC_ADC_DIGI_MAX_BITWIDTH);
}

static void poll_adc(adc_continuous_handle_t handle) {
	adc_continuous_data_t adc_raw_data[ADC_SAMPLE_LEN];
	uint32_t num_samples = 0;

	esp_err_t ret = adc_continuous_read_parse(handle, adc_raw_data, ADC_SAMPLE_LEN, &num_samples, 50);
	if (ret == ESP_OK) {
		joy_1_x = convert_adc_data(adc_raw_data[0].raw_data);
		joy_1_y = convert_adc_data(adc_raw_data[1].raw_data);
		joy_2_x = convert_adc_data(adc_raw_data[2].raw_data);
		joy_2_y = convert_adc_data(adc_raw_data[3].raw_data);
		bat_v = convert_adc_data(adc_raw_data[4].raw_data);
	}
}

static void poll_buttons(void) {

}

