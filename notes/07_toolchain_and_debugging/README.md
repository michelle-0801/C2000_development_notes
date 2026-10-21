# 07 - 工具链、编译器与调试排错 (`07_toolchain_and_debugging`)

本目录汇总 TI C2000-CGT 编译器高阶特性、CCS (Code Composer Studio) 调试工具技巧以及常见疑难 Bug 排查手册。

## 📝 推荐笔记规划

- `01_c2000_cgt_compiler_tips.md`
  - 常用 `#pragma` 指令速查：`CODE_SECTION`、`DATA_SECTION`、`MUST_ITERATE`、`UNROLL`
  - C28x 常用编译器内联函数 (Intrinsics)：`__byte()`、`__eallow()`、`__edis()`、`__fmax()`、`__sinpuf32()`
  - COFF 与 EABI 二进制格式的差异（段命名、未初始化变量默认行为、类型大小差异）
- `02_ccs_debugging_techniques.md`
  - **Real-time Debug Mode（实时仿真模式）**：不暂停 PWM/控制环路在线刷新观察变量
  - Memory Browser 与 Expressions 窗口的高效使用（Q 格式显示、连续刷新）
  - Graph 工具绘制片上缓存波形与导出数据分析
- `03_troubleshooting_and_pitfalls.md`
  - 进入 `ILLEGAL_ISR` (ITRAP0) 的常见原因与定位方法（栈溢出、未搬移 `.TI.ramfunc` 就调用 Flash 初始化、野指针）
  - 优化等级 (`-O0` ~ `-O4`) 导致的 `volatile` 缺失问题排查
