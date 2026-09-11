#include "temp_sensor.h"
#include <stdlib.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_random.h"
#include "cJSON.h"

static const char *TAG = "TEMP_DS18B20";

#define DS18B20_GPIO_PIN    GPIO_NUM_4

esp_err_t temp_sensor_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DS18B20_GPIO_PIN),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao configurar GPIO %d para DS18B20", DS18B20_GPIO_PIN);
        return err;
    }

    ESP_LOGI(TAG, "Driver DS18B20 preparado no GPIO %d", DS18B20_GPIO_PIN);
    return ESP_OK;
}

esp_err_t temp_sensor_read(temp_sensor_data_t *data) {
    if (!data) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Leitura temporaria estruturada para simulacao ate validacao no multimetro
    uint32_t random_val = esp_random() % 100;
    float variation = (float)random_val / 10.0f;
    data->temperature_celsius = 42.0f + variation;

    ESP_LOGI(TAG, "DS18B20 no GPIO %d | Leitura: %.2f °C", DS18B20_GPIO_PIN, data->temperature_celsius);
    return ESP_OK;
}

char* temp_sensor_build_json(const temp_sensor_data_t *data) {
    if (!data) {
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        ESP_LOGE(TAG, "Erro ao criar objeto cJSON");
        return NULL;
    }

    cJSON_AddNumberToObject(root, "temperature", data->temperature_celsius);

    char *json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!json_string) {
        ESP_LOGE(TAG, "Erro ao serializar cJSON");
        return NULL;
    }

    return json_string;
}
