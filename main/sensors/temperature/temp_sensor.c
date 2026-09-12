#include "sensors/temperature/temp_sensor.h"
#include <stdlib.h>
#include "esp_log.h"
#include "cJSON.h"
#include "ds18b20.h"
#include "onewire_bus.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "TEMP_DS18B20";

#define DS18B20_GPIO_PIN GPIO_NUM_4

static ds18b20_device_handle_t ds18b20_handle = NULL;

esp_err_t temp_sensor_init(void) {

    onewire_bus_handle_t bus = NULL;
    onewire_bus_config_t bus_config = {
        .bus_gpio_num = DS18B20_GPIO_PIN,
    };
    onewire_bus_rmt_config_t rmt_config = {
        .max_rx_bytes = 10,
    };

    esp_err_t err = onewire_new_bus_rmt(&bus_config, &rmt_config, &bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao inicializar barramento One-Wire no GPIO %d: %s", DS18B20_GPIO_PIN, esp_err_to_name(err));
        return err;
    }

    onewire_device_iter_handle_t iter = NULL;
    onewire_device_t next_onewire_dev;
    onewire_new_device_iter(bus, &iter);

    if (onewire_device_iter_get_next(iter, &next_onewire_dev) == ESP_OK) {
        ds18b20_config_t ds_cfg = {};
        err = ds18b20_new_device(&next_onewire_dev, &ds_cfg, &ds18b20_handle);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Sensor DS18B20 detectado e inicializado com sucesso no GPIO %d!", DS18B20_GPIO_PIN);
        } else {
            ESP_LOGE(TAG, "Falha ao criar handle para o DS18B20: %s", esp_err_to_name(err));
        }
    } else {
        ESP_LOGE(TAG, "Nenhum dispositivo One-Wire encontrado no GPIO %d. Verifique as conexões e o resistor!", DS18B20_GPIO_PIN);
        err = ESP_FAIL;
    }

    onewire_del_device_iter(iter);
    return err;
}

esp_err_t temp_sensor_read(temp_sensor_data_t *data) {
    if (!data) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!ds18b20_handle) {
        ESP_LOGE(TAG, "Sensor DS18B20 nao foi inicializado corretamente");
        return ESP_FAIL;
    }

    esp_err_t err = ds18b20_trigger_temperature_conversion(ds18b20_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao disparar conversao de temperatura: %s", esp_err_to_name(err));
        return err;
    }

    vTaskDelay(pdMS_TO_TICKS(800));

    float temp_celsius = 0.0f;
    err = ds18b20_get_temperature(ds18b20_handle, &temp_celsius);
    if (err == ESP_OK) {
        data->temperature_celsius = temp_celsius;
        ESP_LOGI(TAG, "DS18B20 no GPIO %d | Leitura Real: %.2f °C", DS18B20_GPIO_PIN, temp_celsius);
    } else {
        ESP_LOGE(TAG, "Falha ao ler dados de temperatura do DS18B20: %s", esp_err_to_name(err));
    }

    return err;
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
