# 02 - Linker Command File (`.cmd`) 链接脚本语法详解

> **关联手册**：*TMS320C28x Assembly Language Tools User's Guide (`SPRU513`)* 第 8.5 节 "Linker Command Files"

在 C2000 工程中，`.cmd` 文件（Linker Command File）负责指挥链接器（Linker）将编译器生成的各个逻辑段（`.text`, `.ebss`, `.stack` 等）放置到芯片具体的物理内存地址上。

一个标准的 `.cmd` 文件仅由两个核心伪指令块组成：
1. **`MEMORY`**：声明芯片有哪些物理内存块（起止地址与长度）。
2. **`SECTIONS`**：指定编译生成的各个代码/数据段应该放进 `MEMORY` 中的哪一块。

---

## 1. `MEMORY` 块语法与 `PAGE 0` / `PAGE 1` 机制

```text
MEMORY
{
PAGE 0 : /* Program Memory (程序空间：习惯放代码段、常量初始化表、Flash 扇区) */
   BEGIN           : origin = 0x080000, length = 0x000002
   RAMM0           : origin = 0x000122, length = 0x0002DE
   RAMLS0_LS2      : origin = 0x008000, length = 0x006000  /* 合并 LS0~LS2 为连续 6K Words */
   FLASHB_C        : origin = 0x082000, length = 0x004000  /* 合并 Sector B & C 为 16K Words */
   RESET           : origin = 0x3FFFC0, length = 0x000002

PAGE 1 : /* Data Memory (数据空间：习惯放全局变量、堆栈、外设寄存器帧) */
   BOOT_RSVD       : origin = 0x000002, length = 0x000120  /* Boot ROM 占用栈空间，不可动 */
   RAMM1           : origin = 0x000400, length = 0x000400  /* 1K Words，专门给 .stack */
   RAMD0_D1        : origin = 0x00B000, length = 0x001000  /* 合并 D0 & D1 为 4K Words */
   RAMLS3_LS5      : origin = 0x009800, length = 0x003000
   RAMGS0          : origin = 0x00C000, length = 0x001000
}
```

### 1.1 为什么会有 `PAGE 0` 和 `PAGE 1`？
- **历史渊源**：早期定点 DSP 采用严格哈佛架构，程序空间（`PAGE 0`）和数据空间（`PAGE 1`）拥有完全独立的物理地址空间（即程序空间的 `0x0000` 和数据空间的 `0x0000` 是两块不同的芯片内存）。
- **C28x 现状**：**C28x 采用统一内存映射（Unified Memory Map）**！在硬件物理上，`PAGE 0` 的 `0x008000` 和 `PAGE 1` 的 `0x008000` 是**同一块物理内存 (`LS0`)**！
- **核心防坑规则**：
  > [!CAUTION]
  > **绝不能把同一段物理地址同时定义在 `PAGE 0` 和 `PAGE 1` 中！**
  > 因为链接器认为 `PAGE 0` 和 `PAGE 1` 是两个互不相干的平行宇宙。如果你在 `PAGE 0` 和 `PAGE 1` 都写了 `origin = 0x008000, length = 0x000800`，链接器不会报错，而是把代码段和全局变量段分配到同一个物理地址上，运行时代码和数据互相覆盖，瞬间死机！
- **现代 EABI 趋势**：在 TI 最新的 EABI `.cmd` 文件中，为了彻底杜绝 `PAGE 0`/`PAGE 1` 重叠隐患，越来越多工程将所有内存统一写在 `PAGE 0`（或在 `SECTIONS` 中不写 `PAGE = 1`），这也是完全合法且推荐的做法。

### 1.2 如何合并相邻内存块以容纳大数组或大代码段？
C2000 默认 `.cmd` 将 `LS0`~`LS5`、`GS0`~`GS15`、`FLASHA`~`FLASHN` 拆成小块。如果你的 `.text` 代码段超过了单个 `FLASHB`（8K Words），或者定义了一个 `6000` 字的大数组超过单个 `GS0`（4K Words），有两种解决办法：
1. **在 `MEMORY` 中直接合并物理连续的块**：如上方示例将 `LS0`（`0x008000`）、`LS1`（`0x008800`）、`LS2`（`0x009000`）合并为 `RAMLS0_LS2 : origin = 0x008000, length = 0x001800`。
2. **在 `SECTIONS` 中使用 `>>` 跨块拆分运算符**（见下文）。

---

## 2. `SECTIONS` 块核心语法与分配操作符

`SECTIONS` 块决定了输入段（Input Sections）如何组装成输出段（Output Sections）并放置到 `MEMORY` 区域中。

### 2.1 四大分配操作符对比：`>`、`|`、`>>`、`LOAD / RUN`

