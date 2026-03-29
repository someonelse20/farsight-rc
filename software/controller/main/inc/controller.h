#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL       /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA       /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0                   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          CONFIG_I2C_MASTER_FREQUENCY /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define SSD1306_ADDR                0x3D                        /*!< Address of the oled display */

typedef struct {
	float throttle;
	float set_x;
	float set_y;
	float set_z;
} controls_t;

typedef struct {
	double latitude;
	double longitude;
	uint16_t altitude;
	uint16_t heading;
	uint16_t speed;
} telemetry_t;

char* get_telemetry(void);

#endif // CONTROLLER_H
