/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    debug_uart.h
  * @brief   Debug UART interface for CH340N USB-to-Serial communication
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#define DEBUG_UART_BUFFER_SIZE 256

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Initialize debug UART (USART3) for CH340N communication
 */
void debug_uart_init(void);

/**
 * @brief Send string via debug UART
 * @param str String to send
 */
void debug_uart_send_string(const char *str);

/**
 * @brief Send formatted string via debug UART
 * @param format Format string (printf style)
 * @param ... Variable arguments
 */
void debug_uart_printf(const char *format, ...);

/**
 * @brief Send data buffer via debug UART
 * @param data Pointer to data buffer
 * @param size Number of bytes to send
 */
void debug_uart_send_data(const uint8_t *data, uint16_t size);

/**
 * @brief Check if data is available for reading
 * @return 1 if data available, 0 otherwise
 */
uint8_t debug_uart_data_available(void);

/**
 * @brief Read one byte from debug UART
 * @return Received byte, 0 if no data available
 */
uint8_t debug_uart_read_byte(void);

/**
 * @brief Read string from debug UART (blocking)
 * @param buffer Buffer to store received string
 * @param max_size Maximum buffer size
 * @return Number of characters read
 */
uint16_t debug_uart_read_string(char *buffer, uint16_t max_size);

/**
 * @brief Flush debug UART receive buffer
 */
void debug_uart_flush(void);

/**
 * @brief Get debug UART handle
 * @return Pointer to UART handle
 */
UART_HandleTypeDef* debug_uart_get_handle(void);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_UART_H */
