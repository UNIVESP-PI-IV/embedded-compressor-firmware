#ifndef VIBRATION_SENSOR_H
#define VIBRATION_SENSOR_H

#include "esp_err.h"

typedef struct {
    float accel_x;
    float accel_y;
    float accel_z;
} vibration_sensor_data_t;

esp_err_t vibration_sensor_init(void);
esp_err_t vibration_sensor_read(vibration_sensor_data_t *data);

#endif
