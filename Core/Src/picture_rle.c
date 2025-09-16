/* USER CODE BEGIN Header */
/**
 * @file picture_rle.c
 * @brief RLE压缩图片数据实现
 */
/* USER CODE END Header */

#include "picture_rle.h"
#include <string.h>

// RLE解压缩函数
void picture_rle_decompress(const uint8_t* compressed_data, uint16_t compressed_size, 
                           uint8_t* output_data, uint16_t output_size)
{
    if (!compressed_data || !output_data || compressed_size == 0 || output_size == 0) {
        return;
    }
    
    uint16_t input_pos = 0;
    uint16_t output_pos = 0;
    
    while (input_pos < compressed_size && output_pos < output_size) {
        uint8_t count = compressed_data[input_pos++];
        
        if (count & 0x80) {
            // 非重复数据: [0x80|length, data...]
            uint8_t length = count & 0x7F;
            for (uint8_t i = 0; i < length && input_pos < compressed_size && output_pos < output_size; i++) {
                output_data[output_pos++] = compressed_data[input_pos++];
            }
        } else {
            // 重复数据: [count, value]
            if (input_pos < compressed_size) {
                uint8_t value = compressed_data[input_pos++];
                for (uint8_t i = 0; i < count && output_pos < output_size; i++) {
                    output_data[output_pos++] = value;
                }
            }
        }
    }
}

// 包含压缩后的图片数据
extern const uint8_t picture1_compressed[];
extern const uint16_t picture1_compressed_size;
extern const uint8_t picture2_compressed[];
extern const uint16_t picture2_compressed_size;

const uint8_t* picture1_get_data(void)
{
    return picture1_compressed;
}

const uint8_t* picture2_get_data(void)
{
    return picture2_compressed;
}

uint16_t picture1_get_width(void)
{
    return PICTURE1_WIDTH;
}

uint16_t picture1_get_height(void)
{
    return PICTURE1_HEIGHT;
}

uint16_t picture2_get_width(void)
{
    return PICTURE2_WIDTH;
}

uint16_t picture2_get_height(void)
{
    return PICTURE2_HEIGHT;
}

uint16_t picture1_get_compressed_size(void)
{
    return picture1_compressed_size;
}

uint16_t picture2_get_compressed_size(void)
{
    return picture2_compressed_size;
}
