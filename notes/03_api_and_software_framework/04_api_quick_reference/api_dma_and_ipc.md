# API 速查 05 - 内存权限配置 (`MemCfg`)、DMA 与双核通信 (`IPC`)

> **头文件**：`#include "memcfg.h"`, `#include "dma.h"`, `#include "ipc.h"`

---

## 1. 内存块归属权与 CLA/双核映射 (`MemCfg_*`)

在使用 CLA 或双核（CPU1 + CPU2）共享内存前，必须先通过 `MemCfg_*` API 在 **CPU1**（或所属 CPU）中分配 RAM 块的主控权。

| 函数签名 | 功能说明与常用参数 |
| :--- | :--- |
| `MemCfg_setLSRAMMasterSel(ramSection, masterSel)` | 设置 `MEMCFG_SECT_LS0` ~ `LS5` 的主控权：<br/>- `MEMCFG_LSRAMMASTER_CPU_ONLY`（默认仅 CPU 使用）<br/>- `MEMCFG_LSRAMMASTER_CPU_CLA1`（共享给 CLA1 访问） |
| `MemCfg_setCLAMemType(ramSection, claMemType)` | 当某块 `LSx` 共享给 CLA 后，指定它是存放 CLA 代码还是 CLA 数据：<br/>- `MEMCFG_CLA_MEM_PROGRAM`（CLA 程序区，CPU 不可再写）<br/>- `MEMCFG_CLA_MEM_DATA`（CLA 数据区） |
| `MemCfg_setGSRAMMasterSel(ramSection, masterSel)` | **双核必用（仅限 CPU1 调用）**：将 `MEMCFG_SECT_GS0` ~ `GS15`（支持按位或组合如 `MEMCFG_SECT_GS8 | MEMCFG_SECT_GS9`）的主控写权限分配给：<br/>- `MEMCFG_GSRAMMASTER_CPU1`（CPU1 & CPU1.DMA 可读写，CPU2 只读）<br/>- `MEMCFG_GSRAMMASTER_CPU2`（CPU2 & CPU2.DMA 可读写，CPU1 只读） |
| `MemCfg_initSections(ramSections)` | 将指定 RAM 块（如 `MEMCFG_SECT_MSGCLA1TOCPU | MEMCFG_SECT_MSGCPUTOCLA1`）硬件清零初始化，并等待 `MemCfg_getInitStatus()` 返回 `true`。 |

---

## 2. 直接内存访问控制器 (`DMA_*`)

F2837xD 每个核拥有 6 个独立通道（`DMA_CH1_BASE` ~ `DMA_CH6_BASE`）。

| 函数签名 | 功能说明 |
| :--- | :--- |
| `DMA_initController(void)` | 软复位初始化整个 DMA 控制器，并开启自由仿真运行模式。 |
| `DMA_configAddresses(base, *destAddr, *srcAddr)` | 设置通道的激活与影子源地址 (`srcAddr`) 和目的地址 (`destAddr`)（传入 `const void *`）。 |
| `DMA_configBurst(base, size, srcStep, destStep)` | **内层循环 (Burst)**：每次硬件触发搬运 `size` 个 16-bit Word（`1~32`），每搬 1 个 Word 后源地址加 `srcStep`、目的地址加 `destStep`（有符号整数，可正可负可为 0）。 |
| `DMA_configTransfer(base, transferSize, srcStep, destStep)` | **外层循环 (Transfer)**：共需要多少次 Burst 触发才算完成一帧传输（`transferSize` 个 Burst），每次 Burst 完成后源/目的地址的步进量。 |
| `DMA_configMode(base, trigger, config)` | 配置触发源（如 `DMA_TRIGGER_ADCA1`）及工作模式（`DMA_CFG_ONESHOT_DISABLE | DMA_CFG_CONTINUOUS_ENABLE | DMA_CFG_SIZE_16BIT`）。 |
| `DMA_setInterruptMode(base, mode)` / `DMA_enableInterrupt(base)` | 设置中断产生时机（`DMA_INT_AT_BEGINNING` 或 `DMA_INT_AT_END`）并使能通道中断。 |
| `DMA_enableTrigger(base)` / `DMA_startChannel(base)` | 使能外设硬件触发并启动该 DMA 通道进入待命状态。 |

---

## 3. 双核核间通信与同步 (`IPC_*`)

C2000Ware 的 `ipc.h` 将 CPU1 与 CPU2 之间的 32 个硬件标志位（`IPC_FLAG0` ~ `IPC_FLAG31`）、基础命令寄存器和环形消息队列封装成了对称的通信接口（通过通道枚举 `IPC_CPU1_L_CPU2_R` 和 `IPC_CPU2_L_CPU1_R` 区分本地与远端）。

| 函数签名 | 功能说明 |
| :--- | :--- |
| `IPC_sync(ipcType, flag)` | **双核双边握手屏障（最常用！）**：本地置位 `flag`，然后死循环等待对方也置位同一个 `flag`，双方到齐后自动清除标志继续往下走（常用于双核初始化完成同步！）。 |
| `IPC_setFlagLtoR(ipcType, flags)` | 置位本地发往远端的 IPC 标志位（若 `flags` 为 `IPC_FLAG0` ~ `IPC_FLAG3`，同时会触发远端 CPU 的 `INT_IPC_0` ~ `INT_IPC_3` 中断！）。 |
| `IPC_isFlagBusyRtoL(ipcType, flag)` | 查询远端发来的某个 `flag` 是否已被置位。 |
| `IPC_ackFlagRtoL(ipcType, flags)` | **远端接收方处理完数据后调用**：清除（Acknowledge）对方发来的 IPC 标志位，通知发送方可以发送下一帧。 |
| `IPC_waitForAck(ipcType, flag)` | 发送方等待远端调用 `IPC_ackFlagRtoL()` 清除指定标志位。 |
| `IPC_sendCommand(ipcType, flags, addrCorrEnable, command, addr, data)` | 通过硬件 IPC 寄存器发送 32 位命令字 `command`、地址 `addr` 和 32 位数据 `data`，并置位 `flags`。 |
| `IPC_readCommand(ipcType, flags, addrCorrEnable, *command, *addr, *data)` | 接收方从 IPC 寄存器中读取对方发来的命令、地址与数据。 |
