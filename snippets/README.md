# 可复用代码片段与配置模板 (`snippets/`)

本目录存放经过验证、带详尽中文注释的独立代码模板与链接脚本，方便在新工程中直接参考或复制复用。

## 📂 已收录模板清单

1. **[f2837xd_flash_custom_template.cmd](./f2837xd_flash_custom_template.cmd)**
   - **TMS320F2837xD 实战定制 Flash 链接脚本模板**
   - 同时兼容 COFF (`coffabi`) 与 EABI (`eabi`)，合并 `RAMLS0_2` 容纳高频 `.TI.ramfunc`，合并 `RAMD0_1` 提供宽裕的 4K Words 栈空间，并预置 `ramgs0`（DMA 专用）与双核/CLA 消息 RAM 映射。
2. **[epwm_adc_closed_loop_template.c](./epwm_adc_closed_loop_template.c)**
   - **C2000 数字电源/电机控制标准实时闭环脚手架**
   - 完整演示：20kHz 中心对齐互补带 1us 死区 ePWM1 发波 $\rightarrow$ `TBCTR = ZERO` 触发 ADCA 采样 $\rightarrow$ 零等待 RAM 中断 (`adcA1ControlISR`) 读取结果并使用 `__fsat()` 硬件限幅更新占空比。
3. **[c28x_byte_pack_unpack.h](./c28x_byte_pack_unpack.h)**
   - **C28x 16-bit `char` 平台专用的 8-bit 字节流打包与解包头文件**
   - 提供基于单周期 `__byte()` (`MOVB`) 的紧凑字节数组读写，以及面向 UART/CAN 报文的 `uint16_t`、`uint32_t`、`float` 大小端序列化/反序列化内联函数。
