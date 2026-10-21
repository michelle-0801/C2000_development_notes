# C2000 MCU 开发学习笔记库 (Knowledge Base)

本仓库用于系统化沉淀 TI C2000 实时微控制器（以 **TMS320F2837xD** / C28x 内核为核心）的底层原理、**内存管理与链接脚本**、**DriverLib/寄存器 API** 以及实时控制外设开发笔记。

---

## 📁 目录架构总览

```text
C2000_development_notes/
├── README.md                          # 知识库总览与全局索引（本文件）
├── doc/                               # 官方技术手册、参考文档与网址导航
│   ├── README.md                      # 官方文档清单与检索指南 (SPRUHM8K / SPRS880 / SPRU513 / SPRU514)
│   ├── spruhm8k.pdf                   # TMS320F2837xD 双核技术参考手册 (TRM)
│   └── websites.txt                   # 常用官方链接备忘
├── notes/                             # 核心学习笔记（7大模块共27篇深度专题笔记）
│   ├── 01_architecture_and_core/      # C28x 内核架构、数据类型陷阱(16-bit char)、流水线与数学加速单元
│   ├── 02_memory_and_linker/          # 【重点】内存映射、CMD链接脚本、Section分配、Flash搬移RAM、.map分析
│   ├── 03_api_and_software_framework/ # 【重点】C2000Ware架构、DriverLib API 字典、Bitfield对比、SysConfig
│   ├── 04_system_and_interrupts/      # 启动流程(Boot)、时钟树(PLL)、看门狗、EALLOW、PIE三级中断体系
│   ├── 05_peripherals/                # 实时控制外设：GPIO/X-BAR、ePWM、ADC/CMPSS硬件保护、eCAP/eQEP、通信
│   ├── 06_accelerators_and_multicore/ # CLA协处理器、DMA二维寻址、双核(CPU1/CPU2)启动与IPC通信
│   └── 07_toolchain_and_debugging/    # C2000-CGT 编译器特性、#pragma/内联函数、CCS实时调试与ITRAP排错
├── snippets/                          # 可复用实战代码模板 (.cmd 链接脚本模板、ePWM+ADC闭环脚手架、字节打包工具)
└── assets/                            # 笔记引用的图片、架构框图、CCS 波形截图等静态资源
```

---

## 🗺️ 全库专题笔记直达导航

### 01. [C28x 内核与硬件架构 (`01_architecture_and_core`)](./notes/01_architecture_and_core/README.md)
- [01 - C28x 数据类型与 16-bit 字节寻址特性 (`sizeof(char)==1` 陷阱、`__byte()` 与结构体对齐)](./notes/01_architecture_and_core/01_c28x_data_types_and_byte_addressing.md)
- [02 - C28x CPU 核心寄存器 (`ACC`/`XAR`/`DP`/`SP`) 与 8 级流水线延迟保护](./notes/01_architecture_and_core/02_cpu_registers_and_pipeline.md)
- [03 - 硬件数学加速单元：FPU32、TMU 三角函数硬件加速与 VCU-II](./notes/01_architecture_and_core/03_math_accelerators_fpu_tmu_vcu.md)

### 02. [内存管理与链接脚本 (`02_memory_and_linker`)](./notes/02_memory_and_linker/README.md) ⭐
- [01 - TMS320F2837xD 内存映射总览 (`M0/M1`, `D0/D1`, `LSx`, `GSx`, `MsgRAM`) 与三大硬件访问铁律](./notes/02_memory_and_linker/01_memory_map_overview.md)
- [02 - Linker Command File (`.cmd`) 语法详解 (`MEMORY`, `PAGE 0/1`, `SECTIONS`, `>>`, `LOAD/RUN`)](./notes/02_memory_and_linker/02_linker_cmd_syntax.md)
- [03 - 编译器标准段 (`.text`, `.ebss`, `.cinit`, `.stack`) 与 `#pragma CODE/DATA_SECTION` 速查](./notes/02_memory_and_linker/03_compiler_sections_reference.md)
- [04 - Flash 代码搬移至零等待 RAM 运行全流程 (`.TI.ramfunc` 与 `memcpy`)](./notes/02_memory_and_linker/04_flash_to_ram_execution.md)
- [05 - `.map` 内存映射文件解读与 `#10099-D` 内存溢出排错](./notes/02_memory_and_linker/05_map_file_analysis.md)

### 03. [API、驱动库与软件架构 (`03_api_and_software_framework`)](./notes/03_api_and_software_framework/README.md) ⭐
- [01 - C2000Ware 目录结构、LaunchPad/ControlCARD 晶振差异与 CCS 工程配置指南](./notes/03_api_and_software_framework/01_c2000ware_directory_guide.md)
- [02 - 编程模型对比：DriverLib API vs Bitfield 寄存器位域 vs 混合编程模式](./notes/03_api_and_software_framework/02_driverlib_vs_bitfield.md)
- [03 - SysConfig (`.syscfg`) 图形化配置工具工作流指南](./notes/03_api_and_software_framework/03_sysconfig_workflow.md)
- **DriverLib 核心外设 API 分类速查字典 (`04_api_quick_reference/`)**：
  - [API 速查 01 - 系统控制 (`SysCtl`)、GPIO 与 X-BAR 路由](./notes/03_api_and_software_framework/04_api_quick_reference/api_sysctl_and_gpio.md)
  - [API 速查 02 - PIE 中断管理 (`Interrupt`) 与 CPU 定时器 (`CPUTimer`)](./notes/03_api_and_software_framework/04_api_quick_reference/api_interrupt_and_pie.md)
  - [API 速查 03 - 增强型脉宽调制 (`EPWM`) 七大子模块 API 字典与 `TBCLKSYNC` 同步法则](./notes/03_api_and_software_framework/04_api_quick_reference/api_epwm_and_hrpwm.md)
  - [API 速查 04 - 模数转换器 (`ADC` + `PPB`) 与片上比较器 (`CMPSS`)](./notes/03_api_and_software_framework/04_api_quick_reference/api_adc_and_cmpss.md)
  - [API 速查 05 - 内存权限配置 (`MemCfg`)、DMA 与双核通信 (`IPC`)](./notes/03_api_and_software_framework/04_api_quick_reference/api_dma_and_ipc.md)

