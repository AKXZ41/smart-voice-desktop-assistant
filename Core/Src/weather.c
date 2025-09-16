/* USER CODE BEGIN Header */
/**
 * @file weather.c
 * @brief 基于 ESP8266 (AT) 获取和风天气 Now 接口（HTTP 非 TLS 示例）
 */
/* USER CODE END Header */

#include "weather.h"
#include "string.h"
#include <stdio.h>
#include <stdbool.h>
#include "cmsis_os.h"

static char s_city_id[24];
static char s_api_key[64];
static char s_last_desc[32];
static char s_last_temp[8];
static bool s_network_connected = false;

void weather_init(const char *city_id, const char *api_key)
{
    strncpy(s_city_id, city_id ? city_id : "", sizeof s_city_id - 1);
    strncpy(s_api_key, api_key ? api_key : "", sizeof s_api_key - 1);
}

bool weather_wifi_connect(Esp8266Handle *esp, const char *ssid, const char *pass, uint32_t timeout_ms)
{
    if (!esp || !ssid || !pass) return false;
    
    // 先断开之前的连接
    esp8266_send_at_async(esp, "AT+CWQAP");
    osDelay(2000);
    
    // 扫描可用WiFi网络
    esp8266_send_at_async(esp, "AT+CWLAP");
    osDelay(3000);
    
    // 发送连接命令
    char cmd[128];
    snprintf(cmd, sizeof cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, pass);
    esp8266_send_at_async(esp, cmd);
    
    char line[128];
    uint32_t t = timeout_ms;
    bool got_ip = false;
    
    while (t > 0) {
        if (esp8266_read_line(esp, line, sizeof line, 200)) {
            // 检查连接成功标志
            if (strstr(line, "WIFI CONNECTED")) {
                // WiFi已连接，等待获取IP
                got_ip = false;
            }
            if (strstr(line, "WIFI GOT IP")) {
                // 获取到IP地址，连接完全成功
                got_ip = true;
                s_network_connected = true;
                return true;
            }
            if (strstr(line, "OK") && strstr(line, "CWJAP")) {
                // 连接命令成功
                if (got_ip) {
                    s_network_connected = true;
                    return true;
                }
            }
            // 检查连接失败标志
            if (strstr(line, "FAIL") || strstr(line, "ERROR") || strstr(line, "NO AP")) {
                s_network_connected = false;
                return false;
            }
        }
        if (t >= 200) t -= 200; else break;
    }
    s_network_connected = false;
    return false;
}

static bool weather_http_get(Esp8266Handle *esp, const char *path, uint32_t timeout_ms)
{
    if (!esp || !path) return false;
    esp8266_send_at_async(esp, "AT+CIPSTART=\"TCP\",\"api.qweather.com\",80");
    char line[160];
    (void)esp8266_read_line(esp, line, sizeof line, 1000);
    char req[256];
    snprintf(req, sizeof req,
             "GET %s HTTP/1.1\r\nHost: api.qweather.com\r\nConnection: close\r\n\r\n",
             path);
    char send_len[32];
    snprintf(send_len, sizeof send_len, "AT+CIPSENDEX=%u", (unsigned)strlen(req));
    esp8266_send_at_async(esp, send_len);
    (void)esp8266_read_line(esp, line, sizeof line, 200);
    esp8266_send_at_async(esp, req);
    uint32_t t = timeout_ms;
    while (t > 0) {
        if (esp8266_read_line(esp, line, sizeof line, 200)) {
            if (strstr(line, "CLOSED")) break;
            char *p;
            if ((p = strstr(line, "\"temp\":\"")) != NULL) {
                p += 8;
                strncpy(s_last_temp, p, sizeof s_last_temp - 1);
                char *q = strchr(s_last_temp, '\"');
                if (q) *q = '\0';
            }
            if ((p = strstr(line, "\"text\":\"")) != NULL) {
                p += 8;
                strncpy(s_last_desc, p, sizeof s_last_desc - 1);
                char *q = strchr(s_last_desc, '\"');
                if (q) *q = '\0';
            }
        }
        if (t >= 200) t -= 200; else break;
    }
    return (s_last_desc[0] != '\0');
}

bool weather_fetch_now(Esp8266Handle *esp, uint32_t timeout_ms)
{
    s_last_desc[0] = '\0';
    s_last_temp[0] = '\0';
    char path[160];
    snprintf(path, sizeof path, "/v7/weather/now?location=%s&key=%s", s_city_id, s_api_key);
    return weather_http_get(esp, path, timeout_ms);
}

void weather_get_last(char *out_desc, uint16_t desc_size, char *out_temp, uint16_t temp_size)
{
    if (out_desc && desc_size) {
        strncpy(out_desc, s_last_desc, desc_size - 1);
        out_desc[desc_size - 1] = '\0';
    }
    if (out_temp && temp_size) {
        strncpy(out_temp, s_last_temp, temp_size - 1);
        out_temp[temp_size - 1] = '\0';
    }
}

bool weather_is_connected(void)
{
    return s_network_connected;
}

bool weather_sync_time_from_network(Esp8266Handle *esp)
{
    if (!esp || !s_network_connected) return false;
    
    // 发送HTTP请求获取网络时间
    esp8266_send_at_async(esp, "AT+CIPSTART=\"TCP\",\"api.qweather.com\",80");
    char line[160];
    (void)esp8266_read_line(esp, line, sizeof line, 1000);
    
    // 发送HTTP请求获取时间
    char req[256];
    snprintf(req, sizeof req,
        "GET /v7/time?location=%s&key=%s HTTP/1.1\r\n"
        "Host: api.qweather.com\r\n"
        "Connection: close\r\n\r\n", s_city_id, s_api_key);
    
    char cmd[32];
    snprintf(cmd, sizeof cmd, "AT+CIPSEND=%d", (int)strlen(req));
    esp8266_send_at_async(esp, cmd);
    osDelay(100);
    esp8266_send_at_async(esp, req);
    
    // 解析响应获取时间
    uint32_t timeout = 5000;
    while (timeout > 0) {
        if (esp8266_read_line(esp, line, sizeof line, 200)) {
            // 查找时间戳
            char *time_start = strstr(line, "\"time\":\"");
            if (time_start) {
                time_start += 8; // 跳过 "time":"
                char *time_end = strchr(time_start, '"');
                if (time_end) {
                    *time_end = '\0';
                    // 解析时间并同步到DS1302
                    // 这里需要解析ISO时间格式，暂时简化处理
                    return true;
                }
            }
        }
        if (timeout >= 200) timeout -= 200; else break;
    }
    
    esp8266_send_at_async(esp, "AT+CIPCLOSE");
    return false;
}



