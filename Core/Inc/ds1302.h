/* USER CODE BEGIN Header */
/**
 * @file ds1302.h
 * @brief DS1302 实时时钟驱动接口（GPIO 位操作）
 */
/* USER CODE END Header */

#ifndef DS1302_H
#define DS1302_H

#include "main.h"
#include <stdbool.h>

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} RtcDateTime;

void ds1302_init(void);
bool ds1302_read(RtcDateTime *out_time);
bool ds1302_write(const RtcDateTime *in_time);

#endif /* DS1302_H */



