# 05 - `.map` 内存映射文件解读与内存溢出排错

> **关联手册**：*TMS320C28x Assembly Language Tools User's Guide (`SPRU513`)* 第 8.4.16 节 "Map File (`--map_file`)"

每次 CCS 编译链接完成后，都会在 `Debug/` 或 `Release/` 目录下生成一个 `<工程名>.map` 文件。它是诊断**内存分配失败**、**检查 `.TI.ramfunc` 是否真的搬移成功**、以及**评估 RAM/Flash 剩余容量**的最权威依据。

---

## 1. `.map` 文件的四大核心部分

打开一个典型的 C2000 `.map` 文件，重点关注以下四个区域：

### 1.1 第一部分：`MEMORY CONFIGURATION`（内存总体使用率表）
这是最直观的“体检表”，展示了 `.cmd` 文件中定义的每个物理内存块的总容量（`length`）、已用空间（`used`）和剩余空间（`unused`），**所有数值均为十六进制，单位为 16-bit Word**：

```text
MEMORY CONFIGURATION

         name            origin    length      used     unused   attr    fill
----------------------  --------  ---------  --------  --------  ----  --------
PAGE 0:
  BEGIN                 00080000   00000002  00000002  00000000  RWIX
  RAMM0                 00000122   000002de  00000000  000002de  RWIX
  RAMLS0                00008000   00000800  0000034a  000004b6  RWIX
  FLASHA                00080002   00001ffe  00000000  00001ffe  RWIX
  FLASHB                00082000   00002000  000018c4  0000073c  RWIX
  FLASHD                00086000   00002000  0000034a  00001cb6  RWIX

PAGE 1:
  RAMM1                 00000400   00000400  00000200  00000200  RWIX
  RAMLS5                0000a800   00000800  00000612  000001ee  RWIX
  RAMGS0                0000c000   00001000  00000400  00000c00  RWIX
```
- **如何快速换算字节数**：例如 `RAMLS0` 已用 `0x034A` = $842\text{ Words} = 1684\text{ Bytes}$。

---

### 1.2 第二部分：`SECTION ALLOCATION MAP`（段分配明细表）
展示每个段（如 `.text`, `.TI.ramfunc`, `.ebss`）具体由哪些 `.obj` 文件组成，以及每个 `.obj` 贡献了多少大小：

```text
SECTION ALLOCATION MAP

 output                                  attributes/
section   page    origin      length       input sections
--------  ----  ----------  ----------   ----------------
.TI.ramfunc 
*          0    00086000    0000034a     RUN ADDR = 00008000
                  00086000    00000210     driverlib.lib : flash.obj (.TI.ramfunc:Flash_initModule)
                  00086210    0000013a     epwm_control.obj (.TI.ramfunc:controlLoopISR)
```
> [!TIP]
> **如何确认函数已经成功配置为“Flash 加载、RAM 运行”？**
> 看上面 `.TI.ramfunc` 一栏：
> - `origin = 00086000`（位于 `FLASHD`）
> - 右侧明确标注了 **`RUN ADDR = 00008000`**（位于 `RAMLS0`）！
> - 下方列出了 `controlLoopISR` 包含在其中。只要看到这三点，说明链接器配置 100% 正确。

---

### 1.3 第三部分：`GLOBAL SYMBOLS`（全局符号地址表）
按字母顺序和按地址顺序列出所有全局变量、函数以及链接器符号的最终物理地址：
- 当遇到数组越界踩踏（某个变量莫名被改写）时，可以在 **`GLOBAL SYMBOLS: SORTED BY Symbol Address`** 中查找该变量地址**紧挨着的前一个变量是谁**，往往前一个数组越界就是真凶！

---

## 2. 经典链接报错：`error #10099-D: program will not fit into available memory`

### 报错示例 1：运行段（RAM 全局变量）溢出
```text
error #10099-D: program will not fit into available memory, or the section contains a call site that requires a trampoline that can't be generated for this section. run placement with alignment/blocking fails for section ".ebss" size 0x920 page 1. Available memory ranges:
   RAMLS5       size: 0x800        unused: 0x780        max hole: 0x780
```
- **原因解读**：`.ebss`（全局变量段）总大小达到了 `0x920` Words，但 `.cmd` 文件里只把它分配给了 `RAMLS5`（总大小仅 `0x800` Words，剩余 `0x780`）。
- **修复方法**：
  - **方法 A（推荐）**：在 `.cmd` 中使用 `>>` 允许 `.ebss` 跨多个 RAM 块分配：
    ```text
    .ebss : >> RAMLS4 | RAMLS5 | RAMGS0, PAGE = 1
    ```
  - **方法 B**：如果有单个超大数组（如 `float waveBuf[2048];` 占 `0x1000` Words），单独用 `#pragma DATA_SECTION(waveBuf, "ramgs1")` 把它挪到容量更大的 `GSx` RAM 中。

### 报错示例 2：加载段（Flash 代码）溢出
```text
error #10099-D: program will not fit into available memory. placement with alignment/blocking fails for section ".text" size 0x2450 page 0. Available memory ranges:
   FLASHB       size: 0x2000       unused: 0x2000       max hole: 0x2000
```
- **原因解读**：代码段 `.text` 大小 `0x2450` 超过了单个 `FLASHB` 扇区的 `0x2000`。
- **修复方法**：将 `.cmd` 中的 `.text : > FLASHB` 改为跨扇区拆分分配：
  ```text
  .text : >> FLASHB | FLASHC | FLASHD | FLASHE, PAGE = 0, ALIGN(8)
  ```

---

## 3. 使用 CCS Memory Allocation 图形化视图

除了直接看文本 `.map` 文件，在 CCS 中点击菜单：
**`View -> Memory Allocation`**
即可打开可视化的柱状进度条视图，实时看到每一块 `RAMM0/M1`、`RAMLSx`、`RAMGSx`、`FLASHx` 的占用百分比，展开后还能直接看到每个函数和数组吃掉了多少字节。
