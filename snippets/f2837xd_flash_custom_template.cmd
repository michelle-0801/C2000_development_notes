/*
 * ============================================================================
 * 文件名: f2837xd_flash_custom_template.cmd
 * 适用芯片: TMS320F28379D / F28377D (CPU1 Flash 独立运行工程模板)
 * 特性说明:
 *   1. 同时兼容 COFF (coffabi) 与 EABI (eabi) 编译器模式；
 *   2. 已将 RAMLS0 ~ RAMLS2 合并为连续 6K Words (RAMLS0_2)，专门容纳高频 .TI.ramfunc；
 *   3. 已将 RAMD0 ~ RAMD1 合并为 4K Words，提供宽裕的 2K Words 系统栈 (.stack)；
 *   4. 预留了 DMA 专用的 ramgs0 段以及 CLA 消息 RAM 映射。
 * ============================================================================
 */

MEMORY
{
PAGE 0 :  /* Program Memory (程序与常量空间) */
   /* Flash 启动固定入口地址 (codestart 必须放在此处) */
   BEGIN               : origin = 0x080000, length = 0x000002

   /* 合并 LS0~LS2 (共 6K x 16) 作为零等待周期高频代码运行区 (.TI.ramfunc) */
   RAMLS0_2            : origin = 0x008000, length = 0x001800

   /* 片上 Flash 扇区 (Sector A ~ Sector N，共 256K x 16) */
   FLASHA              : origin = 0x080002, length = 0x001FFE
   FLASHB_C            : origin = 0x082000, length = 0x004000 /* 合并 B&C (16K Words) 放主代码 */
   FLASHD              : origin = 0x086000, length = 0x002000 /* 8K Words 专门存放 .TI.ramfunc 加载镜像 */
   FLASHE              : origin = 0x088000, length = 0x008000 /* 32K Words */
   FLASHF              : origin = 0x090000, length = 0x008000 /* 32K Words */
   FLASHG              : origin = 0x098000, length = 0x008000 /* 32K Words */

   RESET               : origin = 0x3FFFC0, length = 0x000002

PAGE 1 :  /* Data Memory (全局变量、栈、DMA与多核共享空间) */
   BOOT_RSVD           : origin = 0x000002, length = 0x000120 /* Boot ROM 保留栈区，请勿使用 */
   RAMM0               : origin = 0x000122, length = 0x0002DE /* 0.7K Words */
   RAMM1               : origin = 0x000400, length = 0x000400 /* 1K Words */

   /* 合并 D0 & D1 (共 4K x 16，带 ECC 校验，位于低 64K 内，极适合放大容量 .stack) */
   RAMD0_1             : origin = 0x00B000, length = 0x001000

   /* 合并 LS3 ~ LS5 (共 6K x 16) 存放普通全局变量 (.ebss / .bss) */
   RAMLS3_5            : origin = 0x009800, length = 0x001800

   /* 全局共享 RAM (支持 DMA 直接访问 & CPU1/CPU2 共享) */
   RAMGS0              : origin = 0x00C000, length = 0x001000 /* 4K Words: 专供 CPU1 DMA 缓冲区 */
   RAMGS1_7            : origin = 0x00D000, length = 0x007000 /* 28K Words: 大容量波形缓存或全局数组 */
   RAMGS8_15           : origin = 0x014000, length = 0x008000 /* 32K Words: 可移交给 CPU2 主控 */

   /* CLA 与双核 IPC 消息 RAM */
   CLA1_MSGRAMLOW      : origin = 0x001480, length = 0x000080 /* Cla1ToCpuMsgRAM (128 Words) */
   CLA1_MSGRAMHIGH     : origin = 0x001500, length = 0x000080 /* CpuToCla1MsgRAM (128 Words) */
   CPU2TOCPU1RAM       : origin = 0x03F800, length = 0x000400 /* 1K Words */
   CPU1TOCPU2RAM       : origin = 0x03FC00, length = 0x000400 /* 1K Words */
}


SECTIONS
{
   /* 1. 启动入口与主程序代码段 (必须 8-Word 128-bit 对齐以匹配 Flash ECC) */
   codestart           : > BEGIN,                   PAGE = 0, ALIGN(8)
   .text               : >> FLASHB_C | FLASHE | FLASHF, PAGE = 0, ALIGN(8)
   .cinit              : > FLASHA,                  PAGE = 0, ALIGN(8)
   .switch             : > FLASHA,                  PAGE = 0, ALIGN(8)
   .reset              : > RESET,                   PAGE = 0, TYPE = DSECT

   /* 2. 兼容 EABI 与 COFF 的常量与全局变量段 */
#if defined(__TI_EABI__)
   .init_array         : > FLASHA,                  PAGE = 0, ALIGN(8)
   .const              : >> FLASHA | FLASHE,        PAGE = 0, ALIGN(8)
   .bss                : > RAMLS3_5,                PAGE = 1
   .bss:output         : > RAMLS0_2,                PAGE = 0
   .bss:cio            : > RAMM0,                   PAGE = 1
   .data               : > RAMLS3_5,                PAGE = 1
   .sysmem             : > RAMM1,                   PAGE = 1
#else
   .pinit              : > FLASHA,                  PAGE = 0, ALIGN(8)
   .econst             : >> FLASHA | FLASHE,        PAGE = 0, ALIGN(8)
   .ebss               : >> RAMLS3_5 | RAMGS1_7,    PAGE = 1
   .esysmem            : > RAMM1,                   PAGE = 1
#endif

   /* 3. 系统堆栈：放在合并后的 RAMD0_1 (可在工程属性将 --stack_size 设为 0x800 即 2K Words) */
   .stack              : > RAMD0_1,                 PAGE = 1

   /* 4. 高速 RAM 运行函数段 (.TI.ramfunc)：加载于 FLASHD，上电由 memcpy 搬入 RAMLS0_2 零等待运行 */
   .TI.ramfunc         : {} LOAD = FLASHD,
                            RUN = RAMLS0_2,
#if defined(__TI_EABI__)
                            LOAD_START(RamfuncsLoadStart),
                            LOAD_SIZE(RamfuncsLoadSize),
                            LOAD_END(RamfuncsLoadEnd),
                            RUN_START(RamfuncsRunStart),
                            RUN_SIZE(RamfuncsRunSize),
                            RUN_END(RamfuncsRunEnd),
#else
                            LOAD_START(_RamfuncsLoadStart),
                            LOAD_SIZE(_RamfuncsLoadSize),
                            LOAD_END(_RamfuncsLoadEnd),
                            RUN_START(_RamfuncsRunStart),
                            RUN_SIZE(_RamfuncsRunSize),
                            RUN_END(_RamfuncsRunEnd),
#endif
                            PAGE = 0, ALIGN(8)

   /* 5. 自定义共享数据段（配合 C 代码中的 #pragma DATA_SECTION 使用） */
   ramgs0              : > RAMGS0,                  PAGE = 1
   Cla1ToCpuMsgRAM     : > CLA1_MSGRAMLOW,          PAGE = 1
   CpuToCla1MsgRAM     : > CLA1_MSGRAMHIGH,         PAGE = 1
   MSGRAM_CPU1_TO_CPU2 : > CPU1TOCPU2RAM,           PAGE = 1
   MSGRAM_CPU2_TO_CPU1 : > CPU2TOCPU1RAM,           PAGE = 1
}
