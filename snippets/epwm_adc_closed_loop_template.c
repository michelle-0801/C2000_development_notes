/**
 * @file    epwm_adc_closed_loop_template.c
 * @brief   C2000 实时闭环控制标准工程脚手架 (ePWM1 20kHz 互补发波 + ADCA 同步采样 + RAM ISR)
 * @details
 *   硬件链路：
 *     1. ePWM1 工作在增减计数模式 (20 kHz)，输出互补对称带 1.0us 死区的 ePWM1A (GPIO0) & ePWM1B (GPIO1)；
 *     2. 每当 ePWM1 时基计数器到达波谷 (TBCTR = ZERO) 时，自动发出 SOCA 脉冲触发 ADCA_SOC0；
 *     3. ADCA 采样 ADCINA0 引脚，转换完成后触发 INT_ADCA1 中断；
 *     4. 驻留在零等待 RAM (.TI.ramfunc) 中的 adcA1ISR 读取采样值，更新 ePWM1 CMPA 占空比并清除中断标志。
 */

#include "driverlib.h"
#include "device.h"

// ============================================================================
// 宏定义参数 (假设 EPWMCLK = 100 MHz)
// ============================================================================
#define PWM_SWITCHING_FREQ_HZ   20000UL
#define EPWM_CLK_FREQ_HZ        (DEVICE_SYSCLK_FREQ / 2UL)               // 100 MHz
#define EPWM1_TIMER_TBPRD       (uint16_t)(EPWM_CLK_FREQ_HZ / (2UL * PWM_SWITCHING_FREQ_HZ)) // 2500
#define EPWM1_DEADBAND_TICKS    100U                                     // 100 * 10ns = 1.0us 死区

// ============================================================================
// 全局调试与控制变量 (主循环与 ISR 共享变量必须加 volatile!)
// ============================================================================
volatile uint16_t g_adcRawValue = 0U;
volatile float    g_dutyCyclePu = 0.5f;   // 占空比给定 (0.05f ~ 0.95f)，可在 CCS Expressions 实时修改

// 函数前向声明
void initEPWM1_GPIO(void);
void initEPWM1_Complementary(void);
void initADCA_TriggeredByEPWM1(void);
__attribute__((ramfunc)) __interrupt void adcA1ControlISR(void);

// ============================================================================
// 主函数入口
// ============================================================================
void main(void)
{
    // 1. 初始化系统时钟 (200MHz)、看门狗、拷贝 .TI.ramfunc 到 RAM 并初始化 Flash 等待周期
    Device_init();

    // 2. 解锁 GPIO 并配置 ePWM1A (GPIO0) 和 ePWM1B (GPIO1) 引脚复用
    Device_initGPIO();
    initEPWM1_GPIO();

    // 3. 初始化 PIE 中断控制器与默认向量表，注册 ADCA1 中断服务函数
    Interrupt_initModule();
    Interrupt_initVectorTable();
    Interrupt_register(INT_ADCA1, &adcA1ControlISR);

    // 4. 冻结 TBCLKSYNC 总开关，初始化 ePWM1 与 ADCA，再合闸启动 TBCLKSYNC
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
    initEPWM1_Complementary();
    initADCA_TriggeredByEPWM1();
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    // 5. 开启 PIE 通道与 CPU 全局中断 + 实时仿真调试模式
    Interrupt_enable(INT_ADCA1);
    EINT;
    ERTM;

    // 6. 后台主循环（处理状态机、通信或慢速监控任务）
    for(;;)
    {
        NOP;
    }
}

// ============================================================================
// GPIO0 / GPIO1 复用为 ePWM1A / ePWM1B
// ============================================================================
void initEPWM1_GPIO(void)
{
    GPIO_setPinConfig(GPIO_0_EPWM1A);
    GPIO_setPadConfig(0U, GPIO_PIN_TYPE_STD);

    GPIO_setPinConfig(GPIO_1_EPWM1B);
    GPIO_setPadConfig(1U, GPIO_PIN_TYPE_STD);
}

