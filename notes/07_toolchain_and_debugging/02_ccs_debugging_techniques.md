# 02 - CCS (Code Composer Studio) 实时调试、波形绘图与硬件观察点技巧

在电机驱动或数字电源带载运行时，如果随便点击 CCS 的 **Suspend（暂停按钮）**，CPU 停机导致 PWM 固定输出高电平或控制环路开环，极易烧毁功率管！熟练掌握 CCS 专为 C2000 打造的**不停机实时调试工具**是安全做实验的前提。

---

## 1. Real-Time Debug Mode（实时不停机仿真模式）

C28x 硬件支持在不打断高频控制中断（如 `ADCA1` / `ePWM1`）的前提下，利用 CPU 空闲周期通过 JTAG 实时刷新查看和修改全局变量！

### 1.1 开启步骤
1. **在代码中**：在 `main()` 开启全局中断 `EINT;` 的下一行，确保写上 **`ERTM;`**（Enable Real-Time Debug Interrupt Mode，清除 `ST1.DBGM` 位）。
2. *(可选)* 如果希望哪怕主程序被断点暂停时，某路关键保护或控制中断依然继续在后台正常响应，可配置 **`DBGIER`** 寄存器（如 `DBGIER |= M_INT1;`）。
3. **在 CCS 调试界面中**：
   - 下载程序后，点击工具栏上的芯片小图标 **`Enable Silicon Real-time Mode (Polite real-time mode)`**。
   - 打开 **`Expressions`（表达式观察窗口）**，添加你想观察的变量（如 `g_motorSpeedRPM`, `g_kp`, `g_idRef`），然后点击 Expressions 窗口右上角的黄色循环箭头 **`Continuous Refresh`（连续自动刷新）**！
   - 此时电机正常高速旋转，你可以直接在 Expressions 窗口里在线修改 `g_idRef` 或 `g_kp` 参数并回车，参数在运行中立即生效！

---

## 2. Expressions 窗口高级技巧：Q 格式定点数换算显示

在调试定点算法（如 IQmath）或 ADC 原始偏置值时，如果一个 `int32_t` 变量采用 `Q24` 格式存储（即数值 $1.0$ 对应 $2^{24} = 16777216$）：
1. 在 `Expressions` 窗口中右键点击该变量 $\rightarrow$ 选择 **`Q-Values -> Q-Value(24)`**。
2. CCS 会自动将其除以 $2^{24}$ 显示为直观的十进制浮点数（如 `0.500000`），甚至允许你直接输入浮点数修改它！

---

## 3. 使用 CCS Graph 工具在线绘制片内高频波形

当需要观察 20kHz 电流环每一拍的电角度 $\theta_e$、$I_d/I_q$ 或 SVPWM 马鞍波，而手头没有 DAC 时，可以在 `GS0` RAM 中开一个环形数组缓存，然后用 **CCS Graph** 直接画出波形：

### 3.1 代码侧埋点（简易 Datalog 缓存）
```c
#define LOG_LEN  512U
#pragma DATA_SECTION(g_waveBufA, "ramgs0");
float g_waveBufA[LOG_LEN];
uint16_t g_logIdx = 0U;

// 在 20kHz 控制中断 ISR 末尾调用：
g_waveBufA[g_logIdx] = currentIa;
g_logIdx = (g_logIdx + 1U) & (LOG_LEN - 1U);
```

### 3.2 CCS Graph 配置参数
1. 点击菜单 **`Tools -> Graph -> Single Time`**。
2. 关键属性填写：
   - **Acquisition Buffer Size**：`512`（数组长度）
   - **Display Data Size**：`512`
   - **Dsp Data Type**：**`32-bit floating point`**（若是 `uint16_t` 则选 `16-bit unsigned integer`）
   - **Sampling Rate (Hz)**：`20000`（填你的 ISR 采样频率，横坐标会自动换算成真实时间秒 `s`！）
   - **Start Address**：`g_waveBufA`（直接填数组变量名！）
3. 点击 Graph 窗口右上角的 **`Continuous Refresh`**，即可像看示波器一样实时观察片内算法波形！

---

## 4. 利用硬件观察点 (Hardware Watchpoint) 抓捕“变量被莫名篡改”或栈溢出

如果某个全局变量 `g_status` 在运行中莫名其妙被改成了乱码（怀疑有野指针或数组越界踩踏），不需要大海捞针看代码：
1. 打开 **`View -> Breakpoints`** 窗口。
2. 点击左上角倒三角 $\rightarrow$ 选择 **`New -> Hardware Watchpoint`**。
3. **Location** 填 `&g_status`（或栈顶警戒地址），**Memory** 选择 **`Write`（发生写入时触发）**。
4. 全速运行程序，**一旦任何代码（或越界指针）往 `g_status` 地址写入数据，CPU 会在肇事指令的下一拍瞬间暂停**，直接抓获越界现场！
