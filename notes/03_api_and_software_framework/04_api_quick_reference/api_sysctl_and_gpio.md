# API 速查 01 - 系统控制 (`SysCtl`)、GPIO 与 X-BAR 路由

> **头文件**：`#include "sysctl.h"`, `#include "gpio.h"`, `#include "xbar.h"`（或统一 `#include "driverlib.h"`）

---

## 1. 系统控制与时钟管理 (`SysCtl_*`)

### 1.1 核心 API 速查表

| 函数签名 | 功能说明与常用参数示例 |
| :--- | :--- |
| `SysCtl_setClock(uint32_t config)` | 配置 PLL 锁相环与系统主频 `SYSCLK`。通常直接传入 `device.h` 定义好的 `DEVICE_SETCLOCK_CFG`（200 MHz）。 |
| `SysCtl_getClock(uint32_t clockInHz)` | 根据外部晶振频率返回当前实际运行的 `SYSCLK` 频率（Hz），如 `SysCtl_getClock(DEVICE_OSCSRC_FREQ)`。 |
| `SysCtl_setLowSpeedClock(SysCtl_LSPCLKPrescaler prescaler)` | 配置低速外设时钟 `LSPCLK`（供 SCI/SPI 使用）的分频系数，默认 `SYSCTL_LSPCLK_PRESCALE_4`（即 200MHz / 4 = 50MHz）。 |
| `SysCtl_setEPWMClockDivider(SysCtl_EPWMCLKDivider divider)` | 配置 ePWM 模块输入时钟分频：`SYSCTL_EPWMCLK_DIV_1`（200MHz）或 `SYSCTL_EPWMCLK_DIV_2`（默认 100MHz）。 |
| `SysCtl_enablePeripheral(SysCtl_PeripheralPCLOCKCR peripheral)` | 开启指定外设时钟门控（如 `SYSCTL_PERIPH_CLK_EPWM1`, `SYSCTL_PERIPH_CLK_ADCA`）。 |
| `SysCtl_disablePeripheral(SysCtl_PeripheralPCLOCKCR peripheral)` | 关闭外设时钟。**极其重要用法**：`SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC)` 用于暂停所有 ePWM 时基计数以实现多路完美同步初始化！ |
| `SysCtl_disableWatchdog(void)` | 关闭看门狗定时器（上电初始化必调）。 |
| `SysCtl_serviceWatchdog(void)` | 喂狗（写入 `0x55` + `0xAA` 序列）。 |
| `SysCtl_resetDevice(void)` | 触发软件看门狗立即复位芯片。 |
| `SysCtl_delay(uint32_t count)` | 汇编循环精确延时，实际耗费周期数 $\approx 3 \times \text{count} + 9$。常用封装宏：`DEVICE_DELAY_US(us)`。 |

---

## 2. 通用输入输出 (`GPIO_*`)

在 C2000 中，配置一个引脚作为普通 GPIO 或外设复用引脚，标准步骤为四步：

### 2.1 核心 API 速查表

