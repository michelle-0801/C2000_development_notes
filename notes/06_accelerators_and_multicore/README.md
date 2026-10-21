# 06 - 协处理器、DMA 与双核通信 (`06_accelerators_and_multicore`)

本目录聚焦 F2837xD 等高端 C2000 芯片的并行加速架构：CLA 协处理器、DMA 数据通道以及 CPU1/CPU2 双核协同。

## 📝 推荐笔记规划

- `01_cla_control_law_accelerator.md`
  - CLA 独立流水线特点、支持的数据类型与指令限制（`.cla` 文件编写规范）
  - Task 1~8 硬件触发源配置（如 ADC EOC 直接触发 CLA 执行闭环控制律）
  - CPU 与 CLA 的内存共享配置（`LSx` 划归 CLA 程序/数据空间、`CpuToCla1MsgRAM` / `Cla1ToCpuMsgRAM`）
- `02_dma_controller.md`
  - 6 通道 DMA 架构、Burst 与 Transfer 两级循环计数机制
  - 典型场景：ADC 结果自动搬运至 `GSx` RAM、McBSP/SPI 高速通信数据搬运
- `03_dual_core_cpu1_cpu2_and_ipc.md`
  - **主从上电引导**：CPU1 如何通过 IPC 命令引导 CPU2 从 Flash 或 RAM 启动
  - **外设与 GPIO 归属权分配**：CPU1 通过 `DevCfgRegs.CPUSELx` 与 `GPIO_setMasterCore()` 将外设移交给 CPU2
  - **IPC 核间通信**：IPC 标志位 (`IPCSET`/`IPCCLR`/`IPCFLG`)、IPC 消息队列与 `GSx` 共享内存并发访问规范
