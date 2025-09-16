/* USER CODE BEGIN Header */
/**
 * @file i2c_led.c
 * @brief I2C LED 显示模块实现（占位示例）
 */
/* USER CODE END Header */

#include "i2c_led.h"
#include "string.h"
#include <stdio.h>

#define I2C_LED_ADDR (0x3C << 1) /* SSD1306 常见 I2C 地址 */

/* SSD1306 基础指令 */
#define SSD1306_CMD 0x00
#define SSD1306_DATA 0x40
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

static I2C_HandleTypeDef *s_i2c = NULL;

static void ssd1306_write_cmd(uint8_t cmd)
{
    if (s_i2c == NULL) return;
    uint8_t buf[2] = {SSD1306_CMD, cmd};
    (void)HAL_I2C_Master_Transmit(s_i2c, I2C_LED_ADDR, buf, 2, 100);
}

static void ssd1306_write_data(const uint8_t *data, uint16_t len)
{
    if (s_i2c == NULL) return;
    /* 预留 1 字节控制字 */
    uint8_t tmp[16];
    while (len) {
        uint16_t chunk = (len > (sizeof(tmp) - 1)) ? (sizeof(tmp) - 1) : len;
        tmp[0] = SSD1306_DATA;
        memcpy(&tmp[1], data, chunk);
        (void)HAL_I2C_Master_Transmit(s_i2c, I2C_LED_ADDR, tmp, (uint16_t)(chunk + 1), 200);
        data += chunk;
        len -= chunk;
    }
}

void i2c_led_init(I2C_HandleTypeDef *i2c)
{
    s_i2c = i2c;
    /* 初始化序列（简化）：基本电源/时钟/显示开 */
    ssd1306_write_cmd(0xAE); // display off
    ssd1306_write_cmd(0x20); // memory addressing mode
    ssd1306_write_cmd(0x00); // horizontal addressing
    ssd1306_write_cmd(0x40); // set display start line
    ssd1306_write_cmd(0xB0); // set page start address
    ssd1306_write_cmd(0xC8); // COM scan direction remapped
    ssd1306_write_cmd(0x00); // low column address
    ssd1306_write_cmd(0x10); // high column address
    ssd1306_write_cmd(0x81); // contrast
    ssd1306_write_cmd(0x7F);
    ssd1306_write_cmd(0xA1); // segment remap
    ssd1306_write_cmd(0xA6); // normal display
    ssd1306_write_cmd(0xA8); // multiplex ratio
    ssd1306_write_cmd(0x3F);
    ssd1306_write_cmd(0xA4); // display follows RAM
    ssd1306_write_cmd(0xD3); // display offset
    ssd1306_write_cmd(0x00);
    ssd1306_write_cmd(0xD5); // display clock divide
    ssd1306_write_cmd(0x80);
    ssd1306_write_cmd(0xD9); // pre-charge
    ssd1306_write_cmd(0xF1);
    ssd1306_write_cmd(0xDA); // COM pins
    ssd1306_write_cmd(0x12);
    ssd1306_write_cmd(0xDB); // VCOMH deselect level
    ssd1306_write_cmd(0x40);
    ssd1306_write_cmd(0x8D); // charge pump
    ssd1306_write_cmd(0x14);
    ssd1306_write_cmd(0xAF); // display on
}

void i2c_led_show_text(const char *text)
{
    if (text == NULL) return;
    /* 简化：将 ASCII 映射为 6x8 字模（此处仅写空白，作为占位显示清屏效果） */
    for (uint8_t page = 0; page < 8; ++page) {
        ssd1306_write_cmd(0xB0 + page);
        ssd1306_write_cmd(0x00);
        ssd1306_write_cmd(0x10);
        uint8_t line[SSD1306_WIDTH];
        memset(line, 0x00, sizeof line);
        ssd1306_write_data(line, sizeof line);
    }
}

void i2c_led_show_time_weather(const char *time_str, const char *weather)
{
    char line[64] = {0};
    (void)snprintf(line, sizeof(line), "%s %s", time_str ? time_str : "--:--", weather ? weather : "N/A");
    i2c_led_show_text(line);
}

void i2c_led_refresh_background(void)
{
    // 占位实现：刷新背景图片
    // TODO: 实现背景图片切换逻辑
    // 这里可以切换不同的背景图片或显示效果
}



