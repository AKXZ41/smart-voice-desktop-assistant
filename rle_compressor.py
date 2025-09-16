#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
RLE (Run-Length Encoding) 压缩工具
用于压缩图片字模数据，减少Flash空间占用
"""

import re
import os

def rle_compress(data):
    """
    RLE压缩函数
    格式: [count, value] 或 [0x80|count, value1, value2, ...]
    count: 重复次数 (1-127)
    0x80|count: 非重复数据长度 (1-127)
    """
    if not data:
        return []
    
    compressed = []
    i = 0
    
    while i < len(data):
        # 查找重复数据
        count = 1
        value = data[i]
        
        # 计算重复次数 (最多127次)
        while i + count < len(data) and data[i + count] == value and count < 127:
            count += 1
        
        if count > 1:
            # 重复数据: [count, value]
            compressed.append(count)
            compressed.append(value)
        else:
            # 非重复数据: 查找连续的非重复数据
            non_repeat = [value]
            j = i + 1
            
            while j < len(data) and len(non_repeat) < 127:
                # 检查下一个数据是否与当前数据重复
                if j + 1 < len(data) and data[j] == data[j + 1]:
                    break
                non_repeat.append(data[j])
                j += 1
            
            # 添加非重复数据: [0x80|length, data...]
            compressed.append(0x80 | len(non_repeat))
            compressed.extend(non_repeat)
            count = len(non_repeat)
        
        i += count
    
    return compressed

def rle_decompress(compressed):
    """
    RLE解压缩函数
    """
    if not compressed:
        return []
    
    decompressed = []
    i = 0
    
    while i < len(compressed):
        count = compressed[i]
        
        if count & 0x80:
            # 非重复数据: [0x80|length, data...]
            length = count & 0x7F
            i += 1
            for j in range(length):
                if i + j < len(compressed):
                    decompressed.append(compressed[i + j])
            i += length
        else:
            # 重复数据: [count, value]
            if i + 1 < len(compressed):
                value = compressed[i + 1]
                for j in range(count):
                    decompressed.append(value)
            i += 2
    
    return decompressed

def parse_c_file(filename):
    """
    解析C文件中的数组数据
    """
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

def generate_c_file(compressed_data, original_filename, output_filename):
    """
    生成压缩后的C文件
    """
    with open(original_filename, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 替换数组数据
    lines = content.split('\n')
    new_lines = []
    in_array = False
    
    for line in lines:
        if '0x' in line and not in_array:
            in_array = True
            # 开始数组数据
            new_lines.append(line.split('0x')[0] + '// RLE压缩数据开始')
            new_lines.append('// 原始大小: {} 字节'.format(len(compressed_data) * 2))
            new_lines.append('// 压缩后大小: {} 字节'.format(len(compressed_data)))
            new_lines.append('// 压缩率: {:.1f}%'.format((1 - len(compressed_data) / (len(compressed_data) * 2)) * 100))
            new_lines.append('')
            
            # 添加压缩数据
            for i in range(0, len(compressed_data), 16):
                hex_values = []
                for j in range(16):
                    if i + j < len(compressed_data):
                        hex_values.append('0x{:02X}'.format(compressed_data[i + j]))
                    else:
                        break
                new_lines.append(','.join(hex_values) + ',')
        elif in_array and '0x' in line:
            # 跳过原始数组数据
            continue
        elif in_array and '0x' not in line and line.strip():
            # 数组结束
            in_array = False
            new_lines.append('// RLE压缩数据结束')
            new_lines.append('')
            new_lines.append(line)
        else:
            new_lines.append(line)
    
    # 写入新文件
    with open(output_filename, 'w', encoding='utf-8') as f:
        f.write('\n'.join(new_lines))

def main():
    """
    主函数
    """
    # 处理picture1.c
    print("处理 picture1.c...")
    data1 = parse_c_file('Core/Src/picture1.c')
    print(f"原始数据大小: {len(data1)} 字节")
    
    compressed1 = rle_compress(data1)
    print(f"压缩后大小: {len(compressed1)} 字节")
    print(f"压缩率: {(1 - len(compressed1) / len(data1)) * 100:.1f}%")
    
    # 验证压缩正确性
    decompressed1 = rle_decompress(compressed1)
    if decompressed1 == data1:
        print("✓ 压缩验证成功")
    else:
        print("✗ 压缩验证失败")
        return
    
    generate_c_file(compressed1, 'Core/Src/picture1.c', 'Core/Src/picture1_compressed.c')
    
    # 处理picture2.c
    print("\n处理 picture2.c...")
    data2 = parse_c_file('Core/Src/picture2.c')
    print(f"原始数据大小: {len(data2)} 字节")
    
    compressed2 = rle_compress(data2)
    print(f"压缩后大小: {len(compressed2)} 字节")
    print(f"压缩率: {(1 - len(compressed2) / len(data2)) * 100:.1f}%")
    
    # 验证压缩正确性
    decompressed2 = rle_decompress(compressed2)
    if decompressed2 == data2:
        print("✓ 压缩验证成功")
    else:
        print("✗ 压缩验证失败")
        return
    
    generate_c_file(compressed2, 'Core/Src/picture2.c', 'Core/Src/picture2_compressed.c')
    
    print("\n压缩完成！")
    print("生成文件:")
    print("- Core/Src/picture1_compressed.c")
    print("- Core/Src/picture2_compressed.c")

if __name__ == '__main__':
    main()
