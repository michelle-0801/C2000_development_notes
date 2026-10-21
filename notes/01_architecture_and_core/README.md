# 01 - C28x 内核与硬件架构 (`01_architecture_and_core`)

本目录记录 C28x CPU 内核基础、数据类型特性以及硬件数学加速单元的相关笔记。

## 📚 笔记目录

1. **[01_c28x_data_types_and_byte_addressing.md](./01_c28x_data_types_and_byte_addressing.md)**
   - **C28x 最小寻址单位为 16-bit Word** 的核心机制（`sizeof(char) == sizeof(int) == 1` 且均为 16 位）
   - COFF 与 EABI 格式的数据类型位宽对比（注意 EABI 下 `double` 为 64 位）
   - 跨平台通信（UART/CAN/SPI 8-bit 字节流）拆包与组包方案、`__byte()` 内联函数与结构体偶数地址对齐规则
2. **[02_cpu_registers_and_pipeline.md](./02_cpu_registers_and_pipeline.md)**
   - `ACC`, `P`, `XT`, `XAR0`~`XAR7`, `DP`, `SP`, `PC`, `RPC`, `ST0`/`ST1` 寄存器详解（含 `SP` 仅限低 64K Words 寻址限制）
   - 8 级指令流水线（F1~W）机制、外设写后读流水线延迟与 Flash 取指分支惩罚
3. **[03_math_accelerators_fpu_tmu_vcu.md](./03_math_accelerators_fpu_tmu_vcu.md)**
   - **FPU32**：单精度浮点寄存器组与硬件饱和内联函数 `__fsat()`
   - **TMU**：三角函数（`__sinpuf32`, `__cospuf32`, `__atanpuf32`）、硬件除法（`__divf32`）与开方（`__sqrt`）加速配置与用法
   - **VCU-II**：硬件 CRC-8/16/32 与复数运算加速简介
