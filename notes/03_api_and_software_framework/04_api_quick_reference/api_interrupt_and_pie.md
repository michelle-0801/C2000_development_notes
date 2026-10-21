# API 速查 02 - PIE 中断管理 (`Interrupt`) 与 CPU 定时器 (`CPUTimer`)

> **头文件**：`#include "interrupt.h"`, `#include "cputimer.h"`

---

## 1. 中断与 PIE 向量表管理 (`Interrupt_*`)

C2000 的中断由 **外设级 $\rightarrow$ PIE 级（12 组，编号 `INT_xxx`）$\rightarrow$ CPU 级（`IER` & `INTM`）** 组成。DriverLib 的 `Interrupt_*` 模块将 PIE 和 CPU `IER` 封装在了一起！

### 1.1 核心 API 速查表

| 函数签名 | 功能说明 |
| :--- | :--- |
| `Interrupt_initModule(void)` | 初始化 PIE 控制器：禁用所有 CPU 中断 (`DINT`)、清空 `IER`/`IFR` 并清零所有 PIE 组使能与标志。 |
| `Interrupt_initVectorTable(void)` | 将整个 PIE 向量表初始化指向默认的空错误处理函数（防止意外开启未注册中断导致跑飞），并使能 PIE 向量表映射 (`ENPIE = 1`)。 |
| `Interrupt_register(uint32_t interruptNumber, void (*isr)(void))` | 将自定义的 `__interrupt void myISR(void)` 函数指针写入指定中断号（如 `INT_ADCA1`, `INT_EPWM1`, `INT_TIMER0`）的 PIE 向量表槽位。 |
| `Interrupt_enable(uint32_t interruptNumber)` | **同时使能**该中断对应的 **PIE 组内通道 (`PIEIERx.INTy`)** 和 **CPU 级中断组 (`IER |= M_INTx`)**！ |
| `Interrupt_disable(uint32_t interruptNumber)` | 禁用指定外设在 PIE 中的中断通道。 |
| `Interrupt_clearACKGroup(uint16_t group)` | **在每个 PIE 中断服务函数 (`ISR`) 退出前必调**！清除对应组的应答锁存位（如 `INTERRUPT_ACK_GROUP1`, `INTERRUPT_ACK_GROUP3`），允许同组后续中断再次进入 CPU。 |
| `Interrupt_enableMaster(void)` / `EINT` | 开启 CPU 全局中断总开关（清除 `ST1.INTM` 位，等同于宏 `EINT;`）。 |
| `Interrupt_disableMaster(void)` / `DINT` | 关闭 CPU 全局中断总开关（置位 `ST1.INTM` 位，等同于宏 `DINT;`）。 |

> [!TIP]
> **中断号宏定义 (`INT_xxx`) 如何查找？**
> 所有中断号宏均定义在 `driverlib/inc/hw_ints.h` 中，并且宏的值高 16 位直接编码了它所属的 PIE 组号！常用中断与所属 `ACK_GROUP` 对应关系：
> - **`INT_TIMER0`、`INT_ADCA1`、`INT_ADCB1`、`INT_XINT1`/`2`** $\longrightarrow$ **`INTERRUPT_ACK_GROUP1`**
> - **`INT_EPWM1_TZ` ~ `INT_EPWM12_TZ`（封波保护中断）** $\longrightarrow$ **`INTERRUPT_ACK_GROUP2`**
> - **`INT_EPWM1` ~ `INT_EPWM12`（周期/事件触发中断）** $\longrightarrow$ **`INTERRUPT_ACK_GROUP3`**
> - **`INT_ECAP1` ~ `INT_ECAP6`** $\longrightarrow$ **`INTERRUPT_ACK_GROUP4`**
> - **`INT_EQEP1` ~ `INT_EQEP3`** $\longrightarrow$ **`INTERRUPT_ACK_GROUP5`**
> - **`INT_SPIA_RX`/`TX`、`INT_DMA_CH1` ~ `CH6`** $\longrightarrow$ **`INTERRUPT_ACK_GROUP6` / `GROUP7`**
> - **`INT_SCIA_RX`/`TX`** $\longrightarrow$ **`INTERRUPT_ACK_GROUP9`**

---

## 2. CPU 32位定时器 (`CPUTimer_*`)

每个 C28x CPU 内核自带 3 个独立的 32 位递减定时器：**`CPUTIMER0_BASE`**、**`CPUTIMER1_BASE`**、**`CPUTIMER2_BASE`**。
- `TIMER0` 经过 PIE Group 1 (`INT_TIMER0`)，**必须调 `Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1)`**。
- `TIMER1` (`INT_TIMER1`) 和 `TIMER2` (`INT_TIMER2`) **直接连入 CPU `INT13` 和 `INT14`，不经过 PIE 控制器**，因此它们的 ISR 里**不需要**调 `Interrupt_clearACKGroup()`（常留给 RTOS 系统滴答或低优先级后台调度）。

### 2.1 核心 API 速查表

| 函数签名 | 功能说明 |
| :--- | :--- |
| `CPUTimer_setPeriod(uint32_t base, uint32_t periodCount)` | 设置 32 位重装载周期寄存器 `PRD`。 |
| `CPUTimer_setPreScale(uint32_t base, uint16_t prescaler)` | 设置 16 位预分频器 `TPR`（实际分频数为 `prescaler + 1`）。 |
| `CPUTimer_stopTimer(uint32_t base)` / `CPUTimer_startTimer(uint32_t base)` | 暂停 (`TSS=1`) 或启动 (`TSS=0`) 定时器。 |
| `CPUTimer_reloadTimerCounter(uint32_t base)` | 立即将 `PRD` 的值重装入计数器 `TIM` (`TRB=1`)。 |
| `CPUTimer_enableInterrupt(uint32_t base)` | 使能定时器溢出中断 (`TIE=1`)。 |
| `CPUTimer_getTimerCount(uint32_t base)` | 读取当前 32 位递减计数值（常用于精确测量某段代码的执行耗时周期数！）。 |

### 2.2 标准代码范例：配置 CPU Timer 0 产生 1ms (1kHz) 周期中断

```c
void initCPUTimer0_1ms(void)
{
    // 1. 注册 ISR 到 PIE 向量表
    Interrupt_register(INT_TIMER0, &cpuTimer0ISR);

    // 2. 停止定时器并配置周期：200MHz 下 1ms = 200,000 个周期
    CPUTimer_stopTimer(CPUTIMER0_BASE);
    CPUTimer_setPreScale(CPUTIMER0_BASE, 0U);
    CPUTimer_setPeriod(CPUTIMER0_BASE, (uint32_t)(DEVICE_SYSCLK_FREQ / 1000U) - 1U);
    CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);
    CPUTimer_setEmulationMode(CPUTIMER0_BASE, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);

    // 3. 开启定时器中断与 PIE 通道，并启动计数
    CPUTimer_enableInterrupt(CPUTIMER0_BASE);
    Interrupt_enable(INT_TIMER0);
    CPUTimer_startTimer(CPUTIMER0_BASE);
}

__interrupt void cpuTimer0ISR(void)
{
    // 1ms 周期任务...

    // 必须清除 PIE Group 1 应答位！
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}
```
