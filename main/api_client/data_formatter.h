#ifndef DATA_FORMATTER_H
#define DATA_FORMATTER_H

#include "sensors/temperature/temp_sensor.h"
#include "sensors/vibration/vibration_sensor.h"

char* build_payload(const temp_sensor_data_t *temp, const vibration_sensor_data_t *vib);

#endif
