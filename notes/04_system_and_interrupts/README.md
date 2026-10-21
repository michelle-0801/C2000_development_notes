# 04 - 系统控制、时钟与中断体系 (`04_system_and_interrupts`)

本目录记录芯片上电启动、系统控制寄存器保护机制、时钟树配置以及 C2000 独特的三级中断架构。

## 📚 笔记目录

1. **[01_boot_rom_and_startup.md](./01_boot_rom_and_startup.md)**
   - 复位向量 (`0x3FFFC0`) $\rightarrow$ Boot ROM $\rightarrow$ `codestart` (`F2837xD_CodeStartBranch.asm`) $\rightarrow$ `_c_int00` $\rightarrow$ `main()` 全链路解析
   - 独立上电启动引脚配置 (`GPIO72` / `GPIO84`) 与硬件设计防坑指南
2. **[02_clock_tree_and_watchdog.md](./02_clock_tree_and_watchdog.md)**
   - 振荡器源 (`INTOSC1/2`, `XTAL`) 与 `SYSPLL` 倍频分频计算公式（LaunchPad 10MHz vs ControlCARD 20MHz）
   - 三大子时钟域默认频率陷阱（`EPWMCLK = 100MHz`、`LSPCLK = 50MHz`、`ADCCLK <= 50MHz`）
   - 看门狗 `WDCR` 的 `WDCHK (101b)` 防误写机制与 `0x55` + `0xAA` 喂狗序列
3. **[03_register_protection_eallow_edis.md](./03_register_protection_eallow_edis.md)**
   - `ST1.EALLOW` 写保护硬件原理与受保护/豁免寄存器分类速查表
   - 混合编程（DriverLib + Bitfield）下 `EDIS` 提前关闭导致寄存器静默写失败的陷阱
4. **[04_pie_interrupt_architecture.md](./04_pie_interrupt_architecture.md)**
   - **三级中断链路**：外设级 $\rightarrow$ PIE 控制器 (12 组 $\times$ 16 通道) $\rightarrow$ CPU 级 (`IER`, `IFR`, `INTM`)
   - 排查“中断只进一次就再也不进”的黄金清单與软件中断嵌套标准模板
