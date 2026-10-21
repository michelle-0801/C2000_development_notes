# 05 - 实时控制外设 (`05_peripherals`)

本目录记录 C2000 面向数字电源与电机控制的核心外设原理、寄存器配置与典型应用链路。

## 📝 推荐笔记规划

- `01_gpio_and_xbar.md`
  - GPIO 多路复用 (Mux)、上下拉、输入限定滤波 (Qualification / 采样窗消抖)
  - Input X-BAR、Output X-BAR、ePWM X-BAR 信号内部路由矩阵
- `02_epwm_deep_dive.md`
  - 7 大子模块拆解：时基 (TB)、计数比较 (CC)、动作限定 (AQ)、死区 (DB)、斩波 (PC)、事件触发 (ET)、错误联防/封波保护 (TZ & DC)
  - 移相控制、变频变占空比影子寄存器 (Shadow Register) 更新时机
- `03_adc_and_cmpss.md`
  - 多路独立 ADC (ADCA~ADCD)、12-bit 单端 vs 16-bit 差分模式
  - SOC (Start-of-Conversion) 触发源与轮询优先级、PPB (后处理块：零点偏移校准、越界硬件比较、触发延时记录)
  - CMPSS (片上模拟比较器 + 内置 DAC + 斜坡发生器 + 数字滤波) 与 ePWM 硬件异步直连保护 (OST / CBC)
- `04_ecap_and_eqep.md`
  - eCAP 脉宽/频率捕获
  - eQEP 正交编码器测速（M 法 / T 法实现）与位置计数
- `05_communications.md`
  - SCI (UART) FIFO 中断收发、SPI、I2C、CAN / MCAN 通信笔记
