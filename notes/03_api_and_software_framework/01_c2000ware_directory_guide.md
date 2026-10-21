# 01 - C2000Ware 软件开发包目录结构与工程配置指南

> **关联路径**：默认安装于 `C:/ti/c2000/C2000Ware_<version>/`

**C2000Ware** 是 TI 官方为 C2000 系列提供的全套底层驱动、外设头文件、链接脚本、数学/DSP 算法库以及示例工程集合。新建或移植工程时，必须清楚各个文件夹的职责以及 CCS 编译器/链接器需要引入哪些路径。

---

## 1. C2000Ware 核心目录导航

```text
C2000Ware_<version>/
├── driverlib/                         # 【现代核心】DriverLib 函数式 API 驱动库
│   └── f2837xd/
│       ├── driverlib/                 # CPU1/CPU2 共用的外设驱动源码 (.c / .h)
│       │   ├── inc/                   # hw_memmap.h (外设基地址宏)、hw_types.h (HWREG宏) 及寄存器偏移宏
│       │   ├── ccs/Debug/driverlib.lib# 预编译好的静态驱动库（直接链接进工程即可）
│       │   ├── adc.h / epwm.h / ...   # 各外设 DriverLib API 头文件（含大量 static inline 函数）
│       │   └── driverlib.h            # 一键包含所有外设 DriverLib API 的总头文件
│       └── examples/                  # 基于 DriverLib 的官方单核/双核例程 (cpu1/, dual/)
│
├── device_support/                    # 【传统位域 & 芯片级支持】Bitfield 寄存器结构体、CMD 文件与启动代码
│   └── f2837xd/
│       ├── common/
│       │   ├── cmd/                   # 官方标准 .cmd 链接脚本 (2837xD_RAM_lnk_cpu1.cmd / FLASH_lnk)
│       │   ├── source/                # F2837xD_CodeStartBranch.asm、F2837xD_GlobalVariableDefs.c 等
│       │   └── include/               # F28x_Project.h 等传统辅助头文件
│       ├── headers/
│       │   ├── cmd/                   # F2837xD_Headers_nonBIOS_cpu1.cmd (将位域结构体映射到外设物理地址！)
│       │   └── include/               # F2837xD_device.h 及 F2837xD_epwm.h 等位域结构体定义
│       └── examples/                  # 基于传统 Bitfield 或混合模式的官方例程
│
└── libraries/                         # 【官方算法与加速库】
    ├── math/
    │   ├── FPUfastRTS/                # TMU/FPU 快速数学运算法则库
    │   ├── IQmath/                    # 定点 Q 格式数学库（无 FPU 芯片或特定定点算法使用）
    │   └── CLAmath/                   # 专供 CLA 协处理器调用的三角/开方数学库
    ├── dsp/
    │   ├── FPU/                       # 基于 FPU 的 FFT、FIR、IIR 滤波器库
    │   └── VCU/                       # 基于 VCU 的硬件 CRC 校验与复数运算库
    └── control/
        └── DCL/                       # Digital Control Library（官方汇编优化的 PID / DF22 / PR 控制器库！）
```

---

## 2. 标准 DriverLib 工程必备的构成要素 (`device/` 文件夹)

打开 C2000Ware 中任何一个现代 DriverLib 例程（如 `driverlib/f2837xd/examples/cpu1/empty_projects`），你会发现工程根目录下都有一个 `device/` 文件夹，里面只有两个核心文件：
1. **`device.h`**：
   - 定义外部晶振频率 `DEVICE_OSCSRC_FREQ`（如 LaunchPad 为 `10000000U` 即 10MHz，ControlCARD 为 `20000000U` 即 20MHz）。
   - 计算倍频系数宏 `DEVICE_SETCLOCK_CFG`，目标生成 **200 MHz `SYSCLK`**。
   - 定义常用开发板引脚宏（如 `DEVICE_GPIO_PIN_LED1`）。
2. **`device.c`**：
   - 实现 `Device_init()`：关看门狗 $\rightarrow$ 若定义了 `_FLASH` 则拷贝 `.TI.ramfunc` 到 RAM 并初始化 Flash 等待周期 $\rightarrow$ 调用 `SysCtl_setClock()` 锁定 200 MHz PLL $\rightarrow$ 开启所有外设时钟门控 `Device_enableAllPeripherals()`。
   - 实现 `Device_initGPIO()`：解锁 GPIO 配置锁。
   - 实现 `__error__()`：DriverLib 内部 `ASSERT()` 断言失败时的钩子函数（默认死循环 `ESTOP0` 停机，瞬间定位非法传参行号！）。

> [!WARNING]
> **LaunchPad 与 ControlCARD 晶振频率差异陷阱**！
> - **LAUNCHXL-F28379D (红色 LaunchPad)** 板载外部晶振是 **10 MHz**（需定义预编译宏 `_LAUNCHXL_F28379D`，此时 `IMULT_40, FMULT_0, SYSDIV_2` $\Rightarrow 10 \times 40 / 2 = 200\text{ MHz}$）。
> - **TMDSCNCD28379D (ControlCARD 控制卡)** 板载外部晶振是 **20 MHz**（未定义 `_LAUNCHXL_F28379D` 时默认按 20 MHz 配 `IMULT_20`，若直接烧进 LaunchPad，实际主频只有 **100 MHz**，导致所有 PWM 频率和串口波特率减半！）。

---

## 3. CCS 工程属性（Compiler & Linker）标准配置清单

### 3.1 头文件包含路径 (`Include Options`)
- `${COM_TI_C2000WARE_INSTALL_DIR}/driverlib/f2837xd/driverlib`
- `${PROJECT_ROOT}/device`
- *(若使用混合模式)* `${COM_TI_C2000WARE_INSTALL_DIR}/device_support/f2837xd/headers/include`
- *(若使用混合模式)* `${COM_TI_C2000WARE_INSTALL_DIR}/device_support/f2837xd/common/include`

### 3.2 预定义宏 (`Predefined Symbols` `--define`)
| 宏名称 | 作用说明 |
| :--- | :--- |
| **`CPU1`** 或 **`CPU2`** | **必填**！区分当前工程编译给哪个 CPU 核（决定外设归属与 IPC 寄存器视图）。 |
| **`_FLASH`** | **Flash 固化配置必填**！激活 `Device_init()` 中的 `memcpy(Ramfuncs...)` 和 `Flash_initModule()`。 |
| **`_LAUNCHXL_F28379D`** | 若使用 LaunchPad 开发板则必填，将外部晶振基準切换为 10MHz 并启用板载模拟基准切换。 |
| **`DEBUG`** | 开启 DriverLib 所有 API 入口处的 `ASSERT()` 参数合法性硬件断点检查；量产发布可移除以省去判断开销。 |

### 3.3 链接器库文件 (`File Search Path` `--library`)
1. `driverlib.lib`（位于 `driverlib/f2837xd/driverlib/ccs/Debug/driverlib.lib`）
2. `libc.a`（自动匹配对应的 C 标准运行时库，如 `rts2800_fpu32_eabi.lib`）
