# 01 - C28x 数据类型与 16-bit 字节寻址特性

> **关联手册**：*TMS320C28x Optimizing C/C++ Compiler User's Guide (`SPRU514`)* 第 6 章 "C28x C/C++ Language Implementation"

从 ARM Cortex-M（STM32）或标准 x86/64 平台迁移到 TI C2000（C28x 内核）时，**最容易踩坑的第一件事**就是：**C28x 的最小可寻址内存单元是 16-bit（1 Word），而不是 8-bit（1 Byte）！**

---

## 1. 为什么 `sizeof(char) == 1` 且占 16 位？

在 C 语言标准（ISO/IEC 9899）中，规定了：

1. `sizeof(char)` **恒等于 `1`**（表示 1 个最小可寻址单元 `char`）。
2. 标准只要求 `CHAR_BIT >= 8`，并未强制规定 `1 byte` 必须是 8 bit。

由于 C28x CPU 的地址总线（Address Bus）每加 `1`，指向的物理内存单元就是 **16 bit（2 个 8-bit 字节）**，因此：

- 在 C28x 上，`CHAR_BIT == 16`。
- **`char`、`unsigned char`、`int`、`short` 的长度全部都是 16 bit！**
- `sizeof(int) == sizeof(char) == 1`（单位是 16-bit Word）。
- `sizeof(long) == sizeof(float) == 2`（即 $2 \times 16\text{ bit} = 32\text{ bit}$）。

> [!WARNING]
> 在 C2000 标准头文件 `<stdint.h>` 中，**根本不存在 `uint8_t` 或 `int8_t`**！最小的标准整型是 `uint16_t` 和 `int16_t`。如果直接移植包含 `uint8_t` 的第三方协议栈代码，编译器会直接报错 `identifier "uint8_t" is undefined`。

---

## 2. C28x 标准数据类型大小速查表（COFF vs EABI）

TI C2000 编译器支持两种二进制目标格式：传统的 **COFF (`coffabi`)** 和现代的 **EABI (`eabi`)**。两者在 `long` 和 `double` 类型长度上有重要区别：

| C/C++ 数据类型                 | `<stdint.h>` 别名           |     位宽 (Bits) [COFF]     | 位宽 (Bits) [EABI] | `sizeof()` 返回值 (16-bit Words) | 取值范围（无符号/有符号）                      |
| :----------------------------- | :---------------------------- | :------------------------: | :----------------: | :--------------------------------: | :--------------------------------------------- |
| `char` / `unsigned char`   | `int16_t` / `uint16_t`    |        **16**        |    **16**    |               `1`               | `-32768 ~ 32767` / `0 ~ 65535`             |
| `short` / `unsigned short` | `int16_t` / `uint16_t`    |        **16**        |    **16**    |               `1`               | 同上                                           |
| `int` / `unsigned int`     | `int16_t` / `uint16_t`    |        **16**        |    **16**    |               `1`               | 同上（**注意：`int` 是 16 位！**）     |
| `long` / `unsigned long`   | `int32_t` / `uint32_t`    |        **32**        |    **32**    |               `2`               | $\pm 2.14 \times 10^9$ / `0 ~ 4.29e9`      |
| `long long`                  | `int64_t` / `uint64_t`    |        **64**        |    **64**    |               `4`               | 64 位整数                                      |
| `float`                      | `float32_t`                 |        **32**        |    **32**    |               `2`               | IEEE-754 单精度浮点（FPU32 硬件支持）          |
| `double`                     | `float32_t` / `float64_t` |     **32** ⚠️     | **64** ⚠️ |    `2` (COFF) / `4` (EABI)    | EABI 下未加 `f` 后缀的浮点常量默认为 64 位！ |
| `long double`                | `float64_t`                 |        **64**        |    **64**    |               `4`               | IEEE-754 双精度浮点                            |
| 指针 (`void *` / 数据指针)   | -                             | **32** (22-bit 有效) |    **32**    |               `2`               | 最大寻址 4M Words (`0x000000 ~ 0x3FFFFF`)    |

> [!IMPORTANT]
> **EABI 模式下的浮点性能陷阱**：
> 在 EABI 模式下，`double` 为 64 位。由于 F2837xD 配备的是 **FPU32（单精度硬件浮点单元）**，如果你在代码中写 `y = x * 0.5;`（未写 `0.5f`），编译器会将其提升为 64 位 `double` 软件模拟运算，耗时暴增数十倍！
> **规范做法**：所有单精度浮点字面量必须加 `f` 后缀（如 `0.5f`, `3.1415926f`），并开启编译器警告 `--float_operations_allowed=32` 捕获隐式双精度运算。

---

## 3. 常见开发陷阱与解决方案

### 陷阱 1：`int` 溢出问题

在 32 位 ARM MCU 上，`int` 是 32 位，写 `int count = 100000;` 完全正常；但在 C28x 上，`int` 是 **16 位**（最大值 `32767`），上述赋值会直接发生有符号溢出！

- **法则**：在 C2000 编程中，**严禁裸用 `int`、`char`、`long`**，一律使用 `<stdint.h>` 或 C2000Ware 定义的精确位宽类型：`uint16_t`、`int16_t`、`uint32_t`、`int32_t`、`float32_t`。

