#ifndef API_CLIENT_H
#define API_CLIENT_H

#include "esp_err.h"
#include <stddef.h>

/**
 * @brief Envia um payload JSON para o endpoint especificado via HTTP POST.
 * 
 * @param url Endpoint do backend.
 * @param json_payload String contendo o JSON já formatado.
 * @return esp_err_t ESP_OK em caso de sucesso (status 2xx).
 */
esp_err_t http_post_json(const char *url, const char *json_payload);

#endif