/* USER CODE BEGIN Header */
/**
 * @file weather.h
 * @brief 基于 ESP8266 的和风天气获取与解析
 */
/* USER CODE END Header */

#ifndef WEATHER_H
#define WEATHER_H

#include "main.h"
#include "esp8266.h"

#ifdef __cplusplus
extern "C" {
#endif

void weather_init(const char *city_id, const char *api_key);
bool weather_wifi_connect(Esp8266Handle *esp, const char *ssid, const char *pass, uint32_t timeout_ms);
bool weather_fetch_now(Esp8266Handle *esp, uint32_t timeout_ms);
void weather_get_last(char *out_desc, uint16_t desc_size, char *out_temp, uint16_t temp_size);
bool weather_is_connected(void);
bool weather_sync_time_from_network(Esp8266Handle *esp);

#ifdef __cplusplus
}
#endif

#endif /* WEATHER_H */



