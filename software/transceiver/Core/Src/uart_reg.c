#include "uart_reg.h"
#include "stm32u5xx_hal_uart.h"
#include "sx126x.h"
#include <stdint.h>

// TODO: Add error handling.

const void *REG_RF_CONTEXT = 0;

uint8_t *reg_buf;

int reg_who_am_i_h(UART_HandleTypeDef *huart, settings_t *settings) {
	uint8_t id = 126;
	HAL_UART_Transmit(huart, &id, 1, UART_TIMEOUT);
	return 0;
}

int reg_tx_h(UART_HandleTypeDef *huart, settings_t *settings) {
	uint8_t size;

	HAL_UART_Receive(huart, &size, 1, UART_TIMEOUT);

	HAL_UART_Receive(huart, reg_buf, size, UART_TIMEOUT);

	sx126x_write_buffer(REG_RF_CONTEXT, 0, reg_buf, size);

	sx126x_set_tx(REG_RF_CONTEXT, 1000);

	return 0;
}

int reg_rx_h(UART_HandleTypeDef *huart, settings_t *settings) {
	uint8_t size;

	HAL_UART_Receive(huart, &size, 1, UART_TIMEOUT);

	sx126x_set_rx(REG_RF_CONTEXT, 1000);

	sx126x_read_buffer(REG_RF_CONTEXT, 0, reg_buf, size);

	HAL_UART_Transmit(huart, reg_buf, size, 500);

	return 0;
}

int reg_tx_set_h(UART_HandleTypeDef *huart, settings_t *settings) {
	const uint16_t SIZE = settings->payload_len;

	HAL_UART_Receive(huart, reg_buf, SIZE, UART_TIMEOUT);

	sx126x_write_buffer(REG_RF_CONTEXT, 0, reg_buf, SIZE);

	sx126x_set_tx(REG_RF_CONTEXT, 1000);

	return 0;
}

int reg_rx_set_h(UART_HandleTypeDef *huart, settings_t *settings) {
	const uint16_t SIZE = settings->payload_len;

	sx126x_set_rx(REG_RF_CONTEXT, 1000);

	sx126x_read_buffer(REG_RF_CONTEXT, 0, reg_buf, SIZE);

	HAL_UART_Transmit(huart, reg_buf, SIZE, 500);

	return 0;
}

int reg_tx_rx_h(UART_HandleTypeDef *huart, settings_t *settings) {
	const uint16_t SIZE = 32;

	HAL_UART_Receive(huart, reg_buf, SIZE, UART_TIMEOUT);

	sx126x_write_buffer(REG_RF_CONTEXT, 0, reg_buf, SIZE);

	sx126x_set_tx(REG_RF_CONTEXT, 1000);

	sx126x_set_rx(REG_RF_CONTEXT, 1000);

	sx126x_read_buffer(REG_RF_CONTEXT, 0, reg_buf, SIZE);

	HAL_UART_Transmit(huart, reg_buf, SIZE, 500);

	return 0;
}

int reg_sleep_h(UART_HandleTypeDef *huart, settings_t *settings) {
	// TODO: Put STM32 in some sort of sleep mode too.
	sx126x_set_sleep(REG_RF_CONTEXT, settings->sleep_cfg);
	return 0;
}

int reg_wakeup_h(UART_HandleTypeDef *huart, settings_t *settings) {
	sx126x_wakeup(REG_RF_CONTEXT);

	return 0;
}

int reg_set_all_h(UART_HandleTypeDef *huart, settings_t *settings) {
	// TODO: Find this size and replace with constant value.
	const uint16_t SIZE = sizeof(settings_t);

	HAL_UART_Receive(huart, reg_buf, SIZE, UART_TIMEOUT);

	settings = (settings_t *)reg_buf;

	sx126x_set_rf_freq(REG_RF_CONTEXT, settings->frequency);
	sx126x_set_tx_params(REG_RF_CONTEXT, settings->power, settings->ramp_time);

	sx126x_mod_params_lora_t lora_mod_params = {
		settings->spreading_factor, settings->bandwidth, settings->coding_rate,
		settings->low_datarate_optimization
	};
	sx126x_set_lora_mod_params(REG_RF_CONTEXT, &lora_mod_params);

	sx126x_pkt_params_lora_t lora_pkt_params = {
		settings->preamble_len, settings->header_type, settings->payload_len,
		settings->crc_en, settings->invert_iq
	};
	sx126x_set_lora_pkt_params(REG_RF_CONTEXT, &lora_pkt_params);

	sx126x_set_lora_symb_nb_timeout(REG_RF_CONTEXT, settings->lora_symb_timeout);

	return 0;
}

