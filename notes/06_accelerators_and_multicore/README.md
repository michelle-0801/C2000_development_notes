# 06 - 协处理器、DMA 与双核通信 (`06_accelerators_and_multicore`)

本目录聚焦 F2837xD 等高端 C2000 芯片的并行加速架构：CLA 协处理器、DMA 数据通道以及 CPU1/CPU2 双核协同。

## 📚 笔记目录

1. **[01_cla_control_law_accelerator.md](./01_cla_control_law_accelerator.md)**
   - CLA 硬件直连 ADC 与 ePWM 的零中断控制环路架构
   - `.cla` 编程四大限制（16-bit 指针大小差异、`int` 为 32 位、仅限访问 `LSx` 与 `MsgRAM`）
   - CLA `.cmd` 配置、主 CPU 初始化黄金五步法与 CCS `__mdebugstop()` 单步调试技巧
2. **[02_dma_controller.md](./02_dma_controller.md)**
   - DMA 仅限访问 `GS0~GS15` 与外设寄存器的硬件总线限制
   - 二维循环寻址机制（内层 `Burst` + 外层 `Transfer` 正负步进计算）与多通道 ADC 自动解交织实战代码
3. **[03_dual_core_cpu1_cpu2_and_ipc.md](./03_dual_core_cpu1_cpu2_and_ipc.md)**
   - 主从双核（CPU1 Master + CPU2 Slave）上电引导与资源分配全时序图
   - 双核开发四大铁律（`GpioCtrlRegs` 仅限 CPU1、外设移交后由 CPU2 开时钟、脱机 `Device_bootCPU2`）与 IPC 握手/消息通信模板
