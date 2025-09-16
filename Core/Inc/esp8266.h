/* USER CODE BEGIN Header */
/**
 * @file esp8266.h
 * @brief ESP8266 HAL 驱动接口（USART + FreeRTOS 非阻塞）
 */
/* USER CODE END Header */

#ifndef ESP8266_H
#define ESP8266_H

#include "main.h"
#include "cmsis_os.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    UART_HandleTypeDef *uart;
    osMessageQueueId_t tx_queue;
    osMessageQueueId_t rx_queue;
} Esp8266Handle;

void esp8266_init(Esp8266Handle *handle, UART_HandleTypeDef *uart);
void esp8266_deinit(Esp8266Handle *handle);
bool esp8266_send_at_async(Esp8266Handle *handle, const char *cmd);
bool esp8266_read_line(Esp8266Handle *handle, char *buf, uint16_t buf_size, uint32_t timeout_ms);
void esp8266_on_uart_rx_byte(Esp8266Handle *handle, uint8_t byte);
void esp8266_on_uart_rx_cplt(Esp8266Handle *handle);
bool esp8266_task_tx_poll(Esp8266Handle *handle, uint32_t wait_ms);

#ifdef __cplusplus
}
#endif

#endif /* ESP8266_H */



