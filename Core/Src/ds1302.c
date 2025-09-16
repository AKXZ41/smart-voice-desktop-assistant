/* USER CODE BEGIN Header */
/**
 * @file ds1302.c
 * @brief DS1302 实时时钟驱动实现（GPIO 位操作）
 */
/* USER CODE END Header */

#include "ds1302.h"
#include "gpio.h"

#define DS1302_CMD_SECOND   0x80
#define DS1302_CMD_MINUTE   0x82
#define DS1302_CMD_HOUR     0x84
#define DS1302_CMD_DATE     0x86
#define DS1302_CMD_MONTH    0x88
#define DS1302_CMD_DAY      0x8A
#define DS1302_CMD_YEAR     0x8C
#define DS1302_CMD_CONTROL  0x8E

#define DS1302_WRITE(addr)  ((uint8_t)((addr) & 0xFE))
#define DS1302_READ(addr)   ((uint8_t)((addr) | 0x01))

static void ds1302_delay(void)
{
    for (volatile int i = 0; i < 50; ++i) { __NOP(); }
}

static void ce_high(void) { HAL_GPIO_WritePin(DS1302_CE_GPIO_Port, DS1302_CE_Pin, GPIO_PIN_SET); }
static void ce_low(void)  { HAL_GPIO_WritePin(DS1302_CE_GPIO_Port, DS1302_CE_Pin, GPIO_PIN_RESET); }
static void clk_high(void) { HAL_GPIO_WritePin(DS1302_SCL_GPIO_Port, DS1302_SCL_Pin, GPIO_PIN_SET); }
static void clk_low(void)  { HAL_GPIO_WritePin(DS1302_SCL_GPIO_Port, DS1302_SCL_Pin, GPIO_PIN_RESET); }
static void io_high(void)  { HAL_GPIO_WritePin(DS1302_DATA_GPIO_Port, DS1302_DATA_Pin, GPIO_PIN_SET); }
static void io_low(void)   { HAL_GPIO_WritePin(DS1302_DATA_GPIO_Port, DS1302_DATA_Pin, GPIO_PIN_RESET); }

static void io_output(void)
{
    GPIO_InitTypeDef cfg = {0};
    cfg.Pin = DS1302_DATA_Pin;
    cfg.Mode = GPIO_MODE_OUTPUT_PP;
    cfg.Pull = GPIO_NOPULL;
    cfg.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DS1302_DATA_GPIO_Port, &cfg);
}

static void io_input(void)
{
    GPIO_InitTypeDef cfg = {0};
    cfg.Pin = DS1302_DATA_Pin;
    cfg.Mode = GPIO_MODE_INPUT;
    cfg.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DS1302_DATA_GPIO_Port, &cfg);
}

static uint8_t bcd_to_bin(uint8_t bcd) { return (uint8_t)((bcd >> 4) * 10 + (bcd & 0x0F)); }
static uint8_t bin_to_bcd(uint8_t bin) { return (uint8_t)(((bin / 10) << 4) | (bin % 10)); }

static void ds1302_write_byte(uint8_t byte)
{
    io_output();
    for (int i = 0; i < 8; ++i) {
        if (byte & 0x01) io_high(); else io_low();
        ds1302_delay();
        clk_high(); ds1302_delay(); clk_low();
        byte >>= 1; /* LSB first */
    }
}

static uint8_t ds1302_read_byte(void)
{
    uint8_t byte = 0;
    io_input();
    for (int i = 0; i < 8; ++i) {
        byte >>= 1;
        if (HAL_GPIO_ReadPin(DS1302_DATA_GPIO_Port, DS1302_DATA_Pin) == GPIO_PIN_SET) {
            byte |= 0x80;
        }
        ds1302_delay();
        clk_high(); ds1302_delay(); clk_low();
    }
    return byte;
}

static void ds1302_write_reg(uint8_t addr, uint8_t data)
{
    ce_high(); ds1302_delay();
    ds1302_write_byte(DS1302_WRITE(addr));
    ds1302_write_byte(data);
    ce_low();
}

static uint8_t ds1302_read_reg(uint8_t addr)
{
    uint8_t val;
    ce_high(); ds1302_delay();
    ds1302_write_byte(DS1302_READ(addr));
    val = ds1302_read_byte();
    ce_low();
    return val;
}

void ds1302_init(void)
{
    ce_low(); clk_low(); io_low();
    ds1302_write_reg(DS1302_CMD_CONTROL, 0x00);
}

bool ds1302_read(RtcDateTime *out_time)
{
    if (out_time == NULL) return false;
    uint8_t sec = ds1302_read_reg(DS1302_CMD_SECOND) & 0x7F;
    uint8_t min = ds1302_read_reg(DS1302_CMD_MINUTE);
    uint8_t hour = ds1302_read_reg(DS1302_CMD_HOUR);
    uint8_t date = ds1302_read_reg(DS1302_CMD_DATE);
    uint8_t mon = ds1302_read_reg(DS1302_CMD_MONTH);
    uint8_t year = ds1302_read_reg(DS1302_CMD_YEAR);
    out_time->seconds = bcd_to_bin(sec);
    out_time->minutes = bcd_to_bin(min);
    out_time->hours = bcd_to_bin(hour & 0x3F);
    out_time->day = bcd_to_bin(date);
    out_time->month = bcd_to_bin(mon);
    out_time->year = (uint16_t)(2000 + bcd_to_bin(year));
    return true;
}

bool ds1302_write(const RtcDateTime *in_time)
{
    if (in_time == NULL) return false;
    ds1302_write_reg(DS1302_CMD_CONTROL, 0x00);
    ds1302_write_reg(DS1302_CMD_SECOND, bin_to_bcd(in_time->seconds) & 0x7F);
    ds1302_write_reg(DS1302_CMD_MINUTE, bin_to_bcd(in_time->minutes));
    ds1302_write_reg(DS1302_CMD_HOUR, bin_to_bcd(in_time->hours));
    ds1302_write_reg(DS1302_CMD_DATE, bin_to_bcd(in_time->day));
    ds1302_write_reg(DS1302_CMD_MONTH, bin_to_bcd(in_time->month));
    ds1302_write_reg(DS1302_CMD_YEAR, bin_to_bcd((uint8_t)(in_time->year % 100)));
    return true;
}



