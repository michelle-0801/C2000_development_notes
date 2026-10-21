# 03 - 编译器标准段 (Sections) 与自定义段 `#pragma` 速查

> **关联手册**：*TMS320C28x Optimizing C/C++ Compiler User's Guide (`SPRU514`)* 第 7 章 "Linking C/C++ Code"

C2000 编译器在编译 `.c` / `.cpp` 源码时，不会把代码和变量混在一起，而是按照**生命周期、读写属性以及是否包含初始值**，严格分类放入不同的段（Sections）。

在编写 Flash 固化的独立运行程序时，**最核心的原则是**：
- **已初始化段（Initialized Sections）**：断电不能丢失，**必须加载（`LOAD`）在非易失性的 Flash 中**（部分高频段可在上电后拷贝到 RAM 运行）。
- **未初始化段（Uninitialized Sections）**：需要在运行时自由读写，**必须分配在 RAM 中**。

---

## 1. 编译器内置标准段全表（COFF vs EABI）

| 段名称 (COFF) | 段名称 (EABI) | 类型 | 存放内容说明 | Flash 工程分配位置 |
| :--- | :--- | :---: | :--- | :--- |
| **`codestart`** | **`codestart`** | 已初始化 | `F2837xD_CodeStartBranch.asm` 中的跳转指令（上电第一条用户指令） | **`BEGIN` (`0x080000`)** |
| **`.text`** | **`.text`** | 已初始化 | 所有普通 C/C++ 函数的编译后机器码指令 | **Flash** |
| **`.cinit`** | **`.cinit`** | 已初始化 | 全局/静态变量的**初始值压缩表**（在 `_c_int00` 启动时解压拷贝到 `.ebss`/`.bss`） | **Flash** |
| **`.econst`** | **`.const`** | 已初始化 | `const` 修饰的常量数组、字符串字面量（如 `"Hello"`） | **Flash** |
| **`.switch`** | **`.switch`** | 已初始化 | 大型 `switch-case` 语句生成的跳转地址查找表 | **Flash** |
| **`.pinit`** | **`.init_array`** | 已初始化 | C++ 全局对象构造函数指针列表 | **Flash** |
| **`ramfuncs`** / **`.TI.ramfunc`** | **`.TI.ramfunc`** | 已初始化 | 声明需要驻留 RAM 高速运行的函数（如 `InitFlash`、高频 ISR） | **`LOAD = Flash`, `RUN = RAM`** |
| **`.ebss`** | **`.bss`** + **`.data`** | 未初始化 | 所有全局与静态变量（`static` / global）实际占用的可读可写内存空间 | **RAM (`LSx` / `GSx` / `D0`)** |
| **`.stack`** | **`.stack`** | 未初始化 | 系统函数调用栈与局部变量、中断现场保护空间 | **低64K RAM (`M1` / `D0`)** |
| **`.esysmem`** | **`.sysmem`** | 未初始化 | 动态内存分配堆空间（供 `malloc` / `free` / `new` 使用，实时控制中极少用） | **RAM** |
| **`.reset`** | **`.reset`** | 已初始化 | 复位向量（C2000 硬件固定使用 Boot ROM 向量，工程中通常设 `TYPE = DSECT` 忽略） | `RESET` (`DSECT`) |

> [!WARNING]
> **COFF 与 EABI 对未显式赋初值全局变量的关键差异**：
> - 在 **EABI** 标准下，未赋初值的全局变量（如 `int a;`）严格按 C 标准在启动时**自动清零（Zero-initialized）**。
> - 在传统 **COFF** 模式下，未赋初值的全局变量默认**不会自动清零**（保持 RAM 上电随机值或上次复位前的值），除非显式写 `int a = 0;`！

---

## 2. 使用 `#pragma` 自定义函数与变量所在段

当你需要把某个中断函数放进 RAM、或者把某个数组放进 `GS0` 供 DMA 搬运时，需要使用 `#pragma CODE_SECTION` 和 `#pragma DATA_SECTION`。

> **注意**：`#pragma` 必须在函数或变量**定义/声明之前**出现，并且 **C 语言与 C++ 语言（或 COFF 与 EABI）的语法略有区别**！

### 2.1 将函数放入指定代码段 (`#pragma CODE_SECTION`)

#### 推荐写法 1：使用 TI 跨平台兼容属性宏（C / C++ 通用，最简洁）
在较新的 C2000-CGT 编译器（v15.12+）中，支持 GCC 风格的 `__attribute__((ramfunc))` 或 `__attribute__((section("name")))`：
```c
// 自动放入 .TI.ramfunc 段（C 与 C++ 均适用，无需单独写 #pragma）
__attribute__((ramfunc))
__interrupt void epwm1_isr(void)
{
    // 高速中断控制代码...
}
```

#### 推荐写法 2：传统 `#pragma CODE_SECTION`
```c
#ifdef __cplusplus
#pragma CODE_SECTION(".TI.ramfunc"); // C++ 语法：不写函数名，直接作用于紧随其后的第一个函数
#else
#pragma CODE_SECTION(epwm1_isr, ".TI.ramfunc"); // C 语法：(函数名, "段名")
#endif
__interrupt void epwm1_isr(void)
{
    // 高速中断控制代码...
}
```

---

### 2.2 将变量放入指定数据段 (`#pragma DATA_SECTION`)

例如：将 `adcDmaBuffer` 放入 `.cmd` 中定义好的 `ramgs0` 段，以允许 DMA 写入：

#### 步骤 1：在 C 源码中声明 `#pragma DATA_SECTION`
```c
#include <stdint.h>

// C 语言语法：#pragma DATA_SECTION(变量名, "自定义段名")
#pragma DATA_SECTION(adcDmaBuffer, "ramgs0");
volatile uint16_t adcDmaBuffer[512];

// 如果希望该数组按 32-bit (2 Words) 或更高边界对齐，可配合 DATA_ALIGN：
#pragma DATA_ALIGN(adcDmaBuffer, 4); // 强制按 4 Words (64-bit) 地址边界对齐
```
*(若使用 GCC 属性语法，也可直接写：`volatile uint16_t adcDmaBuffer[512] __attribute__((section("ramgs0"), aligned(4)));`)*

#### 步骤 2：确保 `.cmd` 文件的 `SECTIONS` 中有对应的映射
```text
SECTIONS
{
   ramgs0 : > RAMGS0, PAGE = 1
}
```

---

## 3. 断电保持/热复位不丢失变量 (`NOINIT`)

如果希望某个状态标志在看门狗热复位（Warm Reset）后依然保留复位前的 RAM 值，不被 `_c_int00` 清零：
```c
#pragma DATA_SECTION(resetCauseLog, ".TI.noinit");
uint32_t resetCauseLog;
// 或者使用属性：
uint32_t warmResetCounter __attribute__((noinit));
```
并在 `.cmd` 中确保 `.TI.noinit : > RAMM0, PAGE = 1`。
