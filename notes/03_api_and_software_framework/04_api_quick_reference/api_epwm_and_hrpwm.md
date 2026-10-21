# API 速查 03 - 增强型脉宽调制 (`EPWM`) 核心 API 字典

> **头文件**：`#include "epwm.h"`

ePWM 模块由 7 大子模块组成，DriverLib 的 `EPWM_*` API 命名严格遵循各个子模块的前缀，极易记忆和检索。

---

## 1. 按子模块分类的核心 API 速查表

### 1.1 时基子模块 (Time-Base, `TB`)
| 函数签名 | 功能说明与常用枚举 |
| :--- | :--- |
| `EPWM_setClockPrescaler(base, prescaler, highSpeedPrescaler)` | 设置 `TBCLK` 相对于 `EPWMCLK`（默认 100MHz）的两级分频：<br/>- `EPWM_CLOCK_DIVIDER_1` ~ `128`<br/>- `EPWM_HSCLOCK_DIVIDER_1` ~ `14` |
| `EPWM_setTimeBasePeriod(base, periodCount)` | 设置周期寄存器 `TBPRD`（增减计数模式下 $T_{\text{pwm}} = 2 \times \text{TBPRD} \times T_{\text{tbclk}}$）。 |
| `EPWM_setTimeBaseCounterMode(base, counterMode)` | 计数模式：`EPWM_COUNTER_MODE_UP`（增）、`EPWM_COUNTER_MODE_DOWN`（减）、`EPWM_COUNTER_MODE_UP_DOWN`（对称中心对齐）、`EPWM_COUNTER_MODE_STOP_FREEZE`。 |
| `EPWM_setPeriodLoadMode(base, loadMode)` | `TBPRD` 影子加载模式：`EPWM_PERIOD_SHADOW_LOAD`（默认在 `TBCTR=0` 更新）或 `EPWM_PERIOD_DIRECT_LOAD`。 |
| `EPWM_enablePhaseShiftLoad(base)` / `EPWM_setPhaseShift(base, phaseCount)` | 使能外部同步相移装载并写入相位偏移值 `TBPHS`（多相交错并联/移相全桥核心 API）。 |
| `EPWM_setSyncOutPulseMode(base, mode)` | 设置向下游 ePWM 输出同步脉冲的时机（如 `EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO`）。 |

### 1.2 计数比较子模块 (Counter-Compare, `CC`)
| 函数签名 | 功能说明与常用枚举 |
| :--- | :--- |
| `EPWM_setCounterCompareShadowLoadMode(base, compModule, loadMode)` | 配置 `CMPA`/`CMPB`/`CMPC`/`CMPD` 的影子寄存器重载时机：<br/>- `EPWM_COMP_LOAD_ON_CNTR_ZERO`（过零更新）<br/>- `EPWM_COMP_LOAD_ON_CNTR_PERIOD`（顶点更新）<br/>- `EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD`（双更新模式，常用于高频控制） |
| `EPWM_setCounterCompareValue(base, compModule, compCount)` | **ISR 内最高频调用 API (`static inline`)**：更新比较值（如 `EPWM_COUNTER_COMPARE_A`, `dutyTicks`）。 |

### 1.3 动作限定子模块 (Action-Qualifier, `AQ`)
| 函数签名 | 功能说明与常用枚举 |
| :--- | :--- |
| `EPWM_setActionQualifierAction(base, epwmOutput, outputAction, event)` | 单事件动作配置：<br/>- 输出通道：`EPWM_AQ_OUTPUT_A` / `EPWM_AQ_OUTPUT_B`<br/>- 动作：`EPWM_AQ_OUTPUT_HIGH` / `LOW` / `TOGGLE` / `NO_CHANGE`<br/>- 事件：`EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA`、`DOWN_CMPA`、`ZERO`、`PERIOD` 等 |
| `EPWM_setActionQualifierContSWForceAction(base, epwmOutput, output)` | **软件连续强制封波/置电平**（操作 `AQCSFRC` 寄存器，常用于软件启停逆变桥）。 |