### 04. [系统控制、时钟与中断体系 (`04_system_and_interrupts`)](./notes/04_system_and_interrupts/README.md)
- [01 - 芯片上电启动流程：从复位向量、Boot ROM (`GPIO72/84`)、`codestart` 到 `main()`](./notes/04_system_and_interrupts/01_boot_rom_and_startup.md)
- [02 - 系统时钟树 (`SYSPLL`)、三大子时钟域 (`EPWMCLK`/`LSPCLK`/`ADCCLK`) 与看门狗机制](./notes/04_system_and_interrupts/02_clock_tree_and_watchdog.md)
- [03 - 寄存器写保护机制：`EALLOW` 与 `EDIS` 原理及混合编程防坑](./notes/04_system_and_interrupts/03_register_protection_eallow_edis.md)
- [04 - PIE 三级中断体系、响应流程与软件中断嵌套标准模板](./notes/04_system_and_interrupts/04_pie_interrupt_architecture.md)

### 05. [实时控制外设 (`05_peripherals`)](./notes/05_peripherals/README.md)
- [01 - GPIO 二级复用、输入限定滤波 (Qualification) 与三大 X-BAR 信号路由](./notes/05_peripherals/01_gpio_and_xbar.md)
- [02 - ePWM 深度解析：七大子模块、对称/非对称周期占空比公式、死区 `AHC` 与双更新模式](./notes/05_peripherals/02_epwm_deep_dive.md)
- [03 - ADC 多核同步采样与 `CMPSS + ePWM X-BAR + TZ` 纯硬件微秒级过流封波全链路](./notes/05_peripherals/03_adc_and_cmpss.md)
- [04 - eCAP 四级时间戳脉冲捕获与 eQEP 正交编码器 M 法 / T 法测速原理](./notes/05_peripherals/04_ecap_and_eqep.md)
- [05 - 串行通信外设：SCI (UART) 错误自愈、SPI 极性对照与左对齐移位、DCAN 邮箱机制](./notes/05_peripherals/05_communications.md)

### 06. [协处理器、DMA 与双核通信 (`06_accelerators_and_multicore`)](./notes/06_accelerators_and_multicore/README.md)
- [01 - CLA 控制律加速器编程全指南 (`.cla` 语法限制、内存映射、初始化五步法与单步调试)](./notes/06_accelerators_and_multicore/01_cla_control_law_accelerator.md)
- [02 - DMA 直接内存访问控制器 (二维 `Burst`/`Transfer` 步进计算与多通道 ADC 自动解交织)](./notes/06_accelerators_and_multicore/02_dma_controller.md)
- [03 - 双核架构 (CPU1 & CPU2)：主从启动引导、GPIO/外设归属移交与 IPC 核间通信](./notes/06_accelerators_and_multicore/03_dual_core_cpu1_cpu2_and_ipc.md)

### 07. [工具链、编译器与调试排错 (`07_toolchain_and_debugging`)](./notes/07_toolchain_and_debugging/README.md)
- [01 - TI C2000-CGT 编译器选项、`#pragma`、硬件内联函数 (Intrinsics) 与 COFF/EABI 迁移对照](./notes/07_toolchain_and_debugging/01_c2000_cgt_compiler_tips.md)
- [02 - CCS 实时不停机调试 (Real-Time Mode)、`Q-Value` 换算、Graph 波形绘图与硬件观察点](./notes/07_toolchain_and_debugging/02_ccs_debugging_techniques.md)
- [03 - C2000 常见疑难故障排查手册 (`ILLEGAL_ISR` 栈帧溯源、`volatile` 优化失效与 32位数据撕裂)](./notes/07_toolchain_and_debugging/03_troubleshooting_and_pitfalls.md)

### 🛠️ [可复用实战代码模板 (`snippets/`)](./snippets/README.md)
- [f2837xd_flash_custom_template.cmd](./snippets/f2837xd_flash_custom_template.cmd) —— 兼容 COFF/EABI 的 F2837xD Flash 定制链接脚本模板
- [epwm_adc_closed_loop_template.c](./snippets/epwm_adc_closed_loop_template.c) —— 20kHz ePWM1 互补发波 + ADCA 同步采样 + RAM 闭环中断标准工程模板
- [c28x_byte_pack_unpack.h](./snippets/c28x_byte_pack_unpack.h) —— 基于 `__byte()` 的 8-bit 字节流打包与大小端解析工具头文件

---

## 🔗 常用官方资源入口

- **TI Resource Explorer (在线例程与文档库)**：[https://dev.ti.com/tirex/explore](https://dev.ti.com/tirex/explore)
- **C2000 Code Generation Tools (C2000-CGT 编译器与链接器工具链)**：[https://www.ti.com/tool/C2000-CGT](https://www.ti.com/tool/C2000-CGT)
- **本地官方文档索引**：详见 [doc/README.md](./doc/README.md)
