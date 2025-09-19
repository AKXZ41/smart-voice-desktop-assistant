/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    debug_uart.c
  * @brief   Debug UART implementation for CH340N USB-to-Serial communication
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

/* Includes ------------------------------------------------------------------*/
#include "debug_uart.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static uint8_t s_rx_buffer[DEBUG_UART_BUFFER_SIZE];
static uint16_t s_rx_head = 0;
static uint16_t s_rx_tail = 0;
static uint8_t s_uart_initialized = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize debug UART (USART3) for CH340N communication
 */
void debug_uart_init(void)
{
    /* USER CODE BEGIN debug_uart_init */
    if (s_uart_initialized) {
        return; // Already initialized
    }
    
    // Clear receive buffer
    s_rx_head = 0;
    s_rx_tail = 0;
    memset(s_rx_buffer, 0, DEBUG_UART_BUFFER_SIZE);
    
    // Start receiving data
    HAL_UART_Receive_IT(&huart3, &s_rx_buffer[s_rx_head], 1);
    
    s_uart_initialized = 1;
    /* USER CODE END debug_uart_init */
}

/**
 * @brief Send string via debug UART
 * @param str String to send
 */
void debug_uart_send_string(const char *str)
{
    /* USER CODE BEGIN debug_uart_send_string */
    if (!s_uart_initialized || str == NULL) {
        return;
    }
    
    uint16_t len = strlen(str);
    HAL_UART_Transmit(&huart3, (uint8_t*)str, len, 1000);
    /* USER CODE END debug_uart_send_string */
}

/**
 * @brief Send formatted string via debug UART
 * @param format Format string (printf style)
 * @param ... Variable arguments
 */
void debug_uart_printf(const char *format, ...)
{
    /* USER CODE BEGIN debug_uart_printf */
    if (!s_uart_initialized || format == NULL) {
        return;
    }
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (len > 0 && len < sizeof(buffer)) {
        HAL_UART_Transmit(&huart3, (uint8_t*)buffer, len, 1000);
    }
    /* USER CODE END debug_uart_printf */
}

/**
 * @brief Send data buffer via debug UART
 * @param data Pointer to data buffer
 * @param size Number of bytes to send
 */
void debug_uart_send_data(const uint8_t *data, uint16_t size)
{
    /* USER CODE BEGIN debug_uart_send_data */
    if (!s_uart_initialized || data == NULL || size == 0) {
        return;
    }
    
    HAL_UART_Transmit(&huart3, data, size, 1000);
    /* USER CODE END debug_uart_send_data */
}

/**
 * @brief Check if data is available for reading
 * @return 1 if data available, 0 otherwise
 */
uint8_t debug_uart_data_available(void)
{
    /* USER CODE BEGIN debug_uart_data_available */
    if (!s_uart_initialized) {
        return 0;
    }
    
    return (s_rx_head != s_rx_tail) ? 1 : 0;
    /* USER CODE END debug_uart_data_available */
}

/**
 * @brief Read one byte from debug UART
 * @return Received byte, 0 if no data available
 */
uint8_t debug_uart_read_byte(void)
{
    /* USER CODE BEGIN debug_uart_read_byte */
    if (!s_uart_initialized || !debug_uart_data_available()) {
        return 0;
    }
    
    uint8_t byte = s_rx_buffer[s_rx_tail];
    s_rx_tail = (s_rx_tail + 1) % DEBUG_UART_BUFFER_SIZE;
    return byte;
    /* USER CODE END debug_uart_read_byte */
}

/**
 * @brief Read string from debug UART (blocking)
 * @param buffer Buffer to store received string
 * @param max_size Maximum buffer size
 * @return Number of characters read
 */
uint16_t debug_uart_read_string(char *buffer, uint16_t max_size)
{
    /* USER CODE BEGIN debug_uart_read_string */
    if (!s_uart_initialized || buffer == NULL || max_size == 0) {
        return 0;
    }
    
    uint16_t count = 0;
    uint8_t byte;
    
    while (count < max_size - 1) {
        if (debug_uart_data_available()) {
            byte = debug_uart_read_byte();
            if (byte == '\n' || byte == '\r') {
                break;
            }
            buffer[count++] = byte;
        }
    }
    
    buffer[count] = '\0';
    return count;
    /* USER CODE END debug_uart_read_string */
}

/**
 * @brief Flush debug UART receive buffer
 */
void debug_uart_flush(void)
{
    /* USER CODE BEGIN debug_uart_flush */
    if (!s_uart_initialized) {
        return;
    }
    
    s_rx_head = 0;
    s_rx_tail = 0;
    memset(s_rx_buffer, 0, DEBUG_UART_BUFFER_SIZE);
    /* USER CODE END debug_uart_flush */
}

/**
 * @brief Get debug UART handle
 * @return Pointer to UART handle
 */
UART_HandleTypeDef* debug_uart_get_handle(void)
{
    /* USER CODE BEGIN debug_uart_get_handle */
    return &huart3;
    /* USER CODE END debug_uart_get_handle */
}

/* USER CODE BEGIN 1 */

/**
 * @brief UART receive complete callback for debug UART
 * @param huart UART handle
 */
void debug_uart_rx_cplt_callback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3) {
        // Move head pointer
        s_rx_head = (s_rx_head + 1) % DEBUG_UART_BUFFER_SIZE;
        
        // Check for buffer overflow
        if (s_rx_head == s_rx_tail) {
            s_rx_tail = (s_rx_tail + 1) % DEBUG_UART_BUFFER_SIZE;
        }
        
        // Continue receiving
        HAL_UART_Receive_IT(&huart3, &s_rx_buffer[s_rx_head], 1);
    }
}

/* USER CODE END 1 */
