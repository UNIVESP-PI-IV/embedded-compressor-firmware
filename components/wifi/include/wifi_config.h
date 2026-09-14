#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stddef.h>
#include "esp_err.h"

esp_err_t config_load_credentials(char *ssid, size_t ssid_len, char *password, size_t password_len);
esp_err_t config_save_credentials(const char *ssid, const char *password);

void config_start_web_server(void);
void config_mdns_init(void);

#endif