### 陷阱 2：`memcpy` 与 `sizeof` 的字节计数误区

在 C28x 中：

```c
uint32_t buffer[10];
// sizeof(buffer) 返回的是 20 (即 20 个 16-bit Word)，而不是 40！
// C28x 标准库的 memcpy 第三个参数也是按 16-bit Word 计数的，因此下面写法是正确的：
memcpy(dest, buffer, sizeof(buffer));
```

但如果你在手写通信协议（如计算 8-bit 字节长度的报文头 `len`）时，千万注意：`sizeof(struct)` 返回的是 **16-bit 字数**，对应的 8-bit 字节数是 `sizeof(struct) * 2`！

### 陷阱 3：UART / SPI / CAN 通信中的 8-bit 字节流拆包与组包

外设（如 SCI/UART、CAN）传输的仍然是标准的 **8-bit 字节**。当用 `uint16_t rxBuf[]` 存储接收到的 8-bit 数据时：

#### 方案 A：一个 `uint16_t` 存一个 8-bit 字节（空间换简单，最常用）

直接将每个 `uint16_t` 的低 8 位（`0x00FF`）作为 1 个字节使用，高 8 位闲置为 0：

```c
uint16_t txByte = 0x00A5; // 只用低 8 位
uint32_t rawVal = 0x12345678UL;
uint16_t frame[4];

frame[0] = (uint16_t)((rawVal >> 24) & 0x00FFU);
frame[1] = (uint16_t)((rawVal >> 16) & 0x00FFU);
frame[2] = (uint16_t)((rawVal >> 8)  & 0x00FFU);
frame[3] = (uint16_t)(rawVal & 0x00FFU);
```

#### 方案 B：使用编译器内联函数 `__byte()` 紧凑访问 8-bit 字节（推荐高阶用法）

C28x CPU 提供了专门的字节寻址汇编指令 `MOVB`，编译器将其封装为内联函数 `int &__byte(int *array, unsigned int byte_index)`：

- `array`：指向 `int` / `uint16_t` 数组的指针（强转为 `int *`）。
- `byte_index`：**按 8-bit 字节计算的索引**（小端序：偶数索引访问低 8 位 `LSB`，奇数索引访问高 8 位 `MSB`）。

```c
#include <stdint.h>

uint16_t packedData[2] = {0x1234, 0x5678};

// 按 8-bit 字节读取（C28x 为小端序 Little-Endian）：
uint16_t b0 = __byte((int *)packedData, 0); // 0x0034 (packedData[0] 低8位)
uint16_t b1 = __byte((int *)packedData, 1); // 0x0012 (packedData[0] 高8位)
uint16_t b2 = __byte((int *)packedData, 2); // 0x0078 (packedData[1] 低8位)
uint16_t b3 = __byte((int *)packedData, 3); // 0x0056 (packedData[1] 高8位)

// 按 8-bit 字节写入（单周期 MOVB 指令完成，不破坏同 Word 的另一半字节！）：
__byte((int *)packedData, 1) = 0xAA;        // packedData[0] 变为 0xAA34
```

> [!TIP]
> 在本仓库的 [snippets/c28x_byte_pack_unpack.h](../../../snippets/c28x_byte_pack_unpack.h) 中，提供了一套基于 `__byte()` 封装好的通用 8-bit 字节流读写工具宏与内联函数。

---

## 4. 结构体对齐 (Alignment) 规则

在 C28x 中，硬件对内存访问有以下对齐要求：

1. **16-bit 变量**（`uint16_t`, `int16_t`）：可存放在任意地址（奇数或偶数地址均可，1-word 对齐）。
2. **32-bit 变量**（`uint32_t`, `int32_t`, `float32_t`, 指针）：**必须存放在偶数地址（Even Address，即 2-word 边界对齐）**！

如果结构体中 16-bit 成员与 32-bit 成员交错排列，编译器会自动插入 1 个 16-bit Word 的空洞（Padding）：

```c
typedef struct {
    uint16_t flag;   // 偏移 0 (占 1 Word)
    // [编译器自动插入 1 Word Padding，以保证 value 在偶数偏移 2 上对齐]
    float32_t value; // 偏移 2 (占 2 Words)
    uint16_t status; // 偏移 4 (占 1 Word)
    // [尾部补齐 1 Word Padding，使结构体总大小为偶数 6 Words]
} BadStruct_t;       // sizeof(BadStruct_t) == 6 Words!

typedef struct {
    float32_t value; // 偏移 0 (占 2 Words)
    uint16_t flag;   // 偏移 2 (占 1 Word)
    uint16_t status; // 偏移 3 (占 1 Word)
} GoodStruct_t;      // sizeof(GoodStruct_t) == 4 Words! (零空洞)
```

> [!CAUTION]
> C28x 编译器**不支持** `#pragma pack(1)` 取消 32-bit 变量的偶数地址对齐！因为 C28x 硬件的 32 位读写指令（如 `MOVL`）如果访问奇数地址，会导致高低字错位或硬件行为异常。因此设计结构体时，应始终**将 32-bit 成员集中放在前面，16-bit 成员成对放在后面**。
