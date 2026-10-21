# 05 - 串行通信外设：SCI (UART)、SPI、I2C 与 CAN

> **关联手册**：*TMS320F2837xD Technical Reference Manual (`SPRUHM8K`)* 第 18~22 章

---

## 1. SCI（UART 异步串行通信接口）

F2837xD 包含 4 路 SCI（`SCIA` ~ `SCID`）。在 LaunchPad 开发板上，**`SCIA`（`GPIO43_SCIRXDA` / `GPIO42_SCITXDA` 或 `GPIO85`/`GPIO84`）直接连到了板载 XDS100v2 仿真器的虚拟串口**，插上 USB 线即可与电脑串口助手通信。

### 1.1 波特率分频计算公式
SCI 的时钟源为低速外设时钟 **`LSPCLK`**（默认 $200\text{ MHz} / 4 = 50\text{ MHz}$）：
$$\text{BRR} = \frac{f_{\text{LSPCLK}}}{\text{BaudRate} \times 8} - 1$$
然后将 16-bit 的 `BRR` 拆成高 8 位写入 `SCIHBAUD`，低 8 位写入 `SCILBAUD`（使用 `SCI_setConfig(SCIA_BASE, DEVICE_LSPCLK_FREQ, 115200, ...)` 可自动完成计算）。

### 1.2 16 级硬件 FIFO 与接收错误锁死陷阱
- **16 级收发 FIFO**：开启 FIFO (`SCI_enableFIFO`) 后，可将接收中断深度设为 `SCI_FIFO_RX8` 或在后台主循环轮询 `SCI_getRxFIFOStatus(SCIA_BASE) != SCI_FIFO_RX0`，大幅降低 CPU 中断频率。
- **常见死机陷阱（RX Error 锁死）**：
  - 当串口线上受到拔插毛刺干扰产生帧错误（`FE`）或溢出错误（`OE`）时，SCI 硬件会置位 `SCIRXST.bit.RXERROR` 并**停止接收后续任何字节**！
  - **防呆处理**：在主循环或中断中定期检查 `(SCI_getRxStatus(SCIA_BASE) & SCI_RXSTATUS_ERROR)`，一旦发现错误，必须调用 **`SCI_performSoftwareReset(SCIA_BASE)`**（将 `SCICTL1.SWRESET` 清零再置 `1`）恢复接收机！

---

## 2. SPI（高速同步串行外设接口）

F2837xD 包含 3 路 SPI（`SPIA` ~ `SPIC`），常用于读取外部绝对值编码器（如旋变解码芯片 AD2S1210、磁编 TLE5012B/MT6835）、外扩 DAC 或驱动芯片配置。

### 2.1 四种时钟极性与相位模式对比
> [!WARNING]
> **TI SPI 相位定义与摩托罗拉标准命名习惯相反陷阱**！
> 在配置 SPI 对接第三方传感器 DataSheet 时，请直接对照下表选择 DriverLib 枚举，不要盲目按名字猜 `0/1`：

| 标准 SPI 模式 | 空闲时钟电平 (`CPOL`) | 采样边沿 (`CPHA`) | 对应 C2000 DriverLib `SPI_TransferProtocol` 枚举 |
| :--- | :---: | :--- | :--- |
| **Mode 0** (0, 0) | 低电平 | 第 1 个边沿（上升沿采样，下降沿发送） | **`SPI_PROT_POL0PHA1`** (`CLKPOLARITY=0, CLK_PHASE=1`) |
| **Mode 1** (0, 1) | 低电平 | 第 2 个边沿（上升沿发送，下降沿采样） | **`SPI_PROT_POL0PHA0`** (`CLKPOLARITY=0, CLK_PHASE=0`) |
| **Mode 2** (1, 0) | 高电平 | 第 1 个边沿（下降沿采样，上升沿发送） | **`SPI_PROT_POL1PHA1`** (`CLKPOLARITY=1, CLK_PHASE=1`) |
| **Mode 3** (1, 1) | 高电平 | 第 2 个边沿（下降沿发送，上升沿采样） | **`SPI_PROT_POL1PHA0`** (`CLKPOLARITY=1, CLK_PHASE=0`) |

### 2.2 左对齐数据格式注意事项 (`SPITXBUF` vs `SPIRXBUF`)
C28x 的 SPI 移位寄存器是 **最高位优先（MSB First）左对齐** 的！
- 当你配置字长为 $N\text{ bits}$（例如 `8-bit`）时：
  - **写入发送寄存器 (`SPITXBUF`)**：必须把数据**左移 $(16 - N)$ 位**放到高 $N$ 位发送！例如发 8-bit 数据 `0xA5`，必须写 `SPITXBUF = (0xA5 << 8);`（DriverLib 的 `SPI_writeDataNonBlocking()` 仍需你自己左移，或者使用 `SPI_transmitByte()` 会自动帮你左移 8 位！）。
  - **读取接收寄存器 (`SPIRXBUF`)**：硬件接收完成后会自动右移对齐到低 $N$ 位，直接读 `SPIRXBUF & 0xFF` 即可，无需移位！

---

## 3. CAN 控制器 (`DCAN`) 邮箱对象机制

F2837xD 包含 2 路经典 CAN 2.0B 控制器（`CANA_BASE`, `CANB_BASE`）：
- **时钟源**：可通过 `CAN_selectClockSource()` 选择直接来自外部晶振 `XTAL` 或系统时钟分频。
- **32 个独立邮箱对象 (Message Objects `1 ~ 32`)**：
  - 每个邮箱可独立配置为发送邮箱或带掩码滤波的接收邮箱：`CAN_setupMessageObject(CANA_BASE, objID, msgID, frameType, msgType, msgIDMask, flags, msgLen)`。
  - 发送数据：`CAN_sendMessage(CANA_BASE, objID, msgLen, txDataArray)`（注意 `txDataArray` 为 `uint16_t[]`，每个元素低 8 位存 1 个 CAN 字节）。
  - 接收数据：`CAN_readMessage(CANA_BASE, objID, rxDataArray)`。