```text
SECTIONS
{
   /* 1. 单一分配 (>)：整个段必须完整放入单个内存块 */
   codestart       : > BEGIN,      PAGE = 0, ALIGN(4)
   .stack          : > RAMM1,      PAGE = 1

   /* 2. 备选分配 (|)：按顺序尝试，哪个放得下就整个放进哪个（不拆分） */
   .ebss           : > RAMLS3_LS5 | RAMGS0, PAGE = 1

   /* 3. 跨块拆分分配 (>>)：当单个块放不下时，自动把段拆成多块塞进列表中的多个区域！ */
   .text           : >> FLASHB | FLASHC | FLASHD | FLASHE, PAGE = 0, ALIGN(4)

   /* 4. 加载地址与运行地址分离 (LOAD = ..., RUN = ...)：用于将 Flash 代码搬到 RAM 运行 */
   .TI.ramfunc     : LOAD = FLASHD,
                     RUN = RAMLS0_LS2,
                     LOAD_START(_RamfuncsLoadStart),
                     LOAD_SIZE(_RamfuncsLoadSize),
                     LOAD_END(_RamfuncsLoadEnd),
                     RUN_START(_RamfuncsRunStart),
                     RUN_SIZE(_RamfuncsRunSize),
                     RUN_END(_RamfuncsRunEnd),
                     PAGE = 0, ALIGN(4)
}
```

| 语法符号 | 含义说明 | 适用场景与限制 |
| :--- | :--- | :--- |
| `.sec : > MEM` | 将 `.sec` 完整放入 `MEM` | 适合 `.stack`、`codestart` 或明确大小的数据段 |
| `.sec : > MEM1 \| MEM2` | 尝试放入 `MEM1`，若 `MEM1` 放不下则**整体**改放 `MEM2` | 单个函数或单个大数组不能被拦腰截断，但允许链接器挑一块够大的内存 |
| `.sec : >> MEM1 \| MEM2` | 将 `.sec` 内的多个子模块（如不同 `.obj` 的函数/变量）**拆分**放入 `MEM1` 和 `MEM2` | **极适合 `.text`、`.econst`、`.ebss`**！注意：**单个数组或单个函数本身不可跨越不连续的物理地址拆分** |
| `ALIGN(4)` / `ALIGN(8)` | 强制该段起始地址按 4 Words (64-bit) 或 8 Words (128-bit) 边界对齐 | **所有分配到 Flash 的段必须加 `ALIGN(4)` 或 `ALIGN(8)`**，以满足 Flash 128-bit ECC 编程对齐要求！ |

> [!IMPORTANT]
> **COFF 与 EABI 下链接器符号的前缀差异**：
> 在 C 语言源码中声明 `extern uint16_t RamfuncsLoadStart;` 时：
> - **COFF (`coffabi`) 模式**：C 编译器会在所有 C 符号前自动加下划线 `_`，因此 `.cmd` 文件里必须写 `LOAD_START(_RamfuncsLoadStart)`（带下划线）。
> - **EABI (`eabi`) 模式**：C 编译器**不再加下划线前缀**，因此 `.cmd` 文件里必须写 `LOAD_START(RamfuncsLoadStart)`（无下划线）。
> 为了同时兼容两种模式，TI 官方 `.cmd` 常使用条件宏：
> ```c
> #if defined(__TI_EABI__)
>     LOAD_START(RamfuncsLoadStart),
> #else
>     LOAD_START(_RamfuncsLoadStart),
> #endif
> ```

---

## 3. 特定文件或特定函数的精准内存定向

除了在 C 代码中写 `#pragma CODE_SECTION`，你还可以直接在 `.cmd` 文件中把某个 `.obj` 文件（或静态库 `.lib` 中的某个模块）单独挑出来放进指定 RAM：

```text
SECTIONS
{
   /* 将 motor_foc.obj 中的所有代码 (.text) 和常量直接放入零等待 RAMLS0 运行，无需改 C 源码！ */
   fast_foc_code :
   {
       motor_foc.obj (.text)
       motor_foc.obj (.econst)
   } LOAD = FLASHD, RUN = RAMLS0_LS2,
     TABLE(BINIT),   /* 使用链接器自动生成的 BINIT 表在 _c_int00 阶段自动完成拷贝！ */
     PAGE = 0, ALIGN(4)
}
```
> [!TIP]
> **`TABLE(BINIT)` 黑科技**：在 `LOAD = ..., RUN = ...` 后加上 `TABLE(BINIT)`，C2000 的启动代码 `_c_int00` 会在进入 `main()` 之前自动读取 BINIT 表完成 Flash 到 RAM 的拷贝，甚至不需要在 `main()` 里手写 `memcpy`（但注意：若目标 RAM 是需要解锁或初始化后才可用的区域，仍建议在 `main()` 中手动 `memcpy`）。
