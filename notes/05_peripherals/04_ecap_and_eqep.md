# 04 - eCAP 脉冲捕获与 eQEP 正交编码器测速原理

> **关联手册**：*TMS320F2837xD Technical Reference Manual (`SPRUHM8K`)* 第 16 章 "Enhanced Capture (eCAP)" & 第 17 章 "Enhanced Quadrature Encoder Pulse (eQEP)"

---

## 1. eCAP（增强型脉冲捕获模块）

F2837xD 包含 6 个独立的 32 位捕获模块 (`eCAP1` ~ `eCAP6`)，其输入引脚必须先通过 **Input X-BAR (`INPUT7` ~ `INPUT12`)** 路由进来。

### 1.1 四级时间戳状态机（测量外部 PWM 频率与占空比）
eCAP 内部有一个以 `SYSCLK`（200 MHz，分辨率 $5\text{ ns}$）高速递增的 32-bit 计数器 `TSCTR`，以及 4 个依次轮转锁存的捕获寄存器 `CAP1`、`CAP2`、`CAP3`、`CAP4`：
- **差分模式（Delta Mode，`CTRRSTn = 1`）**：每次捕获到指定边沿把计数值锁存进 `CAPn` 后，**硬件立即自动将 `TSCTR` 清零重新从 0 计时**！
- **典型四事件连续配置**：
  1. `Event 1`（上升沿 + 重置 `TSCTR`）$\rightarrow$ `CAP1` 存上一周期的**低电平持续时间** $T_{\text{low}}$。
  2. `Event 2`（下降沿 + 重置 `TSCTR`）$\rightarrow$ `CAP2` 存本周期的**高电平持续时间** $T_{\text{high}}$。
  3. `Event 3`（上升沿 + 重置 `TSCTR`）$\rightarrow$ `CAP3` 存**低电平持续时间** $T_{\text{low}}$。
  4. `Event 4`（下降沿 + 重置 `TSCTR`）$\rightarrow$ `CAP4` 存**高电平持续时间** $T_{\text{high}}$。
- 无需处理 32 位计数器溢出回绕，直接可得周期 $T = \text{CAP1} + \text{CAP2}$，占空比 $D = \frac{\text{CAP2}}{\text{CAP1} + \text{CAP2}}$！

---

## 2. eQEP（增强型正交编码器接口）

F2837xD 包含 3 个独立的正交编码器模块 (`eQEP1` ~ `eQEP3`)，直接支持光电/磁编码器的 `EQEPxA`、`EQEPxB`（正交两相脉冲）和 `EQEPxI`（Z 相零位索引脉冲）。

### 2.1 四倍频位置计数与电角度换算
- eQEP 的正交解码单元（QDU）默认对 `A`、`B` 两相脉冲的上升沿和下降沿进行 **4 倍频计数**。
- 若编码器线数为 $N$（例如 $N = 1000$ 线或 $2500$ 线），则电机旋转机械一圈产生 $4N$ 个计数脉冲，位置计数器上限设为：
  $$\text{QPOSMAX} = 4N - 1$$
- 读取实时位置 `pos = EQEP_getPosition(EQEP1_BASE)`，配合电机极对数 $P$，即可直接算出归一化电角度。

### 2.2 高速 M 法测速 vs 低速 T 法测速硬件支持

eQEP 在硬件内部同时集成了支撑 **M 法（定时间测脉冲数）** 和 **T 法（定脉冲测高频时钟个数）** 的两套计时器：

| 测速方法 | 适用转速区间 | eQEP 内部硬件支撑单元 | 转速计算公式（RPM） |
| :--- | :--- | :--- | :--- |
| **M 法（频率法）** | **中高速区** | **Unit Timer（单位定时器 `QUPRD`）**：每隔固定时间 $T_{\text{unit}}$（如 $1\text{ ms}$）硬件自动将当前位置锁存进 `QPOSLAT` | $$n = \frac{\Delta \text{QPOSLAT}}{4N \cdot T_{\text{unit}}} \times 60$$ |
| **T 法（周期法）** | **极低速区** | **Edge Capture Unit（边沿捕获单元 `QCTMR` / `QCPRD`）**：以分频后的 `SYSCLK` 测量相邻编码器脉冲之间的精确时钟数 `QCPRD` | $$n = \frac{f_{\text{cap\_clk}}}{4N / U \cdot \text{QCPRD}} \times 60$$ |
