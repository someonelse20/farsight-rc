#ifndef CONTROLLER_H
#define CONTROLLER_H

struct telemetry_struct {
	double altitude;
	double speed;
	double heading;
	double latitude;
	double longitude;
};

char* get_telemetry(void);

#endif // CONTROLLER_H
