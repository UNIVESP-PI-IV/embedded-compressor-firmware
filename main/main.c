#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "config_manager.h"
#include "wifi_helper.h"
#include "sdkconfig.h"
#include "api_client/api_client.h"
#include "api_client/data_formatter.h"
#include "sensors/temperature/temp_sensor.h"
#include "sensors/vibration/vibration_sensor.h"

static const char *TAG = "MAIN_APP";

#define TELEMETRY_URL CONFIG_TELEMETRY_SERVER_URL
#define TELEMETRY_INTERVAL CONFIG_TELEMETRY_SEND_INTERVAL_MS

static void compressor_telemetry_task(void *pvParameters)
{
    temp_sensor_data_t temp_data;
    vibration_sensor_data_t vibration_data;

    temp_sensor_init();
    vibration_sensor_init();

    while (1)
    {
        if (wifi_is_connected())
        {
            esp_err_t err_temp = temp_sensor_read(&temp_data);
            esp_err_t err_vib = vibration_sensor_read(&vibration_data);

            if (err_temp == ESP_OK && err_vib == ESP_OK){

                char *json_payload = build_payload(&temp_data, &vibration_data);

                if (json_payload != NULL)
                {
                    ESP_LOGI(TAG, "Enviando telemetria: %s", json_payload);

                    esp_err_t err = http_post_json(TELEMETRY_URL, json_payload);
                    if (err == ESP_OK) {
                        ESP_LOGI(TAG, "Telemetria enviada com sucesso!");
                    } else {
                        ESP_LOGE(TAG, "Falha ao enviar telemetria via HTTP");
                    }

                    free(json_payload);
                }
            }
            else
            {
                ESP_LOGW(TAG, "Falha ao ler um ou mais sensores. Envio ignorado.");
            }
        }
        else
        {
            ESP_LOGW(TAG, "Aguardando conexão Wi-Fi (IP STA)...");
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
