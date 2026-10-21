# 03 - C2000 常见疑难故障排查手册（`ILLEGAL_ISR`、优化失效与非原子撕裂）

---

## 1. 头号杀手：程序跑进 `Interrupt_illegalOperationHandler` (`ILLEGAL_ISR` / `ITRAP0`)

当 C28x CPU 从内存中取到一条**无法识别的非法机器码（如全 `0x0000` 或全 `0xFFFF`）**时，硬件会立即触发不可屏蔽的 **`ITRAP0`（Illegal Instruction Trap）**，跳入 `ILLEGAL_ISR` 死循环停机。

### 1.1 触发 `ILLEGAL_ISR` 的五大常见真凶

| 排名 | 故障原因 | 详细机理与修复方案 |
| :---: | :--- | :--- |
| **No.1** | **未执行 `memcpy` 就调用了 `.TI.ramfunc` 函数** | 工程切到 Flash 后，忘了在预定义宏加 `_FLASH`（或忘了调 `memcpy(&RamfuncsRunStart, ...)`），随后调用 `InitFlash()` 或进入 RAM ISR 时，CPU 跳到全 `0x0000` 的未初始化 RAM 区取指（`0x0000` 在 C28x 中恰好就是非法指令码 `ITRAP0`！）。 |
| **No.2** | **在 Flash 内部直接修改了 Flash 寄存器** | `InitFlash()` 函数没有被成功放进 `.TI.ramfunc`，导致 CPU 一边在 Flash 里取指，一边写 `Flash0CtrlRegs`，引发 Flash 总线冲突触发 ITRAP。 |
| **No.3** | **系统堆栈 (`.stack`) 溢出** | 默认 `.stack` 分配在 `RAMM1`（仅 `0x400` = 1024 Words）。如果在函数中定义了大局部数组，入栈覆盖了保存在栈上的函数返回地址 `RPC`，函数退出 `LRETR` 时跳飞到非法地址。 |
| **No.4** | **开启了某个外设中断，但未注册 ISR 函数指针** | 虽然 `Interrupt_initVectorTable()` 有默认兜底，但如果手动清空了 `PieVectTable` 或写坏了向量表，进中断瞬间跳向空指针 `0x000000`。 |
| **No.5** | **函数指针未初始化（野指针调用）** | 状态机或回调结构体中的函数指针为 `NULL` (`0x00000000`) 时被直接调用。 |

### 1.2 破案绝招：30 秒从堆栈 (`SP`) 揪出跳飞前的精确代码行！
当仿真器停在 `ILLEGAL_ISR` 的 `ESTOP0` 处时，由于 `ITRAP0` 是一次硬件中断入栈过程，**触发异常那一刻的 `PC` 地址和返回地址 `RPC` 已经被硬件保存在了当前栈顶和 `RPC` 寄存器中**！
1. 打开 CCS 的 **`View -> Registers`** 窗口：
   - 先看 **`Core Registers -> RPC`**：如果是因为调用了一个空函数指针或未拷贝的 `.TI.ramfunc` 函数跳飞的，**`RPC` 寄存器里存的值就是发起那次致命函数调用的下一条指令地址**！
   - 再看 **`Core Registers -> SP`**（例如 `SP = 0x0426`）：打开 `Memory Browser` 查看 `SP - 2`（即 `0x0424` 处存放的 32-bit 压栈 `PC` 值），这就是触发 `ITRAP0` 时的精确 `PC` 地址！
2. 打开 **`View -> Disassembly`（反汇编窗口）**，输入上面查到的 `RPC` 地址，直接就能看到是哪一行 C 代码调用了哪个函数导致的崩溃！

---

## 2. 编译优化 (`-O2`) 引发的死循环：`volatile` 缺失

### 典型现象
在 `-O0`（无优化）下程序完全正常，一旦开启 `-O2` 优化，主循环等待中断标志位 `while(g_adcReadyFlag == 0) {}` 就永远卡死出不来！

### 原因与法则
在 `-O2` 下，编译器分析 `while` 循环体内部没有修改 `g_adcReadyFlag`，于是只在进入循环前从 RAM 读了一次到 CPU 寄存器 `AL`，随后变成死循环比较 `AL == 0`，再也不去 RAM 重新读取被 ISR 修改后的值！
- **铁律**：凡是满足以下任一条件的全局变量，定义时**必须加上 `volatile` 关键字**：
  1. 在 **主循环与 ISR 之间共享** 的标志位或数据变量；
  2. 在 **CPU 与 CLA 之间共享**（`MsgRAM` / `LSx`）的变量；
  3. 在 **CPU1 与 CPU2 之间共享**（`GSx` / `IPC MsgRAM`）或 **DMA 搬运目标** 的变量。

---

## 3. 主循环与高频 ISR 之间的 32-bit 变量非原子读写撕裂 (Data Tearing)

### 机理剖析
虽然 C28x 有 32-bit 累加器 `ACC` 和 32-bit 传送指令 `MOVL`，但在结构体赋值、双字拆分计算或特定寻址模式下，更新一个 32 位变量（`float` / `uint32_t`）或一个包含多个关联参数的结构体（如 `kp` 和 `ki`）需要多条指令：
- 如果主循环刚写完高 16 位（或刚改完 `kp` 还没改 `ki`），恰好被 50kHz 的控制 ISR 打断，ISR 就会读到“半新半旧”的撕裂数据！

### 解决方案
1. **单向传递参数**：主循环准备好新参数后，在 ISR 开头（或结尾）统一从影子缓冲结构体拷贝一次到 ISR 私有工作结构体。
2. **极短临界区保护**：在主循环更新多参数结构体时，用 `DINT;` 和 `EINT;` 包裹（仅耗时 3~5 个时钟周期，约 `20 ns`）：
   ```c
   DINT; // 关中断保护原子更新 (仅几纳秒)
   g_ctrlParams.kp    = newKp;
   g_ctrlParams.ki    = newKi;
   g_ctrlParams.idRef = newIdRef;
   EINT; // 立即恢复全局中断
   ```
