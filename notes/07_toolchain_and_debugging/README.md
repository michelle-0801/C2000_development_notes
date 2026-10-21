# 07 - 工具链、编译器与调试排错 (`07_toolchain_and_debugging`)

本目录汇总 TI C2000-CGT 编译器高阶特性、CCS (Code Composer Studio) 调试工具技巧以及常见疑难 Bug 排查手册。

## 📚 笔记目录

1. **[01_c2000_cgt_compiler_tips.md](./01_c2000_cgt_compiler_tips.md)**
   - 核心编译器开关（`--float_support=fpu32`、`--tmu_support=tmu0`、`--fp_mode=relaxed`、`-O2`）
   - 常用 `#pragma` 指令与 GCC `__attribute__` 对照表、C28x 高频硬件内联函数（`__byte`、`__fsat`、`__sinpuf32` 等）速查
   - COFF (`coffabi`) 迁移至 EABI (`eabi`) 的四大差异对照表
2. **[02_ccs_debugging_techniques.md](./02_ccs_debugging_techniques.md)**
   - **Real-Time Debug Mode（实时不停机仿真）** 开启步骤与在线调参技巧
   - Expressions 窗口 `Q-Value` 定点数换算、CCS Graph 工具在线绘制片内高频波形、Hardware Watchpoint 抓捕内存越界踩踏
3. **[03_troubleshooting_and_pitfalls.md](./03_troubleshooting_and_pitfalls.md)**
   - 头号杀手 **`ILLEGAL_ISR` (`ITRAP0`)** 的五大成因与 **30 秒利用 `RPC` / `SP` 栈帧反查崩溃代码行绝招**
   - `-O2` 优化下 `volatile` 缺失死循环与主循环/ISR 32-bit 数据撕裂防护
