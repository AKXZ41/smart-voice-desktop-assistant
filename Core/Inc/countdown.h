/* USER CODE BEGIN Header */
/**
 * @file countdown.h
 * @brief 倒计时功能模块接口
 */
/* USER CODE END Header */

#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include "main.h"
#include "cmsis_os.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 倒计时状态
typedef enum {
    CountdownState_Stopped = 0,
    CountdownState_Running,
    CountdownState_Finished
} CountdownState;

// 倒计时回调函数类型
typedef void (*CountdownCallback_t)(void);

/**
 * @brief 初始化倒计时模块
 */
void countdown_init(void);

/**
 * @brief 启动倒计时
 * @param minutes 倒计时分钟数
 * @param callback 倒计时结束时的回调函数
 * @return true 启动成功，false 启动失败
 */
bool countdown_start(uint32_t minutes, CountdownCallback_t callback);

/**
 * @brief 停止倒计时
 */
void countdown_stop(void);

/**
 * @brief 获取倒计时状态
 * @return 倒计时状态
 */
CountdownState countdown_get_state(void);

/**
 * @brief 获取剩余时间（秒）
 * @return 剩余秒数
 */
uint32_t countdown_get_remaining_seconds(void);

/**
 * @brief 获取剩余时间（分钟）
 * @return 剩余分钟数
 */
uint32_t countdown_get_remaining_minutes(void);

/**
 * @brief 倒计时任务（需要在FreeRTOS中调用）
 * @param argument 任务参数
 */
void countdown_task(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* COUNTDOWN_H */
