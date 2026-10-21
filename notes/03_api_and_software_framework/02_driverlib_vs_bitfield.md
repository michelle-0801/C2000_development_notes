# 02 - 编程模型对比：DriverLib API vs Bitfield 寄存器位域 vs 混合模式

在 C2000 开发中，存在三种操作硬件外设的方式：**Bitfield（传统寄存器结构体位域）**、**DriverLib（现代函数式驱动库 API）** 以及两者结合的 **Hybrid（混合编程模式）**。

---

## 1. 三种外设访问方式直观对比

假设我们要将 `ePWM1` 的时基周期设为 `1000`，并将计数模式设为增减计数（Up-Down Count）：

```c
// ==================== 方式 1：Bitfield (寄存器结构体位域) ====================
EPwm1Regs.TBPRD = 1000U;
EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

// ==================== 方式 2：DriverLib API (现代标准函数库) ====================
EPWM_setTimeBasePeriod(EPWM1_BASE, 1000U);
EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP_DOWN);

// ==================== 方式 3：HWREGH 宏 (DriverLib 底层展开) ====================
HWREGH(EPWM1_BASE + EPWM_O_TBPRD) = 1000U;
HWREGH(EPWM1_BASE + EPWM_O_TBCTL) = (HWREGH(EPWM1_BASE + EPWM_O_TBCTL) & ~0x3U) | 0x2U;
```

### 1.1 全面特性与性能对比表

| 评估维度 | Bitfield (`EPwm1Regs.xxx`) | DriverLib (`EPWM_xxx(BASE, ...)`) | 底层机制说明 |
| :--- | :--- | :--- | :--- |
| **底层实现原理** | 定义 `volatile struct EPWM_REGS`，通过 `F2837xD_GlobalVariableDefs.c` 和 `Headers_nonBIOS.cmd` 强制绑定到外设物理地址 | 基于外设基地址宏（如 `EPWM1_BASE = 0x00004000U`）加偏移量，使用 `HWREG()` (32-bit) / `HWREGH()` (16-bit) 指针强转访问 | DriverLib 不依赖专门的外设 `.cmd` 文件 |
| **与 TRM 手册对应度** | ⭐⭐⭐⭐⭐（寄存器名与字段名同 `SPRUHM8K` 完全一致） | ⭐⭐⭐⭐（函数语义清晰，但需查 API 名字） | 初学者看 TRM 时 Bitfield 极直观 |
| **代码复用与多通道扩展** | ⭐⭐（`EPwm1Regs`、`EPwm2Regs` 是不同全局变量，写通用函数需传结构体指针） | ⭐⭐⭐⭐⭐（只需把基地址 `uint32_t base` 作为参数传入即可操作任意通道！） | 写三相 PWM 或多通道驱动时 DriverLib 极优雅 |
| **参数防呆检查** | ❌（写错枚举值编译不报错） | ⭐⭐⭐⭐⭐（内置 `ASSERT(EPWM_isBaseValid(base))`） | 开启 `DEBUG` 宏时传错参数直接停在断点 |
| **SysConfig 图形化支持** | ❌ 不支持 | ✅ 原生支持（自动生成 DriverLib 初始化代码） | 新项目及 F28004x/28003x/28P65x 均主推 DriverLib |
| **执行效率 (ISR 内)** | ⭐⭐⭐⭐⭐（单条 `MOV` 或位操作指令） | ⭐⭐⭐⭐⭐（**90% 的高频读写 API 在 `.h` 中声明为 `static inline`**，关闭 `DEBUG` 后编译生成的汇编与手写寄存器完全一样！） | 见下方详解 |

---

## 2. 破除迷思：DriverLib 函数调用真的比直接写寄存器慢吗？

很多老工程师担心在 50kHz 的控制中断（ISR）里调用 `ADC_readResult()` 或 `EPWM_setCounterCompareValue()` 会产生函数跳转（`LCR` / `LRETR`）开销。
实际上，打开 `driverlib/adc.h` 和 `driverlib/epwm.h` 源码可以看到：

```c
// 摘自 driverlib/adc.h：声明为 static inline！
static inline uint16_t
ADC_readResult(uint32_t resultBase, ADC_SOCNumber socNumber)
{
    ASSERT(ADC_isResultBaseValid(resultBase));
    return(HWREGH(resultBase + (uint32_t)ADC_RESULTx_OFFSET_BASE + (uint32_t)socNumber));
}
```
- **在高频 ISR 中使用的读写函数**（如 `ADC_readResult`、`ADC_clearInterruptStatus`、`EPWM_setCounterCompareValue`、`GPIO_writePin`、`Interrupt_clearACKGroup`）**全部是定义在头文件中的 `static inline` 内联函数**！
- 当传入的 `resultBase` 和 `socNumber` 是常量宏（且去掉 `DEBUG` 或开启 `-O2` 优化）时，编译器在编译期就直接算出了最终物理地址，**生成 1 条 `MOV AL, *(0x0B00)` 汇编指令，耗时仅 1 个周期，与 `AdcaResultRegs.ADCRESULT0` 0 差别**！
- 只有复杂的上电一次性初始化函数（如 `SysCtl_setClock`、`EPWM_setEmulationMode` 等位于 `.c` 中的函数）才是普通函数调用。

---

## 3. 最佳实践：如何在工程中启用“混合编程模式” (Bitfield + DriverLib)

如果你想在同一个工程里：**用 DriverLib（或 SysConfig）做外设初始化，同时在代码里也能随时直接敲 `EPwm1Regs.` 或 `AdcaResultRegs.` 看寄存器**：

### 步骤 1：在工程中加入 Bitfield 映射文件
从 `device_support/f2837xd/` 中将以下两个文件加入工程（或建立链接）：
1. `headers/source/F2837xD_GlobalVariableDefs.c`（定义了 `EPwm1Regs` 等全局结构体实例，并通过 `#pragma DATA_SECTION` 放入外设段）。
2. `headers/cmd/F2837xD_Headers_nonBIOS_cpu1.cmd`（将外设段映射到 `0x0000 0B00` ~ `0x0005 FFFF` 的硬件寄存器地址）。

### 步骤 2：在源码中包含两个头文件（注意防止宏名冲突！）
由于 Bitfield 头文件和 DriverLib 头文件中有少量同名枚举/宏（如 CAN 模块），TI 官方专门提供了兼容宏——只需在包含头文件前或工程预定义宏中添加 **`_DUAL_HEADERS`**（或在包含 `F2837xD_device.h` 与 `driverlib.h` 时直接使用 `device.h`）：

```c
#include "driverlib.h"
#include "device.h"
#include "F2837xD_device.h" // 引入 Bitfield 寄存器结构体定义

void example_HybridUsage(void)
{
    // 1. 使用 DriverLib API 开启时钟并配置周期（可读性高、带断言检查）
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM1);
    EPWM_setTimeBasePeriod(EPWM1_BASE, 2500U);

    // 2. 同时也可以直接通过 Bitfield 结构体访问任意底层寄存器位！
    EPwm1Regs.CMPA.bit.CMPA = 1250U;
}
```
> [!TIP]
> **注意 `EALLOW` / `EDIS` 的差异**：
> - **DriverLib API 内部自带 `EALLOW` 和 `EDIS`**，调用完一个受保护的 DriverLib 函数后，`EALLOW` 会被该函数内部的 `EDIS` 关闭！
> - 因此在混合编程时，如果你先写了 `EALLOW;`，中间调用了一个 DriverLib 函数，后面接着写受保护的 `Bitfield` 寄存器，必须重新补一句 `EALLOW;`！
