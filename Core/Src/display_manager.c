/* USER CODE BEGIN Header */
/**
 * @file display_manager.c
 * @brief 显示管理模块实现
 * 负责管理时间、天气、背景的刷新逻辑
 */
/* USER CODE END Header */

#include "display_manager.h"
#include "i2c_led.h"
#include "ds1302.h"
#include "weather.h"
#include "usart.h"
#include "picture_rle.h"
#include <stdio.h>
#include <string.h>

// 显示状态
static DisplayState s_display_state = {0};
static osThreadId_t s_display_task_handle = NULL;
static osEventFlagsId_t s_display_event_handle = NULL;

// 刷新间隔定义 (毫秒)
#define TIME_REFRESH_INTERVAL     1000    // 1秒
#define WEATHER_REFRESH_INTERVAL  3600000 // 1小时
#define BACKGROUND_REFRESH_INTERVAL 3600000 // 1小时

// 显示管理任务
void display_manager_task(void *argument)
{
    uint32_t last_time_refresh = 0;
    uint32_t last_weather_refresh = 0;
    uint32_t last_background_refresh = 0;
    
    // 初始化显示状态
    strcpy(s_display_state.time_str, "--:--");
    strcpy(s_display_state.weather_str, "N/A");
    s_display_state.current_background = 0;
    s_display_state.last_weather_update = 0;
    s_display_state.last_background_update = 0;
    
    for(;;) {
        uint32_t current_time = HAL_GetTick();
        bool need_refresh = false;
        char display_line[64] = {0};
        
        // 1. 时间刷新 (每秒)
        if (current_time - last_time_refresh >= TIME_REFRESH_INTERVAL) {
            RtcDateTime now;
            if (ds1302_read(&now)) {
                snprintf(s_display_state.time_str, sizeof(s_display_state.time_str), 
                        "%02u:%02u", (unsigned)now.hours, (unsigned)now.minutes);
            } else {
                strcpy(s_display_state.time_str, "--:--");
            }
            last_time_refresh = current_time;
            need_refresh = true;
        }
        
        // 2. 天气刷新 (每小时)
        if (current_time - last_weather_refresh >= WEATHER_REFRESH_INTERVAL) {
            if (weather_is_connected()) {
                char desc[32], temp[8];
                weather_get_last(desc, sizeof(desc), temp, sizeof(temp));
                snprintf(s_display_state.weather_str, sizeof(s_display_state.weather_str), 
                        "%s %sC", desc, temp);
            } else {
                strcpy(s_display_state.weather_str, "No WiFi");
            }
            s_display_state.last_weather_update = current_time;
            last_weather_refresh = current_time;
            need_refresh = true;
        }
        
        // 3. 背景刷新 (每小时)
        if (current_time - last_background_refresh >= BACKGROUND_REFRESH_INTERVAL) {
            display_manager_refresh_background();
            last_background_refresh = current_time;
            need_refresh = true;
        }
        
        // 4. 检查强制刷新事件
        uint32_t events = osEventFlagsWait(s_display_event_handle, 
                                          DisplayRefresh_Time | DisplayRefresh_Weather | DisplayRefresh_Background,
                                          osFlagsWaitAny, 0);
        
        if (events & DisplayRefresh_Time) {
            // 强制刷新时间 - 立即处理，避免Flash堵塞
            RtcDateTime now;
            if (ds1302_read(&now)) {
                snprintf(s_display_state.time_str, sizeof(s_display_state.time_str), 
                        "%02u:%02u", (unsigned)now.hours, (unsigned)now.minutes);
            } else {
                strcpy(s_display_state.time_str, "--:--");
            }
            need_refresh = true;
            
            // 立即更新显示，释放Flash资源
            snprintf(display_line, sizeof(display_line), "%s %s", 
                    s_display_state.time_str, s_display_state.weather_str);
            i2c_led_show_text(display_line);
        }
        
        if (events & DisplayRefresh_Weather) {
            // 强制刷新天气 - 立即处理，避免Flash堵塞
            if (weather_is_connected()) {
                char desc[32], temp[8];
                weather_get_last(desc, sizeof(desc), temp, sizeof(temp));
                snprintf(s_display_state.weather_str, sizeof(s_display_state.weather_str), 
                        "%s %sC", desc, temp);
            } else {
                strcpy(s_display_state.weather_str, "No WiFi");
            }
            s_display_state.last_weather_update = current_time;
            need_refresh = true;
            
            // 立即更新显示，释放Flash资源
            snprintf(display_line, sizeof(display_line), "%s %s", 
                    s_display_state.time_str, s_display_state.weather_str);
            i2c_led_show_text(display_line);
        }
        
        if (events & DisplayRefresh_Background) {
            // 强制刷新背景
            display_manager_refresh_background();
            need_refresh = true;
        }
        
        // 5. 更新显示
        if (need_refresh) {
            // 组合显示内容
            snprintf(display_line, sizeof(display_line), "%s %s", 
                    s_display_state.time_str, s_display_state.weather_str);
            
            // 发送到OLED显示
            i2c_led_show_text(display_line);
        }
        
        // 任务延时，避免CPU占用过高
        osDelay(100);
    }
}

