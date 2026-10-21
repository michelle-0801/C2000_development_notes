# C2000 MCU 开发学习笔记库 (Knowledge Base)

本仓库用于系统化沉淀 TI C2000 实时微控制器（以 **TMS320F2837xD** / C28x 内核为核心）的底层原理、内存管理、DriverLib/寄存器 API 以及外设开发笔记。

---

## 📁 目录架构总览

```text
C2000_development_notes/
├── README.md                          # 知识库总览与全局索引（本文件）
├── doc/                               # 官方技术手册、参考文档与网址导航
│   ├── README.md                      # 官方文档清单与检索指南
│   ├── spruhm8k.pdf                   # TMS320F2837xD 双核技术参考手册 (TRM)
│   └── websites.txt                   # 常用官方链接备忘
├── notes/                             # 核心学习笔记（按模块递进编号）
│   ├── 01_architecture_and_core/      # C28x 内核架构、数据类型陷阱(16-bit char)、流水线与数学加速单元
│   ├── 02_memory_and_linker/          # 【重点】内存映射、CMD链接脚本、Section分配、Flash搬移RAM、共享内存
│   ├── 03_api_and_software_framework/ # 【重点】C2000Ware架构、DriverLib API 速查、Bitfield寄存器对比、SysConfig
│   ├── 04_system_and_interrupts/      # 启动流程(Boot)、时钟树(PLL)、看门狗、PIE三级中断体系
│   ├── 05_peripherals/                # 实时控制外设：GPIO/X-BAR、ePWM、ADC/CMPSS、eCAP/eQEP、通信接口
│   ├── 06_accelerators_and_multicore/ # CLA协处理器、DMA控制器、双核(CPU1/CPU2)通信(IPC)与资源分配
│   └── 07_toolchain_and_debugging/    # C2000-CGT 编译器特性、#pragma 指令、CCS 调试技巧与异常排错
├── snippets/                          # 可复用代码片段、自定义 .cmd 模板与外设配置脚手架
└── assets/                            # 笔记引用的图片、架构框图、CCS 波形截图等静态资源
```

---

## 🗺️ 核心模块索引

| 编号 | 模块目录 | 核心关注点 |
| :--- | :--- | :--- |
| **01** | [01_architecture_and_core](./notes/01_architecture_and_core/README.md) | C28x 16位寻址特性（`sizeof(char)==1` 且为16-bit）、FPU32/TMU/VCU 硬件加速 |
| **02** | [02_memory_and_linker](./notes/02_memory_and_linker/README.md) | RAM/Flash 分区、`.cmd` 语法（`MEMORY` & `SECTIONS`）、`.TI.ramfunc`、双核/CLA/DMA 内存权限 |
| **03** | [03_api_and_software_framework](./notes/03_api_and_software_framework/README.md) | DriverLib 函数 API 字典、Bitfield 结构体方式、C2000Ware 目录结构、SysConfig |
| **04** | [04_system_and_interrupts](./notes/04_system_and_interrupts/README.md) | 芯片上电 Boot、系统时钟配置、EALLOW/EDIS 写保护、外设 $\rightarrow$ PIE $\rightarrow$ CPU 三级中断 |
| **05** | [05_peripherals](./notes/05_peripherals/README.md) | 输入/输出 X-BAR 路由、ePWM 发波与保护、ADC 采样触发与后处理(PPB)、正交编码/捕获 |
| **06** | [06_accelerators_and_multicore](./notes/06_accelerators_and_multicore/README.md) | CLA 并行控制任务、DMA 搬运、CPU1 与 CPU2 外设归属划分与 IPC 同步 |
| **07** | [07_toolchain_and_debugging](./notes/07_toolchain_and_debugging/README.md) | C2000-CGT 编译器内联函数(Intrinsics)、`.map` 文件分析、ITRAP 非法中断排查 |

---

## 🔗 常用官方资源入口

- **TI Resource Explorer (在线例程与文档库)**：[https://dev.ti.com/tirex/explore](https://dev.ti.com/tirex/explore)
- **C2000 Code Generation Tools (C2000-CGT 编译器与链接器工具链)**：[https://www.ti.com/tool/C2000-CGT](https://www.ti.com/tool/C2000-CGT)
- **本地官方文档索引**：详见 [doc/README.md](./doc/README.md)
