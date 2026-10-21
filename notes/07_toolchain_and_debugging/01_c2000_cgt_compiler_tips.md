# 01 - TI C2000-CGT 编译器高阶特性、`#pragma` 与内联函数速查

> **关联手册**：*TMS320C28x Optimizing C/C++ Compiler User's Guide (`SPRU514`)*

---

## 1. 核心编译器选项 (`Compiler Options`) 解析

在 CCS 工程属性 `Build -> C2000 Compiler` 中，以下几个开关直接决定了生成机器码的指令集与运算性能：

| 编译器命令行选项 | 推荐配置值 (F2837xD) | 作用与性能影响 |
| :--- | :--- | :--- |
| **`-v28`** | `28` | 指定 C28x 32位内核基础指令集。 |
| **`--float_support`** | `fpu32` | 启用 32 位硬件单精度浮点协处理器指令（F2838x/F28P65x 可选 `fpu64`）。 |
| **`--tmu_support`** | `tmu0` | 启用硬件三角函数与非线性数学加速指令（`SINPUF32`, `DIVF32`, `SQRTF32`）。 |
| **`--fp_mode`** | `relaxed` | **性能关键**！允许编译器将标准 `<math.h>` 的 `sinf`/`cosf`/`sqrtf` 和浮点除法 `/` 自动替换为单周期/5周期 TMU 硬件指令！ |
| **`--cla_support`** | `cla1` | 启用 Type-1 CLA 协处理器编译支持（自动编译 `.cla` 后缀文件）。 |
| **`--vcu_support`** | `vcu2` | 启用 VCU-II 硬件 CRC 与复数运算指令集。 |
| **`--abi`** | `eabi`（新工程推荐）或 `coffabi` | 选择二进制接口标准（详见下方对比表）。 |
| **`-O` (`--opt_level`)** | `off`（调试初期）$\rightarrow$ `2`（实时发布） | `-O2` 开启全局寄存器分配、循环展开与内联函数完全展开，控制环路耗时通常可缩减 **40%~60%**！ |

---

## 2. 常用 `#pragma` 编译器指令速查

| `#pragma` 语法 (C 语言) | GCC 属性等价写法 (推荐) | 作用说明 |
| :--- | :--- | :--- |
| `#pragma CODE_SECTION(fn, ".TI.ramfunc")` | `__attribute__((ramfunc))` | 将函数放入 `.TI.ramfunc` 段（Flash 加载，RAM 运行）。 |
| `#pragma DATA_SECTION(var, "ramgs0")` | `__attribute__((section("ramgs0")))` | 将全局变量或数组放入 `.cmd` 指定的自定义数据段。 |
| `#pragma DATA_ALIGN(var, 2)` | `__attribute__((aligned(2)))` | 强制变量按 2 Words (32-bit) 或 4 Words (64-bit) 地址边界对齐。 |
| `#pragma FUNC_ALWAYS_INLINE(fn)` | `static inline __attribute__((always_inline))` | 无论优化等级是否为 `-O0`，强制将该函数内联展开，消除 `LCR` 调用开销。 |
| `#pragma MUST_ITERATE(min, max, multiple)` | - | 写在 `for` 循环正上方，告诉编译器循环次数确定，触发硬件零开销块循环指令 `RPTB`！ |
| `#pragma UNROLL(n)` | - | 写在 `for` 循环正上方，将循环体展开 `n` 次。 |
| `#pragma INTERRUPT(fn, HPI)` | - | **高优先级中断专用**：声明某 ISR 为 `HPI`（High Priority Interrupt），编译器使用硬件影子寄存器极速入栈，减少中断入口延迟！ |

---

## 3. C28x 必备编译器内联函数 (Intrinsics) 速查表

无需包含额外头文件，C2000 编译器原生内置了直接映射为单条汇编指令的 Intrinsic 函数：

| 内联函数签名 | 生成的汇编指令 | 典型应用场景 |
| :--- | :--- | :--- |
| `int &__byte(int *array, unsigned int byte_idx)` | `MOVB` | 按 8-bit 字节紧凑读写 `uint16_t` 数组的高/低半字。 |
| `float __fsat(float val, float max, float min)` | `MAXF32` + `MINF32` | **无分支单精度浮点上下限幅**（PID 积分限幅与输出限幅必备）。 |
| `long __sat(long val)` | `SAT ACC` | 32 位定点累加器饱和限幅。 |
| `float __fmax(float a, float b)` / `__fmin(a, b)` | `MAXF32` / `MINF32` | 1 周期求两个浮点数最大/最小值（SVPWM 扇区判断常用）。 |
| `float __divf32(float num, float den)` | `DIVF32` | 5 周期 TMU 硬件浮点除法 `num / den`。 |
| `float __sqrt(float x)` | `SQRTF32` | 5 周期 TMU 硬件浮点开方。 |
| `float __sinpuf32(float pu)` / `__cospuf32(float pu)` | `SINPUF32` / `COSPUF32` | 4 周期 TMU 标幺化正余弦（输入 `0.0 ~ 1.0` 对应 $0 \sim 2\pi$）。 |
| `float __atanpuf32(float x)` / `__atan2puf32(y, x)` | `ATANPUF32` + `QUADF32` | TMU 标幺化反正切/四象限反正切。 |
| `__eallow()` / `__edis()` | `EALLOW` / `EDIS` | 开启/关闭受保护寄存器写权限。 |
| `__eston0()` / `__asm(" ESTOP0")` | `ESTOP0` | 软件触发仿真器硬件断点（用于 `ASSERT` 故障停机保护）。 |

---

## 4. COFF (`coffabi`) 迁移至 EABI (`eabi`) 速查清单

| 对比维度 | 传统 COFF (`coffabi`) | 现代 EABI (`eabi`) | 迁移修改点 |
| :--- | :--- | :--- | :--- |
| **double 类型位宽** | 32-bit（等同于 `float`） | **64-bit** | 所有单精度浮点常量必须加 `f` 后缀（如 `1.0f`）！ |
| **C 符号汇编前缀** | 自动加下划线（`main` $\rightarrow$ `_main`） | **无下划线**（`main` $\rightarrow$ `main`） | `.cmd` 文件中的 `LOAD_START(_RamfuncsLoadStart)` 去掉前导下划线 `_`。 |
| **全局变量段名** | `.ebss`、`.econst`、`.esysmem` | **`.bss` + `.data`**、**`.const`**、**`.sysmem`** | 更新 `.cmd` 文件中的 `SECTIONS` 段名。 |
| **未初始化全局变量** | 上电保持 RAM 随机值 | **上电自动清零 (Zero-initialized)** | 符合现代 C99/C11 标准。 |
