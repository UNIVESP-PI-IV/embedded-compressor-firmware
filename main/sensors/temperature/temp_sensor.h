#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <stddef.h>
#include "esp_err.h"

typedef struct {
    float temperature_celsius;
} temp_sensor_data_t;

esp_err_t temp_sensor_init(void);

esp_err_t temp_sensor_read(temp_sensor_data_t *data);

char* temp_sensor_build_json(const temp_sensor_data_t *data);

#endif 
