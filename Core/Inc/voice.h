/* USER CODE BEGIN Header */
/**
 * @file voice.h
 * @brief VC02语音控制模块接口
 */
/* USER CODE END Header */

#ifndef VOICE_H
#define VOICE_H

#include "main.h"
#include "cmsis_os.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VoiceCmd_None = 0,
    VoiceCmd_ShowTime,
    VoiceCmd_ShowWeather,
    VoiceCmd_Refresh,
    VoiceCmd_Introduce,      // 请介绍你自己
    VoiceCmd_Hello,          // 你好小艾
    VoiceCmd_WakeUp,         // 小艾小艾
    VoiceCmd_Weather,        // 天气如何
    VoiceCmd_Time,           // 现在几点了
    VoiceCmd_ChangeImage,    // 更换图片
    VoiceCmd_Countdown5Min,  // 倒计时5分钟
    VoiceCmd_Countdown10Min, // 倒计时10分钟
    VoiceCmd_CountdownStatus, // 倒计时状态查询
    VoiceCmd_CountdownStop,  // 停止倒计时
    VoiceCmd_UpdateWeather,  // 更新天气
} VoiceCmd;

/** \brief 初始化VC02语音模块 */
void voice_init(void);

/** \brief 获取一条解析出的语音指令（带超时） */
bool voice_get_cmd(VoiceCmd *out_cmd, uint32_t timeout_ms);

/** \brief 内部：解析原始文本/关键字 */
void voice_feed_text(const char *text);

/** \brief 绑定 UART 句柄并启动非阻塞接收（用于 VC02 模块） */
void voice_bind_uart(UART_HandleTypeDef *uart);

/** \brief UART 接收完成中断回调适配（在 HAL_UART_RxCpltCallback 调用） */
void voice_on_uart_rx_cplt(void);

/** \brief 发送语音回复到VC02模块 */
void voice_send_reply(const char *reply_text);

#ifdef __cplusplus
}
#endif

#endif /* VOICE_H */



