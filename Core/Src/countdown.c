/* USER CODE BEGIN Header */
/**
 * @file countdown.c
 * @brief 倒计时功能模块实现
 */
/* USER CODE END Header */

#include "countdown.h"
#include "voice.h"
#include "gpio.h"
#include <stdio.h>

// 倒计时相关变量
static CountdownState s_countdown_state = CountdownState_Stopped;
static uint32_t s_countdown_seconds = 0;
static uint32_t s_countdown_start_time = 0;
static CountdownCallback_t s_countdown_callback = NULL;

// 任务句柄
static osThreadId_t s_countdown_task_handle = NULL;

// 倒计时任务
void countdown_task(void *argument)
{
    const uint32_t task_delay = 1000; // 1秒检查一次
    uint32_t last_led_toggle = 0;
    bool led_state = false;
    
    for(;;) {
        if (s_countdown_state == CountdownState_Running) {
            uint32_t current_time = HAL_GetTick() / 1000; // 转换为秒
            uint32_t elapsed_seconds = current_time - s_countdown_start_time;
            
            if (elapsed_seconds >= s_countdown_seconds) {
                // 倒计时结束
                s_countdown_state = CountdownState_Finished;
                
                // 调用回调函数
                if (s_countdown_callback != NULL) {
                    s_countdown_callback();
                }
                
                // 发送语音提示
                voice_send_reply("倒计时结束！");
                
                // LED快速闪烁表示结束
                for(int i = 0; i < 10; i++) {
                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
                    osDelay(100);
                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
                    osDelay(100);
                }
            } else {
                // 倒计时进行中，LED慢速闪烁
                if (current_time - last_led_toggle >= 1) { // 每秒切换一次
                    led_state = !led_state;
                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, led_state ? GPIO_PIN_RESET : GPIO_PIN_SET);
                    last_led_toggle = current_time;
                }
            }
        } else {
            // 倒计时停止，LED熄灭
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        }
        
        osDelay(task_delay);
    }
}

void countdown_init(void)
{
    // 创建倒计时任务
    if (s_countdown_task_handle == NULL) {
        const osThreadAttr_t countdown_task_attributes = {
            .name = "countdown_task",
            .stack_size = 256 * 4,
            .priority = (osPriority_t) osPriorityNormal,
        };
        s_countdown_task_handle = osThreadNew(countdown_task, NULL, &countdown_task_attributes);
    }
}

bool countdown_start(uint32_t minutes, CountdownCallback_t callback)
{
    if (s_countdown_state == CountdownState_Running) {
        return false; // 已经在运行中
    }
    
    s_countdown_seconds = minutes * 60;
    s_countdown_start_time = HAL_GetTick() / 1000;
    s_countdown_callback = callback;
    s_countdown_state = CountdownState_Running;
    
    // 发送语音确认
    char reply[64];
    snprintf(reply, sizeof(reply), "倒计时%d分钟开始", (int)minutes);
    voice_send_reply(reply);
    
    return true;
}

void countdown_stop(void)
{
    s_countdown_state = CountdownState_Stopped;
    s_countdown_seconds = 0;
    s_countdown_callback = NULL;
    
    voice_send_reply("倒计时已停止");
}

CountdownState countdown_get_state(void)
{
    return s_countdown_state;
}

uint32_t countdown_get_remaining_seconds(void)
{
    if (s_countdown_state != CountdownState_Running) {
        return 0;
    }
    
    uint32_t current_time = HAL_GetTick() / 1000;
    uint32_t elapsed_seconds = current_time - s_countdown_start_time;
    
    if (elapsed_seconds >= s_countdown_seconds) {
        return 0;
    }
    
    return s_countdown_seconds - elapsed_seconds;
}

uint32_t countdown_get_remaining_minutes(void)
{
    return countdown_get_remaining_seconds() / 60;
}