int reg_set_frequency_h(UART_HandleTypeDef *huart, settings_t *settings) {
	const uint16_t SIZE = 4;

	HAL_UART_Receive(huart, reg_buf, SIZE, UART_TIMEOUT);

	const uint32_t frequency = (uint32_t)reg_buf;

	sx126x_set_rf_freq(REG_RF_CONTEXT, frequency);

	settings->frequency = frequency;

	return 0;
}

int reg_set_tx_params_h(UART_HandleTypeDef *huart, settings_t *settings) {
	const uint16_t SIZE = 2;

	HAL_UART_Receive(huart, reg_buf, SIZE, UART_TIMEOUT);

	const uint8_t power = reg_buf[0];
	const sx126x_ramp_time_t ramp_time = reg_buf[1];

	sx126x_set_tx_params(REG_RF_CONTEXT, power, ramp_time);

	settings->power = power;
	settings->ramp_time = ramp_time;

	return 0;
}

int reg_set_lora_mod_params_h(UART_HandleTypeDef *huart, settings_t *settings) {
	// TODO: Find this size and replace with constant value.
	const uint16_t SIZE = sizeof(sx126x_mod_params_lora_t);

	HAL_UART_Receive(huart, reg_buf, SIZE, UART_TIMEOUT);

	const sx126x_mod_params_lora_t *lora_mod_params =
		(sx126x_mod_params_lora_t *)reg_buf;

	sx126x_set_lora_mod_params(REG_RF_CONTEXT, lora_mod_params);

	settings->spreading_factor = lora_mod_params->sf;
	settings->bandwidth = lora_mod_params->bw;
	settings->coding_rate = lora_mod_params->cr;
	settings->low_datarate_optimization = lora_mod_params->ldro;

	return 0;
}

int reg_set_lora_pkt_params_h(UART_HandleTypeDef *huart, settings_t *settings) {
	// TODO: Find this size and replace with constant value.
	const uint16_t SIZE = sizeof(sx126x_pkt_params_lora_t);

	HAL_UART_Receive(huart, reg_buf, SIZE, UART_TIMEOUT);

	const sx126x_pkt_params_lora_t *lora_pkt_params =
		(sx126x_pkt_params_lora_t *)reg_buf;

	sx126x_set_lora_pkt_params(REG_RF_CONTEXT, lora_pkt_params);

	settings->preamble_len = lora_pkt_params->preamble_len_in_symb;
	settings->header_type = lora_pkt_params->header_type;
	settings->payload_len = lora_pkt_params->pld_len_in_bytes;
	settings->crc_en = lora_pkt_params->crc_is_on;
	settings->invert_iq = lora_pkt_params->invert_iq_is_on;

	return 0;
}

int reg_set_lora_symb_nb_timeout_h(UART_HandleTypeDef *huart, settings_t *settings) {
	const uint16_t SIZE = 1;

	HAL_UART_Receive(huart, reg_buf, SIZE, UART_TIMEOUT);

	const uint8_t lora_symb_timeout = reg_buf[0];

	sx126x_set_lora_symb_nb_timeout(REG_RF_CONTEXT, lora_symb_timeout);

	settings->lora_symb_timeout = lora_symb_timeout;

	return 0;
}

// TODO: Add encryption.
int reg_set_encrypt_params_h(UART_HandleTypeDef *huart, settings_t *settings) {
	return 0;
}

int reg_enable_encrypt_h(UART_HandleTypeDef *huart, settings_t *settings) {
	return 0;
}

// int foo = 1;

// Array of pointers to register handles.
int (*reg_handle[])(UART_HandleTypeDef *huart, settings_t *settings) = {
	reg_who_am_i_h,
	reg_tx_h,
	reg_rx_h,
	reg_tx_set_h,
	reg_rx_set_h,
	// reg_tx_rx_h,
	reg_sleep_h,
	reg_wakeup_h,
	reg_set_all_h,
	reg_set_frequency_h,
	reg_set_tx_params_h,
	reg_set_lora_mod_params_h,
	reg_set_lora_pkt_params_h,
	reg_set_lora_symb_nb_timeout_h,
	reg_set_encrypt_params_h,
	reg_enable_encrypt_h
};
