# 03 - 硬件数学加速单元：FPU32、TMU 与 VCU

> **关联手册**：*TMS320C28x Extended Instruction Sets Technical Reference Manual (`SPRUHS1`)*

TMS320F2837xD 的每个 C28x CPU 核不仅包含定点数内核，还集成了三大硬件协处理加速扩展：**FPU32（单精度浮点单元）**、**TMU（三角函数数学单元）** 和 **VCU-II（维特比/复数/CRC 单元）**。

---

## 1. FPU32（32位单精度硬件浮点单元）

### 1.1 独立寄存器组
FPU32 拥有自己独立的一套 32 位寄存器，与定点 `ACC`/`XAR` 寄存器并行工作：
- **`R0H` ~ `R7H`**：8 个 32-bit IEEE-754 单精度浮点寄存器（执行浮点加、减、乘、比较、绝对值、最大最小值）。
- **`STF`**：浮点状态寄存器（存放浮点零标志 `ZF`、负标志 `NF`、上溢/下溢标志等）。
- **`RB`**：硬件块循环寄存器（支持零开销 `RPTB` 硬件循环指令，无需每轮软件判断计数跳转）。

### 1.2 流水线延迟槽 (Delay Slot) 注意事项
FPU32 的大多数算术指令（如 `MPYF32` 浮点乘、`ADDF32` 浮点加）需要 **2 个流水线周期（1 个延迟槽）** 才能将结果写入 `RnH`：
- C 编译器会自动重排无关指令填入延迟槽，实现单周期吞吐。
- 单条比较/移动指令（`MOVF32`、`MAXF32`、`MINF32`、`ABSF32`）仅需 **1 个周期**。

---

## 2. TMU（Trigonometric Math Unit，三角函数与非线性数学单元）

在电机控制（FOC 的 Park / InvPark 变换、锁相环 PLL）与数字电源（并网逆变器锁相、坐标变换）中，三角函数、开方和浮点除法极度高频。**TMU 在硬件层面用泰勒级数查找表加速了这些原本需要几十上百周期的软件库函数**！

### 2.1 TMU 加速性能对比

| 数学运算 | 传统软件浮点库 (`rts2800_fpu32.lib`) | 开启 TMU 硬件指令 (`--tmu_support=tmu0`) | 对应 C28x 汇编指令 |
| :--- | :---: | :---: | :--- |
| 浮点除法 `a / b` | ~28 周期 | **5 周期** | `DIVF32` |
| 平方根 `sqrtf(x)` | ~29 周期 | **5 周期** | `SQRTF32` |
| 正弦 `sinf(rad)` | ~49 周期 | **4 周期** | `SINPUF32` |
| 余弦 `cosf(rad)` | ~49 周期 | **4 周期** | `COSPUF32` |
| 反正切 `atanf(x)` / `atan2f(y,x)` | ~80+ 周期 | **4 ~ 11 周期** | `ATANPUF32` / `QUADF32` |

### 2.2 如何在工程中启用与高效使用 TMU

#### 步骤 1：配置编译器选项
在 CCS 工程属性 `C2000 Compiler -> Processor Options` 中：
1. `--float_support = fpu32`
2. `--tmu_support = tmu0`
3. `--fp_mode = relaxed`（**关键**！设为 `relaxed` 后，你写标准 C 的 `sinf(x)`、`cosf(x)`、`sqrtf(x)`、`a / b`，编译器会自动替换为 TMU 硬件指令；若为 `strict`，则必须手动调内联函数）。

#### 步骤 2：极致性能优化——使用标幺化角度（Per-Unit）内联函数
标准 `sinf(theta)` 需要先把弧度 $[-\pi, \pi]$ 除以 $2\pi$ 转换为标幺化角度 $[-1.0, 1.0]$（多耗费 1 次乘法）。如果控制算法中的电角度本身就归一化在 $[0.0, 1.0)$（代表 $0 \sim 2\pi$），直接调用 TMU 标幺内联函数只需 **4 个时钟周期**：

```c
#include <stdint.h>

// 输入 angle_pu 为标幺化角度：0.0f ~ 1.0f 对应 0 ~ 2*PI 弧度
void FOC_UpdateTrig(float angle_pu, float *sin_val, float *cos_val)
{
    *sin_val = __sinpuf32(angle_pu); // 直接生成 1 条 SINPUF32 指令 (4周期)
    *cos_val = __cospuf32(angle_pu); // 直接生成 1 条 COSPUF32 指令 (4周期)
}

// 其他常用 TMU/FPU 内联函数：
// __divf32(num, den)   -> 硬件浮点除法 a / b
// __sqrt(x)            -> 硬件平方根 sqrt(x)
// __atanpuf32(x)       -> 硬件标幺反正切，返回值范围 [-0.25, 0.25] 对应 [-PI/2, PI/2]
// __fmax(a, b)         -> 单周期取最大值 MAXF32
// __fmin(a, b)         -> 单周期取最小值 MINF32
// __fsat(val, max, min)-> 2周期上下限幅饱和（写 PID 抗饱和神器！）
```

> [!TIP]
> **`__fsat(val, max, min)` 硬件限幅内联函数**：
> 在 PID 控制器或占空比限幅中，不要写 `if (out > max) out = max; else if (out < min) out = min;`（会产生两次条件分支跳转）。直接写：
> ```c
> out = __fsat(out, 0.95f, 0.05f); // 仅生成 MAXF32 + MINF32 两条无分支指令！
> ```

---

## 3. VCU-II（Viterbi, Complex Math, and CRC Unit）

VCU-II 主要面向两大场景：
1. **硬件 CRC 校验加速**：
   - 支持 `CRC-8`、`CRC-16`（多项式 `0x8005` 和 `0x1021`）、`CRC-32` 的单周期字节计算。
   - 在做 Modbus RTU、CAN 自定义通信协议、或 Flash 固件完整性自检时，结合 C2000Ware 的 `libraries/dsp/VCU/` 库，速度比软件查表法还快 4~8 倍。
2. **复数运算与 FFT 加速**：
   - 提供复数乘法、蝶形运算指令，用于电力谐波分析（FFT）与电力线载波通信（PLC）。
