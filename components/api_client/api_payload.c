#include "api_payload.h"
#include "cJSON.h"
#include "esp_log.h"

static const char *TAG = "FORMATTER_JSON";

char* build_batch_payload(const telemetry_reading_t *readings, size_t count)
{
    if (!readings || count == 0 || count > TELEMETRY_BATCH_CAPACITY){
        ESP_LOGE(TAG, "Leituras invalidas para montagem do lote JSON");
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (!root)
    {
        ESP_LOGE(TAG, "Falha ao alocar JSON root");
        return NULL;
    }

    cJSON *batch = cJSON_CreateArray();
    if (!batch || !cJSON_AddItemToObject(root, "readings", batch))
    {
        cJSON_Delete(batch);
        cJSON_Delete(root);
        ESP_LOGE(TAG, "Falha ao alocar array de leituras");
        return NULL;
    }

    for (size_t index = 0; index < count; index++)
    {
        cJSON *reading = cJSON_CreateObject();
        cJSON *vibration = cJSON_CreateObject();
        if (!reading || !vibration)
        {
            cJSON_Delete(reading);
            cJSON_Delete(vibration);
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Falha ao montar leitura %u do lote", (unsigned)index);
            return NULL;
        }

        if (!cJSON_AddNumberToObject(reading, "temperature", readings[index].temperature_celsius) ||
            !cJSON_AddNumberToObject(vibration, "x", readings[index].accel_x) ||
            !cJSON_AddNumberToObject(vibration, "y", readings[index].accel_y) ||
            !cJSON_AddNumberToObject(vibration, "z", readings[index].accel_z))
        {
            cJSON_Delete(reading);
            cJSON_Delete(vibration);
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Falha ao adicionar campos da leitura %u", (unsigned)index);
            return NULL;
        }

        if (!cJSON_AddItemToObject(reading, "vibration", vibration))
        {
            cJSON_Delete(reading);
            cJSON_Delete(vibration);
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Falha ao adicionar vibração da leitura %u", (unsigned)index);
            return NULL;
        }

        if (!cJSON_AddItemToArray(batch, reading))
        {
            cJSON_Delete(reading);
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Falha ao adicionar leitura %u ao lote", (unsigned)index);
            return NULL;
        }
    }

    char *json_out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json_out;
}
