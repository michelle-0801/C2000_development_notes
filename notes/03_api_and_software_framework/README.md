# 03 - API、驱动库与软件架构 (`03_api_and_software_framework`)

本目录用于整理 C2000Ware 软件生态、**DriverLib 函数 API**、**Bitfield 寄存器位域**操作方式以及常用外设 API 速查字典。

## 📚 笔记目录

1. **[01_c2000ware_directory_guide.md](./01_c2000ware_directory_guide.md)**
   - `driverlib/`、`device_support/`、`libraries/` 目录职责详解
   - 标准 DriverLib 工程结构 (`device.c` / `device.h`)、LaunchPad (10MHz) 与 ControlCARD (20MHz) 晶振差异陷阱
   - CCS 工程 Include 路径、预定义宏 (`CPU1`, `_FLASH`, `_LAUNCHXL_F28379D`, `DEBUG`) 与静态库配置清单
2. **[02_driverlib_vs_bitfield.md](./02_driverlib_vs_bitfield.md)**
   - **DriverLib API** vs **Bitfield 结构体** vs **`HWREGH` 宏** 全方位对比
   - 为什么高频 ISR 内调用 DriverLib API（`static inline`）与直接写寄存器拥有完全相同的单周期汇编效率
   - **混合编程模式 (Hybrid Mode)** 工程搭建步骤与 `EALLOW`/`EDIS` 注意事项
3. **[03_sysconfig_workflow.md](./03_sysconfig_workflow.md)**
   - TI SysConfig (`.syscfg`) 实时代码生成机制（`board.c` / `board.h`）与标准 `main.c` 模板
4. **外设 DriverLib API 分类速查字典 (`04_api_quick_reference/`)**
   - **[api_sysctl_and_gpio.md](./04_api_quick_reference/api_sysctl_and_gpio.md)**：系统时钟、看门狗、外设门控 (`SysCtl_*`)、GPIO 配置与读写 (`GPIO_*`)、内部信号路由 (`XBAR_*`)
   - **[api_interrupt_and_pie.md](./04_api_quick_reference/api_interrupt_and_pie.md)**：PIE 向量表注册与应答 (`Interrupt_*`)、中断组号对应表、32位 CPU 定时器 (`CPUTimer_*`)
   - **[api_epwm_and_hrpwm.md](./04_api_quick_reference/api_epwm_and_hrpwm.md)**：ePWM 七大子模块 (`TB`/`CC`/`AQ`/`DB`/`ET`/`TZ`/`DC`) 核心 API 与 `TBCLKSYNC` 多路同步初始化黄金法则
   - **[api_adc_and_cmpss.md](./04_api_quick_reference/api_adc_and_cmpss.md)**：ADC 采样与 PPB 硬件去偏置 (`ADC_*`)、片上模拟比较器硬件过流保护 (`CMPSS_*`)
   - **[api_dma_and_ipc.md](./04_api_quick_reference/api_dma_and_ipc.md)**：`LSx`/`GSx` 内存权限划分 (`MemCfg_*`)、DMA 二维传输配置 (`DMA_*`)、双核同步与通信 (`IPC_*`)
