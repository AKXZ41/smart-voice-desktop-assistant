/* USER CODE BEGIN Header */
/**
 * @file voice.c
 * @brief 语音控制模块实现（占位，关键字解析）
 */
/* USER CODE END Header */

#include "voice.h"
#include "string.h"
#include <stdio.h>

static osMessageQueueId_t s_cmd_queue = NULL;
static UART_HandleTypeDef *s_voice_uart = NULL;
static uint8_t s_rx_byte = 0;
static char s_rx_line[64];
static uint16_t s_rx_len = 0;

void voice_init(void)
{
    if (s_cmd_queue == NULL) {
        const osMessageQueueAttr_t attr = { .name = "voice_cmd" };
        s_cmd_queue = osMessageQueueNew(8, sizeof(VoiceCmd), &attr);
    }
}

bool voice_get_cmd(VoiceCmd *out_cmd, uint32_t timeout_ms)
{
    if (s_cmd_queue == NULL || out_cmd == NULL) return false;
    return osMessageQueueGet(s_cmd_queue, out_cmd, NULL, timeout_ms) == osOK;
}

void voice_feed_text(const char *text)
{
    if (text == NULL || s_cmd_queue == NULL) return;
    VoiceCmd cmd = VoiceCmd_None;
    
    // 解析语音指令
    if (strstr(text, "请介绍你自己") || strstr(text, "介绍自己")) {
        cmd = VoiceCmd_Introduce;
    } else if (strstr(text, "你好小艾") || strstr(text, "你好")) {
        cmd = VoiceCmd_Hello;
    } else if (strstr(text, "小艾小艾") || strstr(text, "小爱小爱")) {
        cmd = VoiceCmd_WakeUp;
    } else if (strstr(text, "天气如何") || strstr(text, "天气怎么样") || strstr(text, "天气")) {
        cmd = VoiceCmd_Weather;
    } else if (strstr(text, "现在几点了") || strstr(text, "几点了") || strstr(text, "时间")) {
        cmd = VoiceCmd_Time;
    } else if (strstr(text, "更换图片") || strstr(text, "换图片") || strstr(text, "换背景")) {
        cmd = VoiceCmd_ChangeImage;
    } else if (strstr(text, "刷新") || strstr(text, "refresh")) {
        cmd = VoiceCmd_Refresh;
    }
    
    if (cmd != VoiceCmd_None) {
        (void)osMessageQueuePut(s_cmd_queue, &cmd, 0, 0);
    }
}

void voice_bind_uart(UART_HandleTypeDef *uart)
{
    s_voice_uart = uart;
    if (s_voice_uart) {
        (void)HAL_UART_Receive_IT(s_voice_uart, &s_rx_byte, 1);
    }
}

void voice_on_uart_rx_cplt(void)
{
    if (s_voice_uart == NULL) return;
    char ch = (char)s_rx_byte;
    if (ch == '\n') {
        s_rx_line[s_rx_len] = '\0';
        voice_feed_text(s_rx_line);
        s_rx_len = 0;
    } else if (ch != '\r') {
        if (s_rx_len < sizeof(s_rx_line) - 1) {
            s_rx_line[s_rx_len++] = ch;
        } else {
            s_rx_len = 0;
        }
    }
    (void)HAL_UART_Receive_IT(s_voice_uart, &s_rx_byte, 1);
}

void voice_send_reply(const char *reply_text)
{
    if (s_voice_uart == NULL || reply_text == NULL) return;
    
    // 发送回复文本到VC02模块
    // 格式: "TTS:回复文本\r\n"
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "TTS:%s\r\n", reply_text);
    HAL_UART_Transmit(s_voice_uart, (uint8_t*)cmd, strlen(cmd), 1000);
}



