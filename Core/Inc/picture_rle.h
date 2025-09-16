/* USER CODE BEGIN Header */
/**
 * @file picture_rle.h
 * @brief RLE压缩图片数据头文件
 */
/* USER CODE END Header */

#ifndef PICTURE_RLE_H
#define PICTURE_RLE_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

// 图片尺寸定义
#define PICTURE1_WIDTH  346
#define PICTURE1_HEIGHT 224
#define PICTURE2_WIDTH  639
#define PICTURE2_HEIGHT 471

// RLE压缩数据长度
#define PICTURE1_RLE_SIZE  8000  // 压缩后大小（估算）
#define PICTURE2_RLE_SIZE  15000 // 压缩后大小（估算）

// RLE解压缩函数
void picture_rle_decompress(const uint8_t* compressed_data, uint16_t compressed_size, 
                           uint8_t* output_data, uint16_t output_size);

// 获取图片数据
const uint8_t* picture1_get_data(void);
const uint8_t* picture2_get_data(void);

// 获取图片尺寸
uint16_t picture1_get_width(void);
uint16_t picture1_get_height(void);
uint16_t picture2_get_width(void);
uint16_t picture2_get_height(void);

// 获取压缩数据大小
uint16_t picture1_get_compressed_size(void);
uint16_t picture2_get_compressed_size(void);

#ifdef __cplusplus
}
#endif

#endif /* PICTURE_RLE_H */
