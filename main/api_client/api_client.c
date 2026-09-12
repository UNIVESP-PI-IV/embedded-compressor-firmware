#include "api_client/api_client.h"
#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"

static const char *TAG = "HTTP_CLIENT";

esp_err_t http_post_json(const char *url, const char *json_payload) {
    if (!url || !json_payload) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Falha ao inicializar o esp_http_client");
        return ESP_FAIL;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_payload, strlen(json_payload));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        
        char response_buffer[128];
        esp_http_client_read_response(client, response_buffer, sizeof(response_buffer) - 1);

        if (status_code >= 200 && status_code < 300) {
            ESP_LOGI(TAG, "Sucesso HTTP [%d]", status_code);
        } else {
            ESP_LOGW(TAG, "HTTP POST retornou status code: %d", status_code);
            err = ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "Falha na requisição HTTP: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}