// ============================================================================
// 配置 ePWM1：20kHz 中心对齐、AHC 互补带 1us 死区、在 ZERO 处触发 ADC SOCA
// ============================================================================
void initEPWM1_Complementary(void)
{
    // 1. 时基 (TB)：不分频 (100MHz)，增减对称计数，周期 2500
    EPWM_setClockPrescaler(EPWM1_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
    EPWM_setTimeBasePeriod(EPWM1_BASE, EPWM1_TIMER_TBPRD);
    EPWM_setTimeBaseCounter(EPWM1_BASE, 0U);
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_disablePhaseShiftLoad(EPWM1_BASE);

    // 2. 计数比较 (CC)：初始 50% 占空比，在 TBCTR = ZERO 处从影子寄存器重载
    EPWM_setCounterCompareShadowLoadMode(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, EPWM1_TIMER_TBPRD / 2U);

    // 3. 动作限定 (AQ)：增计数遇 CMPA 置低，减计数遇 CMPA 置高
    EPWM_setActionQualifierAction(EPWM1_BASE, EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(EPWM1_BASE, EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

    // 4. 死区 (DB)：高有效互补模式 (AHC)，以 ePWM1A 为源自动生成互补带死区的 ePWM1A/1B
    EPWM_setDeadBandDelayMode(EPWM1_BASE, EPWM_DB_RED, true);
    EPWM_setDeadBandDelayMode(EPWM1_BASE, EPWM_DB_FED, true);
    EPWM_setDeadBandDelayPolarity(EPWM1_BASE, EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH);
    EPWM_setDeadBandDelayPolarity(EPWM1_BASE, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);
    EPWM_setDeadBandInputMode(EPWM1_BASE, EPWM_DB_RED, EPWM_DB_INPUT_EPWMA);
    EPWM_setDeadBandInputMode(EPWM1_BASE, EPWM_DB_FED, EPWM_DB_INPUT_EPWMA);
    EPWM_setRisingEdgeDelayCount(EPWM1_BASE, EPWM1_DEADBAND_TICKS);
    EPWM_setFallingEdgeDelayCount(EPWM1_BASE, EPWM1_DEADBAND_TICKS);

    // 5. 事件触发 (ET)：每 1 个周期在 TBCTR = ZERO 处发送一次 SOCA 脉冲给 ADC
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO);
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1U);
}

// ============================================================================
// 配置 ADCA：12-bit 单端模式，由 ePWM1 SOCA 触发 SOC0 采样 ADCINA0
// ============================================================================
void initADCA_TriggeredByEPWM1(void)
{
    // 1. 设置 ADCCLK = SYSCLK / 4 = 50 MHz，配置为 12位单端模式
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);
    ADC_setMode(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_CONV);

    // 2. 开启 ADC 电源并延时 1ms 等待模拟电路稳定
    ADC_enableConverter(ADCA_BASE);
    DEVICE_DELAY_US(1000U);

    // 3. 配置 SOC0：由 ePWM1 SOCA 触发，采样通道 ADCIN0，采样保持窗口 20 个 SYSCLK (100ns)
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN0, 20U);

    // 4. 将 SOC0 转换完成信号绑定到 ADCINT1 并使能中断
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
}

// ============================================================================
// 20 kHz 实时控制环路中断服务函数 (驻留零等待 RAM 运行)
// ============================================================================
__attribute__((ramfunc))
__interrupt void adcA1ControlISR(void)
{
    // 1. 读取 ADCA SOC0 采样结果 (注意传 ADCARESULT_BASE!)
    g_adcRawValue = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);

    // 2. 执行控制算法并对占空比执行无分支硬件限幅 [0.05, 0.95]
    float clampedDuty = __fsat(g_dutyCyclePu, 0.95f, 0.05f);
    uint16_t cmpaVal  = (uint16_t)((1.0f - clampedDuty) * (float)EPWM1_TIMER_TBPRD);

    // 3. 更新 ePWM1 CMPA 影子寄存器（将在下一个周期過零点自动生效）
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, cmpaVal);

    // 4. 清除 ADCA INT1 标志（处理潜在溢出）与 PIE Group 1 应答位
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    if (ADC_getInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1))
    {
        ADC_clearInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}
