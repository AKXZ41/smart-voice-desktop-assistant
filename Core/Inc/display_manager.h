/* USER CODE BEGIN Header */
/**
 * @file display_manager.h
 * @brief 显示管理模块接口
 * 负责管理时间、天气、背景的刷新逻辑
 */
/* USER CODE END Header */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "main.h"
#include "cmsis_os.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 显示刷新类型
typedef enum {
    DisplayRefresh_Time = 0x01,      // 时间刷新 (1秒)
    DisplayRefresh_Weather = 0x02,   // 天气刷新 (1小时)
    DisplayRefresh_Background = 0x04 // 背景刷新 (1小时)
} DisplayRefreshType;

// 显示状态
typedef struct {
    char time_str[16];           // 时间字符串
    char weather_str[32];        // 天气字符串
    uint8_t current_background;  // 当前背景图片
    uint32_t last_weather_update; // 上次天气更新时间
    uint32_t last_background_update; // 上次背景更新时间
} DisplayState;

/**
 * @brief 初始化显示管理模块
 */
void display_manager_init(void);

/**
 * @brief 显示管理任务
 * @param argument 任务参数
 */
void display_manager_task(void *argument);

/**
 * @brief 强制刷新显示
 * @param refresh_type 刷新类型
 */
void display_manager_force_refresh(DisplayRefreshType refresh_type);

/**
 * @brief 获取当前显示状态
 * @return 显示状态指针
 */
const DisplayState* display_manager_get_state(void);

/**
 * @brief 设置时间显示
 * @param time_str 时间字符串
 */
void display_manager_set_time(const char *time_str);

/**
 * @brief 设置天气显示
 * @param weather_str 天气字符串
 */
void display_manager_set_weather(const char *weather_str);

/**
 * @brief 刷新背景图片
 */
void display_manager_refresh_background(void);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_MANAGER_H */
