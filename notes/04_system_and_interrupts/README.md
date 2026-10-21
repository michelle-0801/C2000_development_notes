# 04 - 系统控制、时钟与中断体系 (`04_system_and_interrupts`)

本目录记录芯片上电启动、系统控制寄存器保护机制、时钟树配置以及 C2000 独特的三级中断架构。

## 📝 推荐笔记规划

- `01_boot_rom_and_startup.md`
  - 复位向量 $\rightarrow$ Boot ROM $\rightarrow$ `codestart` (`F2837xD_CodeStartBranch.asm`) $\rightarrow$ `_c_int00` $\rightarrow$ `main()` 全流程
  - Boot Mode 引脚配置（Flash 启动、SCI/Wait 启动、RAM 仿真启动）
- `02_clock_tree_and_watchdog.md`
  - 内部振荡器 (`INTOSC1/2`) 与外部晶振 (`XTAL`)
  - SYSPLL 倍频与系统时钟 (`SYSCLK`)、高速外设时钟 (`HISPCP`)、低速外设时钟 (`LOSPCP`) 分频计算
  - 看门狗 (`Watchdog`) 禁用、喂狗与超时复位机制
- `03_register_protection_eallow_edis.md`
  - 为什么关键寄存器修改需要 `EALLOW` / `EDIS` 保护
- `04_pie_interrupt_architecture.md`
  - **三级中断链路**：外设级 (`PIEIER`/`PIEIFR` 前置标志) $\rightarrow$ PIE 控制器 (12 组 $\times$ 8/16 通道向量表) $\rightarrow$ CPU 级 (`IER`, `IFR`, `INTM`)
  - 中断响应流程与退出前必须清除 `PieCtrlRegs.PIEACK.all` 的原因
  - 软件实现中断嵌套的标准模板
