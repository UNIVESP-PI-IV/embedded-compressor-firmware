#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "wifi_config.h"
#include "api_client.h"
#include "api_payload.h"
#include "temp_sensor.h"
#include "vibration_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

static const char *TAG = "MAIN_APP";

#define TELEMETRY_URL CONFIG_TELEMETRY_SERVER_URL
#define TELEMETRY_INTERVAL CONFIG_TELEMETRY_SEND_INTERVAL_MS
#define TELEMETRY_BATCH_INTERVAL CONFIG_TELEMETRY_BATCH_INTERVAL_MS

static telemetry_reading_t readings[TELEMETRY_BATCH_CAPACITY];
static size_t reading_count = 0;

static void compressor_telemetry_task(void *pvParameters)
{
    temp_sensor_data_t temp_data;
    vibration_sensor_data_t vibration_data;
    int64_t batch_started_us = 0;

    temp_sensor_init();
    vibration_sensor_init();

    while (1)
    {
        int64_t now_us = esp_timer_get_time();

        if (reading_count > 0 &&
            (reading_count >= TELEMETRY_BATCH_CAPACITY ||
             now_us - batch_started_us >= (int64_t)TELEMETRY_BATCH_INTERVAL * 1000))
        {
            if (wifi_is_connected())
            {
                char *json_payload = build_batch_payload(readings, reading_count);
                if (json_payload != NULL)
                {
                    ESP_LOGI(TAG, "Enviando lote com %u leituras", (unsigned)reading_count);
                    esp_err_t err = http_post_json(TELEMETRY_URL, json_payload);
                    free(json_payload);

                    if (err == ESP_OK)
                    {
                        ESP_LOGI(TAG, "Lote de telemetria enviado com sucesso!");
                        reading_count = 0;
                        batch_started_us = 0;
                    }
                    else
                    {
                        // Mantém o lote para nova tentativa; quando cheio, as
                        // próximas leituras são pausadas para não sobrescrevê-lo.
                        ESP_LOGE(TAG, "Falha no envio; lote mantido para nova tentativa");
                    }
                }
            }
            else
            {
                ESP_LOGW(TAG, "Lote pronto; aguardando Wi-Fi para enviar");
            }
        }

        // Não descarta um lote ainda não confirmado. Se o buffer estiver cheio,
        // o próximo ciclo tenta enviar antes de coletar novas leituras.
        if (reading_count < TELEMETRY_BATCH_CAPACITY)
        {
            esp_err_t err_temp = temp_sensor_read(&temp_data);
            esp_err_t err_vib = vibration_sensor_read(&vibration_data);

            if (err_temp == ESP_OK && err_vib == ESP_OK)
            {
                if (reading_count == 0)
                {
                    batch_started_us = esp_timer_get_time();
                }

                readings[reading_count++] = (telemetry_reading_t){
                    .temperature_celsius = temp_data.temperature_celsius,
                    .accel_x = vibration_data.accel_x,
                    .accel_y = vibration_data.accel_y,
                    .accel_z = vibration_data.accel_z,
                };
            }
            else
            {
                ESP_LOGW(TAG, "Falha ao ler um ou mais sensores. Leitura ignorada.");
            }
        }
        else if (!wifi_is_connected())
        {
            ESP_LOGW(TAG, "Buffer cheio; aguardando Wi-Fi para preservar o lote");
        }

        vTaskDelay(pdMS_TO_TICKS(TELEMETRY_INTERVAL));
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_loop();

    char wifi_ssid[32] = {0};
    char wifi_pass[64] = {0};

    if (config_load_credentials(wifi_ssid, sizeof(wifi_ssid), wifi_pass, sizeof(wifi_pass)) == ESP_OK)
    {
        ESP_LOGI(TAG, "Credenciais encontradas na NVS. SSID: %s", wifi_ssid);
        wifi_init_apsta(wifi_ssid, wifi_pass);
    }
    else
    {
        ESP_LOGW(TAG, "Nenhuma credencial na NVS. Modo SoftAP ativo.");
        wifi_init_ap();
    }

    config_mdns_init();
    config_start_web_server();

    xTaskCreate(
        compressor_telemetry_task,
        "telemetry_task",
        4096,
        NULL,
        5,
        NULL);
}
