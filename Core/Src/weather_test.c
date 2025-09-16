/* USER CODE BEGIN Header */
/**
 * @file weather_test.c
 * @brief 天气功能测试
 */
/* USER CODE END Header */

#include "weather.h"
#include "display_manager.h"
#include "voice.h"
#include "usart.h"
#include <stdio.h>

// 测试天气更新功能
void weather_update_test(void)
{
    Esp8266Handle *g_esp = usart_get_esp8266_handle();
    
    if (weather_is_connected()) {
        if (weather_fetch_now(g_esp, 5000)) {
            char desc[32], temp[8];
            weather_get_last(desc, sizeof(desc), temp, sizeof(temp));
            char weather_str[32];
            snprintf(weather_str, sizeof(weather_str), "%s %sC", desc, temp);
            
            // 更新显示
            display_manager_set_weather(weather_str);
            display_manager_force_refresh(DisplayRefresh_Weather);
            
            // 语音反馈
            voice_send_reply("天气测试更新成功");
        } else {
            voice_send_reply("天气测试更新失败");
        }
    } else {
        voice_send_reply("网络未连接，无法测试天气");
    }
}

// 测试网络时间同步功能
void weather_time_sync_test(void)
{
    Esp8266Handle *g_esp = usart_get_esp8266_handle();
    
    if (weather_is_connected()) {
        if (weather_sync_time_from_network(g_esp)) {
            voice_send_reply("网络时间同步成功");
        } else {
            voice_send_reply("网络时间同步失败");
        }
    } else {
        voice_send_reply("网络未连接，无法同步时间");
    }
}