void display_manager_init(void)
{
    // 创建显示事件标志
    const osEventFlagsAttr_t event_attr = {
        .name = "display_event"
    };
    s_display_event_handle = osEventFlagsNew(&event_attr);
    
    // 创建显示管理任务
    if (s_display_task_handle == NULL) {
        const osThreadAttr_t task_attr = {
            .name = "display_manager",
            .stack_size = 512 * 4,
            .priority = (osPriority_t) osPriorityNormal,
        };
        s_display_task_handle = osThreadNew(display_manager_task, NULL, &task_attr);
    }
}

void display_manager_force_refresh(DisplayRefreshType refresh_type)
{
    if (s_display_event_handle != NULL) {
        osEventFlagsSet(s_display_event_handle, refresh_type);
    }
}

const DisplayState* display_manager_get_state(void)
{
    return &s_display_state;
}

void display_manager_set_time(const char *time_str)
{
    if (time_str != NULL) {
        strncpy(s_display_state.time_str, time_str, sizeof(s_display_state.time_str) - 1);
        s_display_state.time_str[sizeof(s_display_state.time_str) - 1] = '\0';
    }
}

void display_manager_set_weather(const char *weather_str)
{
    if (weather_str != NULL) {
        strncpy(s_display_state.weather_str, weather_str, sizeof(s_display_state.weather_str) - 1);
        s_display_state.weather_str[sizeof(s_display_state.weather_str) - 1] = '\0';
        s_display_state.last_weather_update = HAL_GetTick();
    }
}

void display_manager_refresh_background(void)
{
    // 使用栈上的临时缓冲区，避免长期占用Flash
    uint8_t decompressed_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
    
    // 切换背景图片
    s_display_state.current_background = (s_display_state.current_background + 1) % 2;
    
    // 解压缩并显示背景图片
    if (s_display_state.current_background == 0) {
        // 显示picture1
        picture_rle_decompress(picture1_get_data(), picture1_get_compressed_size(), 
                              decompressed_buffer, sizeof(decompressed_buffer));
    } else {
        // 显示picture2
        picture_rle_decompress(picture2_get_data(), picture2_get_compressed_size(), 
                              decompressed_buffer, sizeof(decompressed_buffer));
    }
    
    // 立即发送到OLED，然后释放缓冲区
    // TODO: 实现OLED显示逻辑
    // ssd1306_draw_bitmap(0, 0, decompressed_buffer, SSD1306_WIDTH, SSD1306_HEIGHT);
    
    // 更新背景刷新时间
    s_display_state.last_background_update = HAL_GetTick();
    
    // 函数结束时，decompressed_buffer自动释放，不占用Flash
}
