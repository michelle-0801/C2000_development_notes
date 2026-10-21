# 02 - 内存管理与链接脚本 (`02_memory_and_linker`)

本目录专门针对 C2000 的**物理内存架构**、**`.cmd` 链接命令文件**、**编译器段 (Sections) 映射**以及**多核/加速器共享内存**进行深度整理。

## 📚 笔记目录

1. **[01_memory_map_overview.md](./01_memory_map_overview.md)**
   - F2837xD 全景内存架构：专用 RAM (`M0/M1`, `D0/D1`)、局部共享 RAM (`LS0~LS5`)、全局共享 RAM (`GS0~GS15`)、消息 RAM (`MsgRAM`) 与 Flash 扇区
   - **硬件访问三大铁律**：DMA 仅限访问 `GSx`、CLA 仅限访问 `LSx`、双核下 `GSx` 主控写权限互斥
2. **[02_linker_cmd_syntax.md](./02_linker_cmd_syntax.md)**
   - `MEMORY` 块详解：`PAGE 0` 与 `PAGE 1` 的历史背景与防重叠铁律、合并连续内存块技巧
   - `SECTIONS` 块详解：`>`、`|`、`>>`（跨块拆分）、`LOAD = ..., RUN = ...`、`ALIGN(4/8)` 与 `TABLE(BINIT)`
3. **[03_compiler_sections_reference.md](./03_compiler_sections_reference.md)**
   - 编译器内置段全表（COFF vs EABI）：`.text`, `.cinit`, `.econst`/`.const`, `.ebss`/`.bss`, `.stack`, `.TI.ramfunc`
   - 使用 `#pragma CODE_SECTION`、`#pragma DATA_SECTION`、`__attribute__((ramfunc))` 与 `.TI.noinit` 定向分配代码与变量
4. **[04_flash_to_ram_execution.md](./04_flash_to_ram_execution.md)**
   - 为什么 `InitFlash()` 与高频控制 ISR 必须搬移到零等待 RAM 运行
   - `.TI.ramfunc` 实操三步法与 `memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize)` 底层细节
5. **[05_map_file_analysis.md](./05_map_file_analysis.md)**
   - `.map` 文件四大核心区解读（验证 `RUN ADDR`、定位相邻变量越界踩踏）
   - 诊断与修复 `error #10099-D: program will not fit into available memory`

> **配套模板**：可直接参考 [snippets/f2837xd_flash_custom_template.cmd](../../snippets/f2837xd_flash_custom_template.cmd) 查看带完整中文注释的实战 `.cmd` 文件模板。
