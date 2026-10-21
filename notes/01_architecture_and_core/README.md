# 01 - C28x 内核与硬件架构 (`01_architecture_and_core`)

本目录记录 C28x CPU 内核基础、数据类型特性以及硬件数学加速单元的相关笔记。

## 📝 推荐笔记规划

- `01_c28x_data_types_and_byte_addressing.md`
  - **C28x 最小寻址单位为 16-bit Word** 的核心陷阱：`sizeof(char) == sizeof(int) == 1`（均为 16 位）
  - 跨平台通信（UART/CAN/SPI 字节流拆包与组包）中的 `__byte()` 内联函数与位域处理
- `02_cpu_registers_and_pipeline.md`
  - ACC, XAR0~XAR7, DP, SP, PC, ST0/ST1 寄存器作用
  - 8 级流水线特性与分支跳转开销
- `03_math_accelerators_fpu_tmu_vcu.md`
  - FPU32（单精度浮点单元）
  - TMU（三角函数数学单元：`sin`, `cos`, `atan`, `sqrt` 硬件加速）
  - VCU（维特比/复数/CRC 运算单元）
