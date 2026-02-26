/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 * NOTE: MOST OF THE ERROR LOGGING IS FOR TESTING ONLY
 *       Remove most of it when done so it is actually readable.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stm32u585xx.h"
#include "stm32u5xx_hal_def.h"
#include "stm32u5xx_hal_gpio.h"
#include "stm32u5xx_hal_spi.h"
#include "stm32u5xx_hal_uart.h"
#include "sx126x.h"
#include "sx126x_hal.h"
#include "sx126x_status.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define BUFFER_SIZE 32

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

// TODO: Figure out what const void *context is.
// I think it's to allow using multiple radio modules.
const void *rf_context = 0;

uint8_t *buf;

typedef struct settings_s {
  uint32_t frequency;
  uint16_t preamble_len;
  uint8_t power;
  uint8_t ramptime;
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
} settings_t;

settings_t default_settings = {
    915000,
    8,
    22,
    SX126X_RAMP_20_US,
    SX126X_LORA_SF7,
    SX126X_LORA_BW_125,
    SX126X_LORA_CR_4_8,
    0, // NOTE: Check to see if disabling low data rate optimization is needed
       // for thermal performance.
    SX126X_LORA_PKT_EXPLICIT,
    0xFF, // NOTE: I'm not sure if this is needed because packet type is
          // explicit.
    8,
    SX126X_SLEEP_CFG_COLD_START,
    0,
    1,
    0,
};

settings_t *global_settings;

char *debug_msg;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ICACHE_Init(void);
/* USER CODE BEGIN PFP */

int log_msg(char *msg, uint8_t log_level);
static void lora_tx_rx(uint8_t *data);
static void lora_set_config(settings_t *settings);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_ICACHE_Init();
  /* USER CODE BEGIN 2 */

  lora_set_config(&default_settings);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // Receive data and leave one extra byte at the end for commands with a
    // 500ms timeout.
    HAL_UART_Receive(&huart1, buf, BUFFER_SIZE + 1, 500);

    const uint8_t command = buf[32];

    // Create new data array without command byte.
    uint8_t data[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
      data[i] = buf[i];
    }

    if (command == 1) { // Enter sleep mode.
      if (global_settings->log_level > 2) {
        log_msg("SX1262 entering sleep mode.", 3);
      }
      if (sx126x_set_sleep(rf_context, global_settings->sleep_cfg) !=
              SX126X_STATUS_OK &&
          global_settings->log_level > 0) {
        log_msg("Error with SX1262.", 1);
      }
    } else if (command == 2) { // Wakeup from sleep mode.
      if (global_settings->log_level > 2) {
        log_msg("Waking up SX1262.", 3);
      }
      if (sx126x_wakeup(rf_context) == SX126X_STATUS_OK &&
          global_settings->log_level > 0) {
        log_msg("Error waking up SX1262.", 1);
      }
    } else if (command == 3) { // Set radio settings.
      if (global_settings->log_level > 2) {
        log_msg("Setting SX1262 config.", 3);
      }
      lora_set_config((settings_t *)data);
    } else { // If command byte is 0 or invalid perform normal radio operation.
      lora_tx_rx(data);

      // Send data back with datatype byte.
      const uint8_t datatype = 0;

      // Create new data array with command byte.
      uint8_t rx_data[BUFFER_SIZE + 1];
      for (int i = 0; i < BUFFER_SIZE; i++) {
        rx_data[i] = data[i];
      }
      rx_data[BUFFER_SIZE] = datatype;

      // Print RX data.
      HAL_UART_Transmit(&huart1, rx_data, BUFFER_SIZE + 1, 100);
    }
  }

  if (global_settings->log_level > 3) {
    log_msg("End of loop.", 4);
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE4) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief ICACHE Initialization Function
 * @param None
 * @retval None
 */
