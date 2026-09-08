#include "wifi/wifi_helper.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include <string.h>
#include "config_manager.h"

static const char *TAG = "WIFI_MODULE";
static int s_retry_num = 0;

#define WIFI_SSID_AP "device"
#define WIFI_PASS_AP "12345678"
#define MAXIMUM_RETRY 3

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        wifi_event_sta_disconnected_t *disconected = (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGE(TAG, "Falha na conexao STA. Motivo: %d", disconected->reason);

        if (s_retry_num < MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGW(TAG, "Tentativa de conexao STA %d de %d", s_retry_num, MAXIMUM_RETRY);
        }
        else
        {
            ESP_LOGE(TAG, "Falha permanente no STA. Mantendo apenas SoftAP ativo.");
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "IP Obtido na rede STA: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    }
}

void wifi_init_loop(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));
}

void wifi_init_apsta(const char *ssid, const char *password)
{
    s_retry_num = 0;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    wifi_config_t ap_config = {
        .ap = {
            .ssid = WIFI_SSID_AP,
            .password = WIFI_PASS_AP,
            .ssid_len = strlen(WIFI_SSID_AP),
            .channel = 1,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    wifi_config_t sta_config = {
        .sta = {
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {.capable = true, .required = false},
        },
    };
    
    if (ssid) strncpy((char *)sta_config.sta.ssid, ssid, sizeof(sta_config.sta.ssid) - 1);
    if (password) strncpy((char *)sta_config.sta.password, password, sizeof(sta_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    if (ssid && strlen(ssid) > 0)
    {
        ESP_LOGI(TAG, "Iniciando conexao STA para a rede: %s", ssid);
        esp_wifi_connect();
    }
}

void wifi_init_ap(void)
{
    wifi_init_apsta(NULL, NULL);
}
