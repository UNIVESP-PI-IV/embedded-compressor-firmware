#include "data_formatter.h"
#include "cJSON.h"
#include "esp_log.h"

static const char *TAG = "FORMATTER_JSON";

char* build_payload(const temp_sensor_data_t *temp, const vibration_sensor_data_t *vib)
{
    if (!temp || !vib){
        ESP_LOGE(TAG, "Ponteiros dos sensores invalidos para montagem do JSON");
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (!root)
    {
        ESP_LOGE(TAG, "Falha ao alocar JSON root");
        return NULL;
    }

    cJSON_AddNumberToObject(root, "temperature", temp->temperature_celsius);

    cJSON *vibration = cJSON_CreateObject();
    if (vibration)
    {
        cJSON_AddNumberToObject(vibration, "x", vib->accel_x);
        cJSON_AddNumberToObject(vibration, "y", vib->accel_y);
        cJSON_AddNumberToObject(vibration, "z", vib->accel_z);
        cJSON_AddItemToObject(root, "vibration", vibration);
    }

    char *json_out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json_out;
}
