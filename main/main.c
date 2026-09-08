#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "config_manager.h"
#include "wifi_helper.h"

static const char *TAG = "MAIN_APP";

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
}
