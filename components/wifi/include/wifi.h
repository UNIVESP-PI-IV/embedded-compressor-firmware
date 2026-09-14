#ifndef WIFI_HELPER_H
#define WIFI_HELPER_H

void wifi_init_loop(void);
void wifi_init_apsta(const char *ssid, const char *password);
void wifi_init_ap(void);
bool wifi_is_connected(void);

#endif