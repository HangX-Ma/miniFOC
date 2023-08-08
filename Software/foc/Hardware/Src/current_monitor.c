#include "current_monitor.h"

#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_utils.h"

#include "delay.h"
#include "qfplib-m3.h"
#include "vofa_usart.h"

// ref: STM32F0使用LL库实现DMA方式AD采集 <https://blog.51cto.com/u_520887/5290076>
// ref: STM32L476多通道TIM+DMA+ADC采样（LL库） <https://codeantenna.com/a/1MnFm9oX2G>
// ref: STM32 定时器触发 ADC 多通道采集，DMA搬运至内存 <https://blog.51cto.com/u_15456236/4801335>
// ref: [STM32] HAL库 STM32CubeMX教程九---ADC <https://www.guyuehome.com/36010>

static CurrentMonitorADC current_monitor_adc;

// TIM3 is used to determine the current sampling frequency.
// The ADC will use TIM3 channel 3 as the external trigger signal.
// Only the rising edge will lead to ADC conversion.
// Note:
//      Regular Conversion launched by software 规则的软件触发 调用函数触发即可
//      Timer X Capture Compare X event 外部引脚触发,
//      Timer X Trigger Out event 定时器通道输出触发 需要设置相应的定时器设置
static void current_monitor_tim3_init(void) {
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

    // ADC sampling frequency: 72 MHz / 7200 / 2 = 5 KHz
    TIM_InitStruct.Prescaler       = 36 - 1;
    TIM_InitStruct.CounterMode     = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload      = 400 - 1;
    TIM_InitStruct.ClockDivision   = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM3, &TIM_InitStruct);

    LL_TIM_EnableARRPreload(TIM3);
    LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_INTERNAL);

    // PWM mode 1 with polarity low means before the reach of ARR,
    // the PWM will output effectively LOW.
    LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH2);
    TIM_OC_InitStruct.OCMode       = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCState      = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCNState     = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.CompareValue = 0;
    TIM_OC_InitStruct.OCPolarity   = LL_TIM_OCPOLARITY_LOW;
    LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
    LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH2);
    LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_UPDATE);
    LL_TIM_DisableMasterSlaveMode(TIM3);

    // We don't enable TIM2 until we have set DMA and ADC
    LL_TIM_DisableCounter(TIM3);
    LL_TIM_DisableAllOutputs(TIM3);
}

static void current_monitor_adc_dma_init(void) {
    /* DMA controller clock enable */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    /* DMA interrupt init */
    NVIC_SetPriority(DMA1_Channel1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    LL_DMA_InitTypeDef ADCx_DMA_InitStruct = {0};

    LL_DMA_DeInit(DMA1, CURRENT_MONITOR_ADCx_DMAx_CHANNEL);
    ADCx_DMA_InitStruct.PeriphOrM2MSrcAddress  = LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA);
    ADCx_DMA_InitStruct.MemoryOrM2MDstAddress  = (uint32_t)current_monitor_adc.chx;
    ADCx_DMA_InitStruct.Direction              = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    ADCx_DMA_InitStruct.Mode                   = LL_DMA_MODE_CIRCULAR;
    ADCx_DMA_InitStruct.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
    ADCx_DMA_InitStruct.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
    ADCx_DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
    ADCx_DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
    ADCx_DMA_InitStruct.NbData                 = ADCx_CHANNEL_NUM; // 3 channels, 3 half word
    ADCx_DMA_InitStruct.Priority               = LL_DMA_PRIORITY_HIGH;
    LL_DMA_Init(DMA1, CURRENT_MONITOR_ADCx_DMAx_CHANNEL, &ADCx_DMA_InitStruct);

    //* start DMA
    LL_DMA_EnableChannel(DMA1, CURRENT_MONITOR_ADCx_DMAx_CHANNEL);
}

static void current_monitor_adc_init(void) {
    LL_ADC_InitTypeDef ADC_InitStruct = {0};
    LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

    // avoid abnormal phenomenon
    LL_ADC_Disable(ADC1);

    /**ADC1 GPIO Configuration
        PA0-WKUP    ------> ADC1_IN0
        PA1         ------> ADC1_IN1
        PA2         ------> ADC1_IN2
    */
    GPIO_InitStruct.Pin  = CURRENT_MONITOR_ADCx_IN0_PIN | CURRENT_MONITOR_ADCx_IN1_PIN | CURRENT_MONITOR_ADCx_IN2_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    LL_GPIO_Init(CURRENT_MONITOR_ADC_GPIO_PORT, &GPIO_InitStruct);

    ADC_InitStruct.DataAlignment        = LL_ADC_DATA_ALIGN_RIGHT;
    ADC_InitStruct.SequencersScanMode   = LL_ADC_SEQ_SCAN_ENABLE;
    LL_ADC_Init(ADC1, &ADC_InitStruct);

    ADC_CommonInitStruct.Multimode      = LL_ADC_MULTI_INDEPENDENT;
    LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC1), &ADC_CommonInitStruct);

    ADC_REG_InitStruct.TriggerSource    = LL_ADC_REG_TRIG_EXT_TIM3_TRGO;
    ADC_REG_InitStruct.SequencerLength  = LL_ADC_REG_SEQ_SCAN_ENABLE_3RANKS;
    ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
    // otherwise, TIM3 cannot trigger ADC conversion after the first time trigger
    ADC_REG_InitStruct.ContinuousMode   = LL_ADC_REG_CONV_SINGLE;
    ADC_REG_InitStruct.DMATransfer      = LL_ADC_REG_DMA_TRANSFER_UNLIMITED;
    LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);

    /** Configure Regular Channel
     */
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_0);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_0, LL_ADC_SAMPLINGTIME_13CYCLES_5);

    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_1);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_1, LL_ADC_SAMPLINGTIME_13CYCLES_5);

    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_3, LL_ADC_CHANNEL_2);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_2, LL_ADC_SAMPLINGTIME_13CYCLES_5);
}

