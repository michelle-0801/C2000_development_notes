# 02 - 内存管理与链接脚本 (`02_memory_and_linker`)

本目录专门针对 C2000 的**物理内存架构**、**`.cmd` 链接命令文件**、**编译器段 (Sections) 映射**以及**多核/加速器共享内存**进行深度整理。

## 📝 推荐笔记规划

- `01_memory_map_overview.md`
  - **专用 RAM**：M0 / M1 SARAM（复位后栈空间常用区）、D0 / D1 RAM
  - **局部共享 RAM (`LS0` ~ `LS5`)**：CPU 与 CLA 共享，如何配置归属权与程序/数据属性
  - **全局共享 RAM (`GS0` ~ `GS15`)**：CPU1、CPU2 与 DMA 共享，如何通过 `GSxMSEL` 分配主控权
  - **Flash 与 Wait-State**：Flash 扇区划分、预取缓冲 (Prefetch)、等待周期配置
  - **消息 RAM (`MsgRAM`)**：CPU1 $\leftrightarrow$ CPU2、CPU $\leftrightarrow$ CLA 专用消息传递区
- `02_linker_cmd_syntax.md`
  - `MEMORY` 块详解：`PAGE 0`（程序空间）与 `PAGE 1`（数据空间）的历史背景与现行规范
  - `SECTIONS` 块详解：`>`（分配到单一区域）、`>>`（跨区域拆分分配）、`LOAD = ... RUN = ...`（加载地址与运行地址分离）、`ALIGN()` 对齐要求
- `03_compiler_sections_reference.md`
  - 代码与常量：`.text`、`.econst` / `.const`、`.cinit`、`.switch`
  - 变量与堆栈：`.ebss` / `.bss`、`.stack`、`.esysmem`（动态堆）
  - 自定义段：`#pragma CODE_SECTION` 与 `#pragma DATA_SECTION` 的用法（COFF vs EABI 格式差异）
- `04_flash_to_ram_execution.md`
  - `.TI.ramfunc`（或 `ramfuncs`）配置全流程
  - 链接器生成符号（`RamfuncsLoadStart`, `RamfuncsLoadSize`, `RamfuncsRunStart`）与上电 `memcpy()` 实操
- `05_map_file_analysis.md`
  - 如何阅读编译生成的 `.map` 文件排查内存溢出 (`placement fails for object`) 与查看函数/变量占用
