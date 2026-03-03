#ifndef __REG_H
#define __REG_H

#include "stm32u5xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define UART_TIMEOUT 500

typedef struct settings_s {
  uint64_t encrpt_key;
  uint32_t frequency;
  uint16_t preamble_len;
  uint8_t power;
  uint8_t ramp_time;
  uint8_t spreading_factor;
  uint8_t bandwidth;
  uint8_t coding_rate;
  uint8_t low_datarate_optimization;
  uint8_t header_type;
  uint8_t payload_len;
  uint8_t lora_symb_timeout;
  uint8_t sleep_cfg;
  uint8_t log_level; // 0 = none, 1 = error, 2 = warning, 3 = info, 4 = debug.
  bool crc_en;
  bool invert_iq;
  bool auto_rx;
  bool encrypt_en;
} settings_t;

// Register handle functions. 0 = OK, 1 == error.

int reg_tx_h(UART_HandleTypeDef *huart, settings_t *settings);
int reg_rx_h(UART_HandleTypeDef *huart, settings_t *settings);
int reg_tx_rx_h(UART_HandleTypeDef *huart, settings_t *settings);
int reg_sleep_h(UART_HandleTypeDef *huart, settings_t *settings);
int reg_wakeup_h(UART_HandleTypeDef *huart, settings_t *settings);
int reg_set_all_h(UART_HandleTypeDef *huart, settings_t *settings);
int reg_set_frequency_h(UART_HandleTypeDef *huart, settings_t *settings);
int reg_set_tx_params_h(UART_HandleTypeDef *huart, settings_t *settings);
int reg_set_lora_mod_params_h(UART_HandleTypeDef *huart, settings_t *settings);
int reg_set_lora_pkt_params_h(UART_HandleTypeDef *huart, settings_t *settings);
int reg_set_lora_symb_nb_timeout_h(UART_HandleTypeDef *huart,
                                   settings_t *settings);
int reg_set_encrypt_params_h(UART_HandleTypeDef *huart, settings_t *settings);
int reg_enable_encrypt_h(UART_HandleTypeDef *huart, settings_t *settings);

// Array of pointers to register handles.
extern int (*reg_handle[])(UART_HandleTypeDef *huart, settings_t *settings);

#endif