### 1.4 死区子模块 (Dead-Band, `DB`)
| 函数签名 | 功能说明与常用枚举 |
| :--- | :--- |
| `EPWM_setDeadBandDelayMode(base, delayMode, enableDelay)` | 使能上升沿死区 (`EPWM_DB_RED`) 和下降沿死区 (`EPWM_DB_FED`)。 |
| `EPWM_setDeadBandDelayPolarity(base, delayMode, polarity)` | 设置死区极性。**生成标准高有效互补对称波形 (`AHC`)**：<br/>- `EPWM_DB_RED` 设为 `EPWM_DB_POLARITY_ACTIVE_HIGH`<br/>- `EPWM_DB_FED` 设为 `EPWM_DB_POLARITY_ACTIVE_LOW`（将 B 通道反相！） |
| `EPWM_setDeadBandInputMode(base, delayMode, inputMode)` | 选择死区输入源，通常 RED 与 FED 均选 `EPWM_DB_INPUT_EPWMA`（仅需配置 ePWMxA 的 AQ，自动生成互补带死区的 ePWMxA/B！）。 |
| `EPWM_setRisingEdgeDelayCount(base, redCount)` / `EPWM_setFallingEdgeDelayCount(base, fedCount)` | 设置上升沿 `DBRED` 与下降沿 `DBFED` 死区计数值（14-bit）。 |

### 1.5 事件触发子模块 (Event-Trigger, `ET` —— 触发 ADC 与 CPU 中断)
| 函数签名 | 功能说明与常用枚举 |
| :--- | :--- |
| `EPWM_enableADCTrigger(base, adcTriggerType)` | 使能 `EPWM_SOC_A` 或 `EPWM_SOC_B` 向 ADC 发送采样启动脉冲。 |
| `EPWM_setADCTriggerSource(base, adcTriggerType, socSource)` | 设置触发时机（如 `EPWM_SOC_TBCTR_ZERO` 在波谷采样，或 `EPWM_SOC_TBCTR_PERIOD` 在波峰采样以避开开关管动作噪声）。 |
| `EPWM_setADCTriggerEventPrescale(base, adcTriggerType, preScaleCount)` | 每 `1 ~ 15` 个 PWM 周期触发 1 次 ADC（支持高频发波、低频控制采样）。 |
| `EPWM_enableInterrupt(base)` / `EPWM_setInterruptSource(base, interruptSource)` | 使能 ePWM 自身向 PIE 发送中断并选择触发点。 |
| `EPWM_clearEventTriggerInterruptFlag(base)` | **在 ePWM ISR 内必调**：清除 `ETCLR.INT` 标志位，否则后续无法再次进入 ePWM 中断！ |

### 1.6 错误联防与数字比较子模块 (Trip-Zone `TZ` & Digital Compare `DC`)
| 函数签名 | 功能说明 |
| :--- | :--- |
| `EPWM_setTripZoneAction(base, tzAdvEvent, tzAction)` | 设置发生硬件故障时的引脚保护动作：通常将 `EPWM_TZ_ACTION_EVENT_TZA` 和 `TZB` 设为 `EPWM_TZ_ACTION_LOW`（瞬间强制拉低上下管关断 IGBT/MOSFET！）。 |
| `EPWM_enableTripZoneSignals(base, tzSignals)` | 使能触发源：`EPWM_TZ_SIGNAL_OSHT1~6`（单次锁死保护 One-Shot，需软件复位）或 `EPWM_TZ_SIGNAL_CBC1~6`（逐周期限流保护 Cycle-by-Cycle）。 |
| `EPWM_forceTripZoneEvent(base, EPWM_TZ_FORCE_EVENT_OST)` | 软件立即强制触发一次 OST 硬件封波保护。 |
| `EPWM_clearTripZoneFlag(base, tzFlags)` | 故障解除后，清除 `EPWM_TZ_FLAG_OST` | `EPWM_TZ_INTERRUPT` 恢复发波。 |

---

## 2. 多路 ePWM 同步初始化的黄金法则 (`TBCLKSYNC`)

当系统中有多路 ePWM（如三相逆变器 `ePWM1`、`ePWM2`、`ePWM3`）需要保持严格的 0° 相位对齐或 120° 交错移相时，**绝不能配完一路就启动一路**，必须按以下三步包裹：

```c
// 1. 【配置前】冻结所有 ePWM 模块的时基时钟同步总开关！
SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

// 2. 依次初始化 ePWM1, ePWM2, ePWM3 的寄存器并将 TBCTR 清零...
initEPWM1();
initEPWM2();
initEPWM3();

// 3. 【配置后】同时合闸开启 TBCLKSYNC，所有 ePWM 计数器在同一时钟沿起跑！
SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
```
