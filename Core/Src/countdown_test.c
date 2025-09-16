/* USER CODE BEGIN Header */
/**
 * @file countdown_test.c
 * @brief 倒计时功能测试
 */
/* USER CODE END Header */

#include "countdown.h"
#include "voice.h"
#include <stdio.h>

// 倒计时结束回调函数
static void countdown_finished_callback(void)
{
    // 可以在这里添加倒计时结束时的特殊处理
    // 比如播放特殊音效、发送通知等
    voice_send_reply("时间到了！该休息了！");
}

// 测试倒计时功能
void countdown_test(void)
{
    // 测试5分钟倒计时
    if (countdown_start(5, countdown_finished_callback)) {
        voice_send_reply("5分钟倒计时测试开始");
    } else {
        voice_send_reply("倒计时测试启动失败");
    }
}

// 查询倒计时状态
void countdown_status_test(void)
{
    CountdownState state = countdown_get_state();
    uint32_t remaining_seconds = countdown_get_remaining_seconds();
    uint32_t remaining_minutes = countdown_get_remaining_minutes();
    
    char status_msg[128];
    snprintf(status_msg, sizeof(status_msg), 
            "倒计时状态: %d, 剩余: %d分%d秒", 
            (int)state, (int)remaining_minutes, (int)(remaining_seconds % 60));
    
    voice_send_reply(status_msg);
}
