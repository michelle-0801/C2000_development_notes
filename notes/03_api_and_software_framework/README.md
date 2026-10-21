# 03 - API、驱动库与软件架构 (`03_api_and_software_framework`)

本目录用于整理 C2000Ware 软件生态、**DriverLib 函数 API**、**Bitfield 寄存器位域**操作方式以及常用 API 速查笔记。

## 📝 推荐笔记规划

- `01_c2000ware_directory_guide.md`
  - `driverlib/`（底层函数驱动库源码与头文件）
  - `device_support/`（芯片级头文件、传统位域结构体、通用 `.cmd` 文件、启动代码）
  - `libraries/`（DSP、数学库、电机控制/数字电源算法库）
- `02_driverlib_vs_bitfield.md`
  - **DriverLib 方式**（如 `EPWM_setTimeBasePeriod(myEPWM_BASE, 1000)`）：可读性高、带 `ASSERT` 参数检查、支持 SysConfig
  - **Bitfield 方式**（如 `EPwm1Regs.TBPRD.bit.TBPRD = 1000`）：直观映射 TRM 寄存器名、极致执行效率考量
  - **混合编程模式**：如何在同一个工程中优雅混用 DriverLib 初始化与高频 ISR 内的寄存器直写
- `03_sysconfig_workflow.md`
  - 使用图形化工具 `.syscfg` 自动生成 `board.c` / `board.h` 的工程组织方法
- `04_api_quick_reference/`（可按模块细分子笔记）
  - `api_sysctl_and_gpio.md`：时钟、外设门控、引脚配置与 X-BAR API
  - `api_interrupt_and_pie.md`：`Interrupt_register()`、`Interrupt_enable()` 等中断管理 API
  - `api_epwm_and_hrpwm.md`：ePWM 核心配置 API 组合套路
  - `api_adc_and_cmpss.md`：ADC SOC 配置、结果读取与比较器保护 API
  - `api_dma_and_ipc.md`：DMA 通道配置与双核 IPC 握手/数据交换 API
