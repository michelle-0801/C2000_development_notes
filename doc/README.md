# 官方技术文档与参考资料索引 (`doc/`)

本目录用于收纳 TI 官方技术手册（PDF）与常用在线资源导航。

---

## 📚 已收录本地文档

| 文件名 | 文献编号 | 文档名称 | 核心用途 |
| :--- | :--- | :--- | :--- |
| [spruhm8k.pdf](./spruhm8k.pdf) | **SPRUHM8K** | *TMS320F2837xD Dual-Core Microcontrollers Technical Reference Manual (TRM)* | **最核心底层手册**：包含内存映射细则、系统控制、双核IPC、CLA、DMA 及所有外设寄存器与工作机制详解。 |
| [websites.txt](./websites.txt) | - | 原始官方网址收藏记录 | 见下方整理后的在线导航。 |

---

## 📌 建议后续补充的官方核心文档

在深入学习**内存管理**、**编译器特性**与**硬件设计**时，推荐从 TI 官网补充以下几份高频查阅手册放入本目录：

1. **数据手册 (Datasheet)**
   - **文献号**：`SPRS880` (*TMS320F2837xD Dual-Core Microcontrollers Data Manual*)
   - **用途**：查阅具体的 Memory Map 起止地址表、引脚复用表 (Pinmux / GPIO Mux)、电气特性与时序参数。
2. **汇编与链接工具手册 (Linker & Assembly Tools)**
   - **文献号**：`SPRU513` (*TMS320C28x Assembly Language Tools User's Guide*)
   - **用途**：**学习内存管理必读**，详解 Linker Command File (`.cmd`) 语法、`MEMORY` 与 `SECTIONS` 伪指令、`.map` 文件解读。
3. **C/C++ 编译器手册 (Optimizing Compiler)**
   - **文献号**：`SPRU514` (*TMS320C28x Optimizing C/C++ Compiler User's Guide*)
   - **用途**：详解 C28x 数据类型长度（16位 `char`/`int`）、编译器生成的段（`.ebss`, `.econst`, `.TI.ramfunc` 等）、`#pragma` 指令与内联汇编/Intrinsics。
4. **C2000Ware DriverLib API 指南**
   - **本地路径**（安装 C2000Ware 后）：`C:/ti/c2000/C2000Ware_<version>/device_support/f2837xd/docs/` 或 `driverlib/f2837xd/docs/`

---

## 🌐 官方在线入口（整理自 `websites.txt`）

- **TI Resource Explorer (TIREX)**：[https://dev.ti.com/tirex/explore](https://dev.ti.com/tirex/explore)
  - 在线浏览 C2000Ware 所有芯片例程、DriverLib API 文档及 C2000 Academy 教程。
- **C2000-CGT 编译器工具链主页**：[https://www.ti.com/tool/C2000-CGT#tech-docs](https://www.ti.com/tool/C2000-CGT#tech-docs)
  - 下载编译器版本及查阅 `SPRU513` / `SPRU514` 最新版文档。
