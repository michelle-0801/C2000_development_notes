# 03 - 寄存器写保护机制：`EALLOW` 与 `EDIS`

> **关联手册**：*TMS320F2837xD Technical Reference Manual (`SPRUHM8K`)* 第 3.3.2 节 "Register Protection (EALLOW)"

在 C2000 中，为了防止程序在强电磁干扰下指针跑飞意外篡改系统时钟、引脚复用、PIE 中断向量表或 ePWM 封波保护配置，硬件设计了 **EALLOW 写保护机制**。

---

## 1. 工作原理

CPU 状态寄存器 `ST1` 的 Bit 6 是 **`EALLOW` 标志位**：
- **芯片复位后默认 `ST1.EALLOW = 0`（锁定状态）**：此时 CPU 尝试向任何受保护的寄存器地址执行写操作，**硬件会直接忽略该写入（静默失败，不会报错）**，但允许正常读取。
- **执行 `EALLOW;` 宏（汇编指令 `EALLOW`）**：将 `ST1.EALLOW` 置 `1`，解锁受保护寄存器的写权限。
- **执行 `EDIS;` 宏（汇编指令 `EDIS`）**：将 `ST1.EALLOW` 清 `0`，重新上锁。

> [!TIP]
> **怎么知道某个寄存器是否需要 `EALLOW`？**
> 在阅读 TRM 手册（`spruhm8k.pdf`）各章节的 Register Summary 表格时，表头或寄存器说明底部凡是标注了 **`Write-Protection: EALLOW`** 的寄存器，修改前都必须解锁！

---

## 2. 常见受 `EALLOW` 保护的寄存器清单

| 所属模块 | 受保护的典型寄存器 | 不受保护（可在 ISR 直接改写）的典型寄存器 |
| :--- | :--- | :--- |
| **系统与时钟 (`CpuSysRegs` / `ClkCfgRegs`)** | 几乎全部（PLL、外设时钟门控 `PCLKCRx`、看门狗 `WDCR`/`WDKEY`） | 无 |
| **GPIO (`GpioCtrlRegs` vs `GpioDataRegs`)** | **`GpioCtrlRegs` 全部受保护**（复用 `GPxMUX`、方向 `GPxDIR`、上拉 `GPxPUD`） | **`GpioDataRegs` 不受保护**（`GPxSET`, `GPxCLEAR`, `GPxTOGGLE`, `GPxDAT`） |
| **PIE 中断 (`PieVectTable` / `PieCtrlRegs`)** | **`PieVectTable` 全部中断向量地址** | `PieCtrlRegs`（如 `PIEACK`, `PIEIERx`）**不受保护** |
| **ePWM (`EPwm1Regs`)** | **`TZSEL`, `TZCTL`, `TZEINT`, `TZCLR`, `DCTRIPSEL` 等故障保护寄存器**及 `HRCNFG` | **`TBPRD`, `CMPA`, `CMPB`, `AQCTLA`, `DBRED`, `ETCLR` 均不受保护**！ |
| **ADC (`AdcaRegs` vs `AdcaResultRegs`)** | **`AdcaRegs` 几乎全部配置寄存器**（包括 `ADCSOCxCTL` 甚至 **`ADCINTFLGCLR` 清中断标志寄存器**！） | **`AdcaResultRegs`（结果只读）** |
| **内存与双核 (`MemCfgRegs` / `DevCfgRegs`)** | 全部受保护（`LSxMSEL`, `GSxMSEL`, `CPUSELx`） | 无 |

> [!WARNING]
> **极易踩坑细节：在 Bitfield 模式下清除 ADC 中断标志需要 `EALLOW`？**
> 实际上在 F2837xD 中，大部分 `AdcaRegs` 配置寄存器受 `EALLOW` 保护，唯有 `ADCINTFLGCLR`、`ADCINTOVFCLR` 和 `ADCSOCFRC1` 这几个运行期频繁触发/清除寄存器**豁免了 `EALLOW` 保护**；但 **ePWM 的清除故障标志寄存器 `EPwm1Regs.TZCLR.all` 是受 `EALLOW` 保护的**！如果不写 `EALLOW` 直接写 `EPwm1Regs.TZCLR.bit.OST = 1;`，故障永远清不掉！

---

## 3. 中断打断引发的 `EALLOW` 状态丢失隐患

### 3.1 硬件自动保护机制
当主循环正在 `EALLOW` 和 `EDIS` 之间修改受保护寄存器时，如果恰好来了高优先级中断（ISR）：
1. **进入 ISR 时**：CPU 硬件自动把当前的 `ST1`（包含 `EALLOW = 1` 的状态）压入堆栈（Stack），然后**硬件自动将 `ST1.EALLOW` 清零**（确保 ISR 默认在安全锁定状态下运行）。
2. **退出 ISR (`IRET`) 时**：CPU 硬件从堆栈弹出恢复原来的 `ST1`，主循环的 `EALLOW = 1` 状态得以自动恢复！

### 3.2 混合编程（Bitfield + DriverLib）下的陷阱
前面提到，**DriverLib 的每个受保护 API 函数内部末尾都会无条件执行 `EDIS;`**！
因此，绝对不要写出下面这样的代码：

```c
// ❌ 错误示范：
EALLOW;
GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;           // 成功写入 (EALLOW=1)
SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM1); // 内部执行了 EALLOW -> 写寄存器 -> EDIS! 此时 EALLOW 已经变成 0 了！
GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;           // ❌ 静默失败！因为上一行 DriverLib 函数把 EALLOW 关掉了！
EDIS;
```
**正确规范**：每一段直接操作受保护 Bitfield 寄存器的代码块，紧贴着它自己的 `EALLOW;` 和 `EDIS;`，中间不要穿插调用任何子函数或 DriverLib API。
