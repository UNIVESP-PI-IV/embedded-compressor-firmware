#include "config/config_manager.h"
#include "esp_err.h"
#include <stddef.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include "mdns.h"

static const char *TAG = "CONFIG_MGR";
#define NVS_NAMESPACE "wifi_store"

const char* html_page = 
    "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Configuração ESP32</title>"
    "<style>body{font-family:Arial; margin:20px; max-width:400px;} input{margin-bottom:15px; width:100%; padding:10px; box-sizing:border-box;}</style>"
    "</head><body>"
    "<h2>Configurar Wi-Fi</h2>"
    "<form method='POST' action='/salvar'>"
    "SSID:<br><input type='text' name='ssid' required><br>"
    "Senha:<br><input type='password' name='password' required><br>"
    "<input type='submit' value='Salvar e Conectar' style='background:#007BFF; color:white; border:none; padding:12px; cursor:pointer;'>"
    "</form></body></html>";

void config_mdns_init(void) {
    esp_err_t err = mdns_init();
    if (err == ESP_ERR_INVALID_STATE) {
        return;
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(mdns_hostname_set("device"));
    ESP_ERROR_CHECK(mdns_instance_name_set("Monitoramento de Compressor"));
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);

    ESP_LOGI(TAG, "mDNS ativo: http://device.local");
}

esp_err_t config_load_credentials(char *ssid, size_t ssid_len, char *password, size_t password_len) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) return err;

    err = nvs_get_str(my_handle, "wifi_ssid", ssid, &ssid_len);
    if (err == ESP_OK) {
        err = nvs_get_str(my_handle, "wifi_pass", password, &password_len);
    }

    nvs_close(my_handle);
    return err;
}

esp_err_t config_save_credentials(const char *ssid, const char *password) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    err = nvs_set_str(my_handle, "wifi_ssid", ssid);
    if (err == ESP_OK) {
        err = nvs_set_str(my_handle, "wifi_pass", password);
    }
    if (err == ESP_OK) {
        err = nvs_commit(my_handle);
    }

    nvs_close(my_handle);
    return err;
}

static esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t save_post_handler(httpd_req_t *req) {
    char content[128] = {0};
    int ret = httpd_req_recv(req, content, MIN(req->content_len, sizeof(content) - 1));
    
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Corpo invalido");
        return ESP_FAIL;
    }

    char ssid[32] = {0};
    char password[64] = {0};

    if (httpd_query_key_value(content, "ssid", ssid, sizeof(ssid)) == ESP_OK &&
        httpd_query_key_value(content, "password", password, sizeof(password)) == ESP_OK) 
    {
        ESP_LOGI(TAG, "Novas credenciais salvas para SSID: %s", ssid);
        config_save_credentials(ssid, password);

        httpd_resp_sendstr(req, "<h1>Salvo! Reiniciando...</h1>");
        vTaskDelay(pdMS_TO_TICKS(1500));
        esp_restart();
    } else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Parametros ausentes");
    }

    return ESP_OK;
}

void config_start_web_server(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true; 

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root_uri = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler };
        httpd_uri_t save_uri = { .uri = "/salvar", .method = HTTP_POST, .handler = save_post_handler };
        
        httpd_register_uri_handler(server, &root_uri);
        httpd_register_uri_handler(server, &save_uri);
        ESP_LOGI(TAG, "Servidor HTTP iniciado na porta 80");
    }
}
