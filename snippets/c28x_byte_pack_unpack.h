/**
 * @file    c28x_byte_pack_unpack.h
 * @brief   针对 C2000 (C28x 16-bit char) 的 8-bit 字节流打包与解包工具库
 * @details
 *   由于 C28x 没有原生 8-bit uint8_t 类型（最小寻址单元 uint16_t 为 16-bit），
 *   在处理 UART / CAN / SPI / Modbus 通信报文时，本头文件提供两套零开销内联工具：
 *     1. 【紧凑模式 (Packed Array)】：基于编译器 __byte() (MOVB 指令)，在 uint16_t 数组中每元素存 2 个字节；
 *     2. 【松散模式 (Unpacked Array)】：每个 uint16_t 元素仅低 8 位存 1 个字节（直接对接 SCI/CAN 驱动）。
 */

#ifndef C28X_BYTE_PACK_UNPACK_H
#define C28X_BYTE_PACK_UNPACK_H

#include <stdint.h>

// ============================================================================
// 一、紧凑字节数组访问 (1 个 uint16_t 存放 2 个 8-bit 字节，基于单周期 MOVB 指令)
// ============================================================================

/**
 * @brief 从紧凑 uint16_t 缓冲区中读取第 byteIdx 个 8-bit 字节 (返回 0x0000 ~ 0x00FF)
 */
static inline uint16_t C28x_PackedGetByte(const uint16_t *packedBuf, uint16_t byteIdx)
{
    return (uint16_t)__byte((int *)packedBuf, byteIdx) & 0x00FFU;
}

/**
 * @brief 向紧凑 uint16_t 缓冲区的第 byteIdx 个 8-bit 字节位置写入 1 个字节 (不破坏相邻字节)
 */
static inline void C28x_PackedSetByte(uint16_t *packedBuf, uint16_t byteIdx, uint16_t byteVal)
{
    __byte((int *)packedBuf, byteIdx) = (int)(byteVal & 0x00FFU);
}

// ============================================================================
// 二、松散字节数组 (uint16_t 每个元素低 8 位存 1 Byte) 的多字节拼接与拆解
//     (极适合直接处理 CAN_readMessage / SCI_readCharArray 读出的报文数组)
// ============================================================================

/**
 * @brief 将小端序 (Little-Endian) 的 2 个字节拼成 uint16_t
 */
static inline uint16_t C28x_UnpackU16_LE(const uint16_t *byteArr)
{
    return ((byteArr[0] & 0x00FFU) | ((byteArr[1] & 0x00FFU) << 8U));
}

/**
 * @brief 将大端序 (Big-Endian) 的 2 个字节拼成 uint16_t
 */
static inline uint16_t C28x_UnpackU16_BE(const uint16_t *byteArr)
{
    return (((byteArr[0] & 0x00FFU) << 8U) | (byteArr[1] & 0x00FFU));
}

/**
 * @brief 将小端序 (Little-Endian) 的 4 个字节拼成 uint32_t
 */
static inline uint32_t C28x_UnpackU32_LE(const uint16_t *byteArr)
{
    return ((uint32_t)(byteArr[0] & 0x00FFU))        |
           ((uint32_t)(byteArr[1] & 0x00FFU) << 8U)  |
           ((uint32_t)(byteArr[2] & 0x00FFU) << 16U) |
           ((uint32_t)(byteArr[3] & 0x00FFU) << 24U);
}

/**
 * @brief 将小端序 (Little-Endian) 的 4 个字节还原为 IEEE-754 单精度浮点数 float
 */
static inline float C28x_UnpackFloat_LE(const uint16_t *byteArr)
{
    union {
        uint32_t u32;
        float    f32;
    } conv;
    conv.u32 = C28x_UnpackU32_LE(byteArr);
    return conv.f32;
}

/**
 * @brief 将 IEEE-754 单精度浮点数 float 按小端序拆解为 4 个字节存入 byteArr[0..3]
 */
static inline void C28x_PackFloat_LE(uint16_t *byteArr, float val)
{
    union {
        uint32_t u32;
        float    f32;
    } conv;
    conv.f32   = val;
    byteArr[0] = (uint16_t)(conv.u32 & 0x00FFUL);
    byteArr[1] = (uint16_t)((conv.u32 >> 8U)  & 0x00FFUL);
    byteArr[2] = (uint16_t)((conv.u32 >> 16U) & 0x00FFUL);
    byteArr[3] = (uint16_t)((conv.u32 >> 24U) & 0x00FFUL);
}

#endif /* C28X_BYTE_PACK_UNPACK_H */
