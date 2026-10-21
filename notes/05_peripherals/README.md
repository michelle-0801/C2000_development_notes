# 05 - 实时控制外设 (`05_peripherals`)

本目录记录 C2000 面向数字电源与电机控制的核心外设原理、寄存器配置与典型应用链路。

## 📚 笔记目录

1. **[01_gpio_and_xbar.md](./01_gpio_and_xbar.md)**
   - GPIO 二级复用机制 (`GPxGMUX` + `GPxMUX`) 与 `SET`/`CLEAR`/`TOGGLE` 原子操作防读-改-写竞争
   - 硬件输入限定滤波 (Input Qualification 3/6 采样窗消抖) 与三大 X-BAR（Input / Output / ePWM X-BAR）路由矩阵
2. **[02_epwm_deep_dive.md](./02_epwm_deep_dive.md)**
   - ePWM 七大子模块信号流向、增计数 vs 增减对称计数的周期 `TBPRD` 与占空比计算公式
   - 死区模块 `AHC` 互补对称模式与影子寄存器单周期双更新（Double Update）技术
3. **[03_adc_and_cmpss.md](./03_adc_and_cmpss.md)**
   - ADC 通道与 SOC 解耦架构、四路独立 ADC 多相电流同步采样技巧
   - **CMPSS + ePWM X-BAR + ePWM DC/TZ 纯硬件微秒级双向过流封波全链路**（附完整 DriverLib 代码模板）
4. **[04_ecap_and_eqep.md](./04_ecap_and_eqep.md)**
   - eCAP 四级时间戳差分模式测量外部 PWM 频率与占空比
   - eQEP 正交编码器四倍频位置计数与高速 M 法 / 低速 T 法测速硬件原理
5. **[05_communications.md](./05_communications.md)**
   - SCI (UART) 波特率计算、16 级 FIFO 与 `RXERROR` 锁死软件复位自愈
   - SPI 极性/相位命名对照表与左对齐移位 (`<< 8`) 注意事项、DCAN 32 邮箱对象收发机制
