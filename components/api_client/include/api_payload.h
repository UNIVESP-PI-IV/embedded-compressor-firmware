#ifndef DATA_FORMATTER_H
#define DATA_FORMATTER_H

#include "temp_sensor.h"
#include "vibration_sensor.h"

char* build_payload(const temp_sensor_data_t *temp, const vibration_sensor_data_t *vib);

#endif
