#include "bldc_config.h"

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_rcc.h"

BLDC g_bldc;

void bldc_drven_gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct;

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    LL_GPIO_ResetOutputPin(BLDC_DRV_EN_GPIO_PORT, BLDC_DRV_EN_PIN);

    GPIO_InitStruct.Pin        = BLDC_DRV_EN_PIN;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_DOWN;
    LL_GPIO_Init(BLDC_DRV_EN_GPIO_PORT, &GPIO_InitStruct);
}

void bldc_pwm_init(void) {
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct = {0};

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

    TIM_InitStruct.Prescaler         = 36 - 1;
    TIM_InitStruct.CounterMode       = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload        = 100 - 1;
    TIM_InitStruct.ClockDivision     = LL_TIM_CLOCKDIVISION_DIV1;
    TIM_InitStruct.RepetitionCounter = 0;
    LL_TIM_Init(TIM1, &TIM_InitStruct);
    LL_TIM_EnableARRPreload(TIM1);
    LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);

    TIM_OC_InitStruct.OCMode       = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCState      = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCNState     = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.CompareValue = 0;
    TIM_OC_InitStruct.OCPolarity   = LL_TIM_OCPOLARITY_HIGH;
    TIM_OC_InitStruct.OCNPolarity  = LL_TIM_OCPOLARITY_HIGH;
    TIM_OC_InitStruct.OCIdleState  = LL_TIM_OCIDLESTATE_LOW;
    TIM_OC_InitStruct.OCNIdleState = LL_TIM_OCIDLESTATE_LOW;

    LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
    LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH1);
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH2);

    LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
    LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH2);
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH3);

    LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH3, &TIM_OC_InitStruct);
    LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH3);
    LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);

    LL_TIM_DisableMasterSlaveMode(TIM1);
    TIM_BDTRInitStruct.OSSRState       = LL_TIM_OSSR_DISABLE;
    TIM_BDTRInitStruct.OSSIState       = LL_TIM_OSSI_DISABLE;
    TIM_BDTRInitStruct.LockLevel       = LL_TIM_LOCKLEVEL_OFF;
    TIM_BDTRInitStruct.DeadTime        = 0;
    TIM_BDTRInitStruct.BreakState      = LL_TIM_BREAK_DISABLE;
    TIM_BDTRInitStruct.BreakPolarity   = LL_TIM_BREAK_POLARITY_HIGH;
    TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_DISABLE;
    LL_TIM_BDTR_Init(TIM1, &TIM_BDTRInitStruct);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    /**TIM1 GPIO Configuration
    PA8     ------> TIM1_CH1
    PA9     ------> TIM1_CH2
    PA10    ------> TIM1_CH3
    */
    GPIO_InitStruct.Pin = BLDC_PWMA_PIN | BLDC_PWMB_PIN | BLDC_PWMC_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(BLDC_PWMx_GPIO_PORT, &GPIO_InitStruct);
}

static void bldc_start_pwm_output(void) {
    LL_GPIO_SetOutputPin(BLDC_DRV_EN_GPIO_PORT, BLDC_DRV_EN_PIN);
    LL_TIM_SetCounter(TIM1, 0);
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2);
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH3);
    LL_TIM_EnableCounter(TIM1);
    LL_TIM_EnableAllOutputs(TIM1);
}

static void bldc_stop_pwm_output(void) {
    LL_GPIO_ResetOutputPin(BLDC_DRV_EN_GPIO_PORT, BLDC_DRV_EN_PIN);
    LL_TIM_DisableAllOutputs(TIM1);
    LL_TIM_DisableCounter(TIM1);
    LL_TIM_SetCounter(TIM1, 0);
    LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH2);
    LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH3);
}

void bldc_init(void) {
    bldc_drven_gpio_init();
    bldc_pwm_init();
    // close PWM output at first to protect user
    bldc_stop_pwm_output();

    g_bldc.start_pwm = bldc_start_pwm_output;
    g_bldc.stop_pwm = bldc_stop_pwm_output;
}
