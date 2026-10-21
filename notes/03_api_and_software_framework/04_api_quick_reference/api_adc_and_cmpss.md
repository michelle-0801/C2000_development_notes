# API 速查 04 - 模数转换器 (`ADC`) 与片上比较器 (`CMPSS`)

> **头文件**：`#include "adc.h"`, `#include "cmpss.h"`

F2837xD 拥有 4 个独立的 SAR ADC 模块（`ADCA_BASE`, `ADCB_BASE`, `ADCC_BASE`, `ADCD_BASE`）及对应的结果寄存器基地址（**`ADCARESULT_BASE`** ~ **`ADCDRESULT_BASE`**），外加 8 路带内置 DAC 的高速模拟比较器子系统（`CMPSS1_BASE` ~ `CMPSS8_BASE`）。

---

## 1. ADC 模块核心 API 速查表 (`ADC_*`)

> [!CAUTION]
> **基地址区分陷阱**：
> - **配置类函数**（如 `ADC_setupSOC`、`ADC_setInterruptSource`）第一个参数传控制寄存器基地址：**`ADCA_BASE` (`0x00007400`)**。
> - **读取结果类函数**（如 `ADC_readResult`、`ADC_readPPBResult`）第一个参数必须传**结果寄存器基地址**：**`ADCARESULT_BASE` (`0x00000B00`)**！如果传错，开启 `DEBUG` 时会直接触发 `ASSERT` 停机。

| 函数签名 | 功能说明与常用参数 |
| :--- | :--- |
| `ADC_setPrescaler(base, clkPrescale)` | 设置 `ADCCLK` 相对于 `SYSCLK`（200MHz）的分频。**F2837xD 的 `ADCCLK` 最高不得超过 50 MHz**，因此在 200MHz 主频下必选 **`ADC_CLK_DIV_4_0`**！ |
| `ADC_setMode(base, resolution, signalMode)` | 设置分辨率与输入模式（内部会自动从 OTP 加载工厂校准参数！）：<br/>- 12位单端：`ADC_RESOLUTION_12BIT`, `ADC_MODE_SINGLE_ENDED`<br/>- 16位差分：`ADC_RESOLUTION_16BIT`, `ADC_MODE_DIFFERENTIAL` |
| `ADC_setInterruptPulseMode(base, pulseMode)` | 设置 EOC 中断脉冲产生时机：推荐选 **`ADC_PULSE_END_OF_CONV`**（转换彻底完成、结果已锁存进 `ADCRESULTx` 时才触发中断）。 |
| `ADC_enableConverter(base)` | 给 ADC 模拟内核上电，**上电后必须延时至少 `1 ms` (`DEVICE_DELAY_US(1000)`) 等待模拟基准稳定**！ |
| `ADC_setupSOC(base, socNumber, trigger, channel, sampleWindow)` | **最核心配置函数**：将 `ADC_SOC_NUMBER0~15` 绑定到指定触发源（如 `ADC_TRIGGER_EPWM1_SOCA`）、模拟通道（如 `ADC_CH_ADCIN0`）和采样保持窗口 `sampleWindow`（单位为 `SYSCLK` 周期数，12位模式最小填 `15U` 即 75ns，16位模式最小填 `64U` 即 320ns）。 |
| `ADC_setInterruptSource(base, adcIntNum, socNumber)` | 将某个 SOC 的转换完成信号（EOCx）路由到 `ADC_INT_NUMBER1~4`（通常将某组最后一个转换完的 SOC 绑定到 `ADC_INT_NUMBER1`）。 |
| `ADC_enableInterrupt(base, adcIntNum)` / `ADC_enableContinuousMode(base, adcIntNum)` | 使能 ADC 中断输出（若开启 Continuous Mode，即使未清中断标志也能继续产生后续中断脉冲）。 |
| `ADC_readResult(resultBase, socNumber)` | **ISR 内读取原始结果 (`static inline`)**：传入 `ADCARESULT_BASE` 和 `ADC_SOC_NUMBER0`，返回 `0 ~ 4095`（12-bit）或 `0 ~ 65535`（16-bit）。 |
| `ADC_clearInterruptStatus(base, adcIntNum)` | **在 ADC ISR 退出前必调**：清除 `ADCINT1` 标志位（若发现硬件发生中断溢出，还需调 `ADC_clearInterruptOverflowStatus`）。 |

### 1.1 后处理块 (Post-Processing Block, `PPB`) 硬件减偏置加速
采样交流电流/电压时，硬件电路通常有 $1.5\text{V}$ 或 $1.65\text{V}$ 直流偏置（对应码值约 `2048`）。利用每个 ADC 自带的 4 个 PPB 硬件单元，可以在转换完成的瞬间**由硬件自动减去零点偏置**，直接读出带符号的 `int32_t` 正负值：
```c
// 配置阶段：将 PPB1 绑定到 SOC0，并设置零点校准偏移量为 2048
ADC_setupPPB(ADCA_BASE, ADC_PPB_NUMBER1, ADC_SOC_NUMBER0);
ADC_setPPBCalibrationOffset(ADCA_BASE, ADC_PPB_NUMBER1, 2048);

// ISR 内直接读取带符号结果（等于 ADCRESULT0 - 2048），省去软件减法与类型强转！
int32_t currentRawSigned = ADC_readPPBResult(ADCARESULT_BASE, ADC_PPB_NUMBER1);
```

---

## 2. 片上模拟比较器子系统 (`CMPSS_*` —— 硬件微秒级过流保护)

CMPSS 包含 **高侧比较器 (`COMPH`)** 和 **低侧比较器 (`COMPL`)**，每个比较器反相端都自带一个 **12-bit 内部 DAC**，可设置正负双向过流保护阈值，输出经过数字滤波器（Digital Filter）消毛刺后，直接通过 **ePWM X-BAR** 触发 ePWM 的 **TZ 封波**，全程纯硬件异步响应（$< 50\text{ ns}$），无需 CPU 介入！

| 函数签名 | 功能说明 |
| :--- | :--- |
| `CMPSS_enableModule(base)` | 使能指定 CMPSS 模块电源与时钟。 |
| `CMPSS_configHighComparator(base, config)` | 配置高比较器（如 `CMPSS_INSRC_DAC` 反相端接内部 DAC）。 |
| `CMPSS_configLowComparator(base, config)` | 配置低比较器（如 `CMPSS_INSRC_DAC | CMPSS_INV_INVERTED` 当输入低于下限 DAC 值时输出高电平触发保护！）。 |
| `CMPSS_configDAC(base, config)` | 配置内部 DAC 基准源（`CMPSS_DACREF_VDDA`）与加载模式（`CMPSS_DACVAL_SYSCLK`）。 |
| `CMPSS_setDACValueHigh(base, value)` / `CMPSS_setDACValueLow(base, value)` | 设置过流保护上限与下限阈值（`0 ~ 4095`）。 |
| `CMPSS_configFilterHigh(base, samplePrescale, sampleWindow, threshold)` | 配置高侧数字滤波器（在 `sampleWindow` 个采样点中有超过 `threshold` 个点为高才判定为真实过流，滤除开关尖峰）。 |
| `CMPSS_initFilterHigh(base)` | 初始化数字滤波器采样寄存器。 |
| `CMPSS_configOutputsHigh(base, config)` | 选择输出信号源：`CMPSS_TRIP_FILTER`（走数字滤波输出到 ePWM X-BAR）或 `CMPSS_TRIP_ASYNC_COMP`（极速异步直通）。 |
