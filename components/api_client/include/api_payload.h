#ifndef DATA_FORMATTER_H
#define DATA_FORMATTER_H

#include "temp_sensor.h"
#include "vibration_sensor.h"
#include <stddef.h>

#define TELEMETRY_BATCH_CAPACITY 60

typedef struct
{
    float temperature_celsius;
    float accel_x;
    float accel_y;
    float accel_z;
} telemetry_reading_t;

char* build_batch_payload(const telemetry_reading_t *readings, size_t count);

#endif
