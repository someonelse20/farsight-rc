#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>

typedef struct {
	float throttle;
	float set_x;
	float set_y;
	float set_z;
} controls_t;

typedef struct {
	double latitude;
	double longitude;
	uint16_t altidude;
	uint16_t heading;
	uint16_t speed;
} telemetry_t;

char* get_telemetry(void);

#endif // CONTROLLER_H
