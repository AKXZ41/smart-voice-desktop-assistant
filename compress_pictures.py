#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
简化的RLE压缩工具
"""

import re

def simple_rle_compress(data):
    """简化的RLE压缩"""
    if not data:
        return []
    
    compressed = []
    i = 0
    
    while i < len(data):
        # 查找重复数据
        count = 1
        value = data[i]
        
        # 计算重复次数
        while i + count < len(data) and data[i + count] == value and count < 255:
            count += 1
        
        if count > 2:  # 只有重复3次以上才压缩
            # 重复数据: [0xFF, count, value]
            compressed.append(0xFF)
            compressed.append(count)
            compressed.append(value)
        else:
            # 非重复数据: 直接添加
            for j in range(count):
                compressed.append(data[i + j])
        
        i += count
    
    return compressed

def parse_c_file(filename):
    """解析C文件"""
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 提取数组数据
        pattern = r'0x[0-9A-Fa-f]+'
        matches = re.findall(pattern, content)
        
        # 转换为整数列表
        data = []
        for match in matches:
            data.append(int(match, 16))
        
        return data
    except Exception as e:
        print(f"解析文件 {filename} 失败: {e}")
        return []

def generate_compressed_c(compressed_data, original_filename, output_filename):
    """生成压缩后的C文件"""
    try:
        with open(original_filename, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 提取图片尺寸信息
        width_match = re.search(r'(\d+)x(\d+)', content)
        if width_match:
            width = width_match.group(1)
            height = width_match.group(2)
        else:
            width = "128"
            height = "64"
        
        # 生成压缩后的C文件
        with open(output_filename, 'w', encoding='utf-8') as f:
            f.write(f"/* RLE压缩图片数据 - {width}x{height} */\n")
            f.write(f"/* 原始大小: {len(compressed_data) * 2} 字节 */\n")
            f.write(f"/* 压缩后大小: {len(compressed_data)} 字节 */\n")
            f.write(f"/* 压缩率: {(1 - len(compressed_data) / (len(compressed_data) * 2)) * 100:.1f}% */\n\n")
            
            f.write("const uint8_t picture_compressed[] = {\n")
            
            # 每行16个字节
            for i in range(0, len(compressed_data), 16):
                hex_values = []
                for j in range(16):
                    if i + j < len(compressed_data):
                        hex_values.append(f"0x{compressed_data[i + j]:02X}")
                    else:
                        break
                f.write("    " + ", ".join(hex_values) + ",\n")
            
            f.write("};\n\n")
            f.write(f"const uint16_t picture_width = {width};\n")
            f.write(f"const uint16_t picture_height = {height};\n")
            f.write(f"const uint16_t picture_compressed_size = {len(compressed_data)};\n")
        
        print(f"✓ 生成压缩文件: {output_filename}")
        return True
        
    except Exception as e:
        print(f"生成文件 {output_filename} 失败: {e}")
        return False

def main():
    """主函数"""
    print("开始压缩图片...")
    
    # 处理picture1.c
    print("\n处理 picture1.c...")
    data1 = parse_c_file('Core/Src/picture1.c')
    if data1:
        print(f"原始数据大小: {len(data1)} 字节")
        compressed1 = simple_rle_compress(data1)
        print(f"压缩后大小: {len(compressed1)} 字节")
        print(f"压缩率: {(1 - len(compressed1) / len(data1)) * 100:.1f}%")
        
        if generate_compressed_c(compressed1, 'Core/Src/picture1.c', 'Core/Src/picture1_compressed.c'):
            print("✓ picture1 压缩完成")
        else:
            print("✗ picture1 压缩失败")
    else:
        print("✗ 无法读取 picture1.c")
    
    # 处理picture2.c
    print("\n处理 picture2.c...")
    data2 = parse_c_file('Core/Src/picture2.c')
    if data2:
        print(f"原始数据大小: {len(data2)} 字节")
        compressed2 = simple_rle_compress(data2)
        print(f"压缩后大小: {len(compressed2)} 字节")
        print(f"压缩率: {(1 - len(compressed2) / len(data2)) * 100:.1f}%")
        
        if generate_compressed_c(compressed2, 'Core/Src/picture2.c', 'Core/Src/picture2_compressed.c'):
            print("✓ picture2 压缩完成")
        else:
            print("✗ picture2 压缩失败")
    else:
        print("✗ 无法读取 picture2.c")
    
    print("\n压缩完成！")

if __name__ == '__main__':
    main()