static void MX_ICACHE_Init(void) {

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache (default 2-ways set associative cache)
   */
  if (HAL_ICACHE_Enable() != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */
}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void) {

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = {0};

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_INPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x7;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  hspi1.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
  hspi1.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) {
    Error_Handler();
  }
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerState = SPI_AUTO_MODE_DISABLE;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerSelection =
      SPI_GRP1_GPDMA_CH0_TCF_TRG;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerPolarity = SPI_TRIG_POLARITY_RISING;
  if (HAL_SPIEx_SetConfigAutonomousMode(
          &hspi1, &HAL_SPI_AutonomousMode_Cfg_Struct) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) !=
      HAL_OK) {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) !=
      HAL_OK) {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

int log_msg(char *msg, uint8_t log_level) {
  size_t msg_len = sizeof(&msg);
  char data[BUFFER_SIZE + 1];

  // Check to see if log_level is valid.
  if (log_level < 1) {
    return 1;
  }

  // Check to see if msg_len is valid.
  if (msg_len <= BUFFER_SIZE) {
    for (int i = 0; i < msg_len; i++) {
      if (i < msg_len) {
        data[i] = msg[i];
      } else {
        data[i] = 0;
      }
    }
    // Add log level to data.
    data[BUFFER_SIZE] = log_level;
    if (HAL_UART_Transmit(&huart1, (uint8_t *)data, BUFFER_SIZE, 100) ==
        HAL_OK) {
      return 0;
    }
  }

  return 1;
}

static void lora_tx_rx(uint8_t *data) {
  if (global_settings->log_level > 2) {
    log_msg("Transmitting data.", 3);
  }

  // Write data to TX buffer.
  if (sx126x_write_buffer(rf_context, 0, data, BUFFER_SIZE) ==
          SX126X_STATUS_OK &&
      global_settings->log_level > 0) {
    log_msg("Error writing to SX1262 buffer.", 1);
  }

  // Transmit data with a 1s timeout.
  if (sx126x_set_tx(rf_context, 1000) == SX126X_STATUS_OK &&
      global_settings->log_level > 0) {
    log_msg("Error setting SX1262 to tx.", 1);
  }

  if (global_settings->log_level > 2) {
    log_msg("Receiving data.", 3);
  }
  // Receive data with a 1s timeout.
  if (sx126x_set_rx(rf_context, 1000) == SX126X_STATUS_OK &&
      global_settings->log_level > 0) {
    log_msg("Error setting to SX1262 to rx", 1);
  }

  // Read RX buffer data.
  if (sx126x_read_buffer(rf_context, 0, data, BUFFER_SIZE) ==
          SX126X_STATUS_OK &&
      global_settings->log_level > 0) {
    log_msg("Error reading SX1262 buffer", 1);
  }
}

static void lora_set_config(settings_t *settings) {
  if (global_settings->log_level > 2) {
    log_msg("Configuring SX1262.", 3);
  }

  global_settings = settings;

  sx126x_set_rf_freq(rf_context, settings->frequency);
  sx126x_set_tx_params(rf_context, settings->power, settings->ramptime);

  sx126x_mod_params_lora_t lora_mod_params = {
      settings->spreading_factor, settings->bandwidth, settings->coding_rate,
      settings->low_datarate_optimization};
  sx126x_set_lora_mod_params(rf_context, &lora_mod_params);

  sx126x_pkt_params_lora_t lora_pkt_params = {
      settings->preamble_len, settings->header_type, settings->payload_len,
      settings->crc_en, settings->invert_iq};
  sx126x_set_lora_pkt_params(rf_context, &lora_pkt_params);

  sx126x_set_lora_symb_nb_timeout(rf_context, settings->lora_symb_timeout);
}

/* SX126X hal funcion definitions --------------------------------------------*/
sx126x_hal_status_t sx126x_hal_write(const void *context,
                                     const uint8_t *command,
                                     const uint16_t command_length,
                                     const uint8_t *data,
                                     const uint16_t data_length) {
  // Wait until the busy pin is low and ready for commands.
  while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)) {
    ;
    ;
  }

  uint8_t buf[command_length + data_length];
  for (int i = 0; i < command_length; i++) {
    buf[i] = command[i];
  }
  for (int i = 0; i < data_length; i++) {
    buf[i] = data[i];
  }

  if (HAL_SPI_Transmit(&hspi1, buf, command_length + data_length, 100) ==
      HAL_OK) {
    return SX126X_HAL_STATUS_OK;
  } else {
    return SX126X_HAL_STATUS_ERROR;
  }
}

sx126x_hal_status_t sx126x_hal_read(const void *context, const uint8_t *command,
                                    const uint16_t command_length,
                                    uint8_t *data, const uint16_t data_length) {
  // Wait until the busy pin is low and ready for commands.
  while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)) {
    ;
    ;
  }

  if (HAL_SPI_Transmit(&hspi1, command, command_length, 100) == HAL_OK) {
    if (HAL_SPI_Receive(&hspi1, data, data_length, 100) == HAL_OK) {
      return SX126X_HAL_STATUS_OK;
    }
  }

  return SX126X_HAL_STATUS_ERROR;
}

sx126x_hal_status_t sx126x_hal_reset(const void *context) {
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  usleep(100);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

  // Wait until the busy pin is low and ready for commands.
  while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)) {
    ;
    ;
  }

  return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_wakeup(const void *context) {
  // Wait until the busy pin is low and ready for commands.
  while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)) {
    ;
    ;
  }

  // Send dummy data to sx1262 to pull NSS low which will wake it up.
  uint8_t *data = {0};
  HAL_SPI_Transmit(&hspi1, data, 1, 100);
  return SX126X_HAL_STATUS_OK;
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
