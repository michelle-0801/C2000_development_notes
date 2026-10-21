# 可复用代码片段与配置模板 (`snippets/`)

本目录用于存放经过验证的独立代码片段、自定义 `.cmd` 链接脚本模板以及外设初始化脚手架，方便在新工程中直接参考或复用。

## 📂 建议存放内容

- `cmd_templates/`：注释详尽的 RAM / Flash 链接脚本模板（如自定义 `GSx` 分配、`.TI.ramfunc` 标准写法）
- `peripheral_init/`：最小化、清晰的 DriverLib / Bitfield 外设初始化代码段（如 ePWM 互补带死区发波 + ADC 同步采样模板）
- `utilities/`：针对 C28x 16-bit `char` 编写的字节流打包/解包工具函数、环形缓冲区 (Ring Buffer) 等