void current_mointor_init(void) {
    current_monitor_tim3_init();
    current_monitor_adc_dma_init();
    current_monitor_adc_init();

    //* start ADC1
    LL_ADC_Enable(ADC1);
    // wait until internal voltage reference stable
    delay_nus_72MHz(LL_ADC_DELAY_TEMPSENSOR_STAB_US);

    // wait at least 2 ADC cycles after ADC power-on but before calibration
    LL_mDelay(10);
    // wait until ADC calibration done
    LL_ADC_StartCalibration(ADC1);
    while (LL_ADC_IsCalibrationOnGoing(ADC1) != RESET) {}
    // rising edge trigger
    LL_ADC_REG_StartConversionExtTrig(ADC1, LL_ADC_REG_TRIG_EXT_RISING);

    //* start TIM3
    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM3);
    LL_TIM_EnableAllOutputs(TIM3);

    LL_mDelay(1);
}

static void LPF_current(float *curr, float *prev) {
    *curr = qfp_fadd(qfp_fmul(0.9f, *prev), qfp_fmul(0.1f, *curr));
    *prev = *curr;
}

typedef struct {
    float input_curr;
    float input_prev;
    float output_curr;
    float output_prev;
    float alpha;
} HighPassFilter;
static HighPassFilter hpf_adc1, hpf_adc2;

static void hpf_reset(HighPassFilter *hpf) {
    hpf->input_curr  = 0.0f;
    hpf->input_prev  = 0.0f;
    hpf->output_curr = 0.0f;
    hpf->output_prev = 0.0f;
    hpf->alpha       = 0.75f;
}

void HPF_calculation(HighPassFilter *hpf){
    hpf->output_curr =
        qfp_fadd(
            qfp_fmul(hpf->alpha, hpf->output_prev),
            qfp_fmul(hpf->alpha, qfp_fsub(hpf->input_curr, hpf->input_prev))
        );

    hpf->output_prev = hpf->output_curr;
    hpf->input_prev = hpf->input_curr;
}

#include "foc.h"
// rotor and stator current (Iq and Id)
static RotorStatorCurrent RS_current_prev = {0};
RotorStatorCurrent get_RS_current(float e_angle) {
    PhaseCurrent phase_current;
    RotorStatorCurrent RS_current_curr;
    float I_alpha, I_beta;
    float adc1, adc2, adc3;

    adc1 = qfp_fdiv(qfp_fmul((float)current_monitor_adc.chx[0], ADCx_VOLTAGE_REFERENCE), (float)ADCx_RESOLUTION);
    adc2 = qfp_fdiv(qfp_fmul((float)current_monitor_adc.chx[1], ADCx_VOLTAGE_REFERENCE), (float)ADCx_RESOLUTION);
    adc3 = qfp_fdiv(qfp_fmul((float)current_monitor_adc.chx[2], ADCx_VOLTAGE_REFERENCE), (float)ADCx_RESOLUTION);

    // debug
    g_foc.state_.I.a = adc1;
    g_foc.state_.I.b = adc2;
    g_foc.state_.I.c = adc3;

    // debug
    // g_foc.state_.q   = hpf_adc1.output_curr;
    // g_foc.state_.d   = hpf_adc2.output_curr;

    phase_current.Ia = qfp_fdiv(qfp_fdiv((qfp_fsub(adc1, ADCx_VOLTAGE_BIAS)), CURRENT_SENSE_REGISTER), INA199x1_GAIN);
    phase_current.Ib = qfp_fdiv(qfp_fdiv((qfp_fsub(adc2, ADCx_VOLTAGE_BIAS)), CURRENT_SENSE_REGISTER), INA199x1_GAIN);

    // debug
    // g_foc.state_.q   = phase_current.Ia;
    // g_foc.state_.d   = phase_current.Ib;

    I_alpha = phase_current.Ia;
    I_beta  = qfp_fadd(qfp_fmul(_1_SQRT3, phase_current.Ia), qfp_fmul(_2_SQRT3, phase_current.Ib));

    // debug
    // g_foc.state_.q   = I_alpha;
    // g_foc.state_.d   = I_beta;

    RS_current_curr.Id = qfp_fadd(
                           qfp_fmul(I_alpha, qfp_fcos(e_angle)),
                           qfp_fmul(I_beta, qfp_fsin(e_angle))
                         );

    RS_current_curr.Iq = qfp_fsub(
                           qfp_fmul(I_beta, qfp_fcos(e_angle)),
                           qfp_fmul(I_alpha, qfp_fsin(e_angle))
                         );
    LPF_current(&RS_current_curr.Id, &RS_current_prev.Id);
    LPF_current(&RS_current_curr.Iq, &RS_current_prev.Iq);

    // debug
    // g_foc.state_.q = RS_current_curr.Iq;
    // g_foc.state_.d = RS_current_curr.Id;

    return RS_current_curr;
}

void current_monitor_reset(void) {
    RS_current_prev.Id = 0.0f;
    RS_current_prev.Iq = 0.0f;

    hpf_reset(&hpf_adc1);
    hpf_reset(&hpf_adc2);
}

static float vofa_buf[2];
void current_monitor_test(float e_angle) {
    RotorStatorCurrent current = get_RS_current(e_angle);

    vofa_buf[0] = current.Id;
    vofa_buf[1] = current.Iq;
    vofa_usart_dma_send_config(vofa_buf, 2);

    LL_mDelay(1);
}