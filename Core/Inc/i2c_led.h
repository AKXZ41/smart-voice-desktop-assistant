/* USER CODE BEGIN Header */
/**
 * @file i2c_led.h
 * @brief I2C LED 显示模块接口（适配 I2C1）
 */
/* USER CODE END Header */

#ifndef I2C_LED_H
#define I2C_LED_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \brief 初始化 SSD1306 OLED 显示器 */
void i2c_led_init(I2C_HandleTypeDef *i2c);

/** \brief 显示一行文本（自动截断，ASCII，占位渲染） */
void i2c_led_show_text(const char *text);

/** \brief 显示时间与天气的简易接口 */
void i2c_led_show_time_weather(const char *time_str, const char *weather);

/** \brief 刷新背景图片 */
void i2c_led_refresh_background(void);

#ifdef __cplusplus
}
#endif

#endif /* I2C_LED_H */