| 函数签名 | 功能说明与常用参数示例 |
| :--- | :--- |
| `GPIO_setPinConfig(uint32_t pinConfig)` | **设置引脚复用功能 (Mux)**。参数宏定义在 `pin_map.h` 中，格式为 `GPIO_<引脚号>_<功能名>`（如 `GPIO_0_EPWM1A`, `GPIO_28_SCIRXDA`, `GPIO_31_GPIO31`）。 |
| `GPIO_setDirectionMode(uint32_t pin, GPIO_Direction pinIO)` | 设置输入/输出方向：`GPIO_DIR_MODE_IN` 或 `GPIO_DIR_MODE_OUT`。 |
| `GPIO_setPadConfig(uint32_t pin, uint32_t pinType)` | 配置上下拉与驱动模式：`GPIO_PIN_TYPE_STD`（浮空推挽）、`GPIO_PIN_TYPE_PULLUP`（内部上拉）、`GPIO_PIN_TYPE_OD`（开漏）、`GPIO_PIN_TYPE_INVERT`（输入反相）。 |
| `GPIO_setQualificationMode(uint32_t pin, GPIO_QualificationMode qualification)` | **设置输入采样滤波模式**：<br/>- `GPIO_QUAL_SYNC`（与 SYSCLK 同步，默认）<br/>- `GPIO_QUAL_3SAMPLE` / `GPIO_QUAL_6SAMPLE`（3/6次采样窗硬件消抖）<br/>- `GPIO_QUAL_ASYNC`（异步直通，**SPI / SCI / eQEP / ePWM 引脚必选**） |
| `GPIO_setQualificationPeriod(uint32_t pin, uint32_t divider)` | 设置某组（每 8 个引脚共享）的采样窗分频周期（`1 ~ 510` 倍 `SYSCLK`）。 |
| `GPIO_setMasterCore(uint32_t pin, GPIO_CoreSelect core)` | 分配引脚主控权：`GPIO_CORE_CPU1`, `GPIO_CORE_CPU1_CLA1`, `GPIO_CORE_CPU2`, `GPIO_CORE_CPU2_CLA1`。 |
| `GPIO_writePin(uint32_t pin, uint32_t outVal)` | 输出高 (`1U`) 或低 (`0U`)（`static inline`，内部操作 `GPASET`/`GPACLEAR` 寄存器，原子操作无竞争隐患！）。 |
| `GPIO_togglePin(uint32_t pin)` | 翻转指定 GPIO 输出电平（内部写 `GPATOGGLE`）。 |
| `GPIO_readPin(uint32_t pin)` | 读取指定 GPIO 当前电平状态（返回 `0` 或 `1`）。 |

### 2.2 标准配置代码范例

```c
// 范例 1：配置 GPIO31 为推挽输出（驱动板载蓝灯 LED）
GPIO_setPinConfig(GPIO_31_GPIO31);
GPIO_setPadConfig(31U, GPIO_PIN_TYPE_STD);
GPIO_setDirectionMode(31U, GPIO_DIR_MODE_OUT);
GPIO_writePin(31U, 1U); // 初始置高（熄灭）

// 范例 2：配置 GPIO0 为 ePWM1A 输出
GPIO_setPinConfig(GPIO_0_EPWM1A);
GPIO_setPadConfig(0U, GPIO_PIN_TYPE_STD);
GPIO_setQualificationMode(0U, GPIO_QUAL_ASYNC);
```

---

## 3. 信号内部交叉开关路由 (`XBAR_*`)

X-BAR 允许把任意 GPIO 引脚或内部比较器（CMPSS）信号路由到外部中断（XINT）、eCAP、ADC 触发或 ePWM 封波（Trip）输入。

| 函数签名 | 功能说明 |
| :--- | :--- |
| `XBAR_setInputPin(XBAR_InputNum input, uint16_t pin)` | **Input X-BAR**：将任意 `pin`（`0~168`）路由到 `XBAR_INPUT1` ~ `XBAR_INPUT14`。<br/>- 注意：`INPUT1~3` 映射到外部中断 `XINT1~3`<br/>- `INPUT1~6` 可作为 ePWM 的 `TZ1~TZ3` 外部故障封波源<br/>- `INPUT5` 是外部 ADC 触发源 `ADCEXTSOC` |
| `XBAR_setEPWMMuxConfig(XBAR_TripNum trip, XBAR_EPWMMuxConfig muxConfig)` | **ePWM X-BAR**：将内部信号（如 `XBAR_EPWM_MUX00_CMPSS1_CTRIPH` 比较器1高阈值触发）配置到 `XBAR_TRIP4/5/7/8/9/10/11/12` 通道。 |
| `XBAR_enableEPWMMux(XBAR_TripNum trip, uint32_t muxes)` | 使能上面配置好的 ePWM X-BAR 复用开关（如 `XBAR_MUX00`）。 |
| `XBAR_setOutputMuxConfig(...)` / `XBAR_enableOutputMux(...)` | **Output X-BAR**：将内部信号（如 CMPSS 输出、ADC 事件）引出到指定外部 GPIO 管脚供示波器观测。 |
