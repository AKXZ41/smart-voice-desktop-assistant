/* USER CODE BEGIN Header */
/**
 * @file esp8266.c
 * @brief ESP8266 HAL 驱动实现（USART + FreeRTOS 非阻塞）
 */
/* USER CODE END Header */

#include "esp8266.h"
#include "string.h"
#include <stdio.h>

#ifndef HAVE_STRNLEN
static size_t local_strnlen(const char *s, size_t maxlen)
{
    size_t n = 0;
    while (n < maxlen && s && s[n] != '\0') n++;
    return n;
}
#define strnlen local_strnlen
#endif

#define ESP8266_TX_QUEUE_LEN 8
#define ESP8266_RX_QUEUE_LEN 8
#define ESP8266_MAX_LINE_LEN 128

typedef struct {
    char data[ESP8266_MAX_LINE_LEN];
} Esp8266Line;

static void esp8266_start_rx_it(Esp8266Handle *handle);

static volatile uint8_t s_rx_byte;
static char s_line_buf[ESP8266_MAX_LINE_LEN];
static uint16_t s_line_len = 0;

void esp8266_init(Esp8266Handle *handle, UART_HandleTypeDef *uart)
{
    if (handle == NULL) return;
    handle->uart = uart;
    const osMessageQueueAttr_t tx_attr = { .name = "esp8266_tx" };
    const osMessageQueueAttr_t rx_attr = { .name = "esp8266_rx" };
    handle->tx_queue = osMessageQueueNew(ESP8266_TX_QUEUE_LEN, sizeof(Esp8266Line), &tx_attr);
    handle->rx_queue = osMessageQueueNew(ESP8266_RX_QUEUE_LEN, sizeof(Esp8266Line), &rx_attr);
    esp8266_start_rx_it(handle);
}

void esp8266_deinit(Esp8266Handle *handle)
{
    if (handle == NULL) return;
    if (handle->tx_queue) osMessageQueueDelete(handle->tx_queue);
    if (handle->rx_queue) osMessageQueueDelete(handle->rx_queue);
    handle->tx_queue = NULL;
    handle->rx_queue = NULL;
}

bool esp8266_send_at_async(Esp8266Handle *handle, const char *cmd)
{
    if (handle == NULL || cmd == NULL) return false;
    Esp8266Line line = {0};
    size_t len = strnlen(cmd, ESP8266_MAX_LINE_LEN - 3);
    memcpy(line.data, cmd, len);
    line.data[len++] = '\r';
    line.data[len++] = '\n';
    return osMessageQueuePut(handle->tx_queue, &line, 0, 0) == osOK;
}

bool esp8266_read_line(Esp8266Handle *handle, char *buf, uint16_t buf_size, uint32_t timeout_ms)
{
    if (handle == NULL || buf == NULL || buf_size == 0) return false;
    Esp8266Line line = {0};
    uint32_t timeout = timeout_ms;
    if (osMessageQueueGet(handle->rx_queue, &line, NULL, timeout) == osOK) {
        strncpy(buf, line.data, buf_size - 1);
        buf[buf_size - 1] = '\0';
        return true;
    }
    return false;
}

void esp8266_on_uart_rx_byte(Esp8266Handle *handle, uint8_t byte)
{
    if (handle == NULL) return;
    if (byte == '\n') {
        Esp8266Line line = {0};
        uint16_t copy_len = (s_line_len < (ESP8266_MAX_LINE_LEN - 1)) ? s_line_len : (ESP8266_MAX_LINE_LEN - 1);
        memcpy(line.data, s_line_buf, copy_len);
        line.data[copy_len] = '\0';
        (void)osMessageQueuePut(handle->rx_queue, &line, 0, 0);
        s_line_len = 0;
    } else if (s_line_len < ESP8266_MAX_LINE_LEN - 1) {
        if (byte != '\r') {
            s_line_buf[s_line_len++] = (char)byte;
        }
    } else {
        s_line_len = 0;
    }
}

static void esp8266_start_rx_it(Esp8266Handle *handle)
{
    if (handle == NULL || handle->uart == NULL) return;
    (void)HAL_UART_Receive_IT(handle->uart, (uint8_t *)&s_rx_byte, 1);
}

bool esp8266_task_tx_poll(Esp8266Handle *handle, uint32_t wait_ms)
{
    if (handle == NULL) return false;
    Esp8266Line line = {0};
    if (osMessageQueueGet(handle->tx_queue, &line, NULL, wait_ms) == osOK) {
        (void)HAL_UART_Transmit(handle->uart, (uint8_t *)line.data, (uint16_t)strnlen(line.data, ESP8266_MAX_LINE_LEN), HAL_MAX_DELAY);
        return true;
    }
    return false;
}

void esp8266_on_uart_rx_cplt(Esp8266Handle *handle)
{
    if (handle == NULL || handle->uart == NULL) return;
    esp8266_on_uart_rx_byte(handle, s_rx_byte);
    (void)HAL_UART_Receive_IT(handle->uart, (uint8_t *)&s_rx_byte, 1);
}



