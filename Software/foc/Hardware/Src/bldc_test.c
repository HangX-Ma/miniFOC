#include "bldc_test.h"
#include "bldc_config.h"
#include "qfplib-m3.h"
#include "vofa_usart.h"
#include "config.h"

#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_utils.h"

/* ----------------- TEST CODE BELOW ----------------- */

//* TEST1 start
#define TEST1_PWMx_DUTY                 (uint32_t)(PWM_RELOAD_PERIOD/100 * 5)
#define TEST1_PWMA_DUTY_ZERO            LL_TIM_OC_SetCompareCH1(TIM1, 0)
#define TEST1_PWMB_DUTY_ZERO            LL_TIM_OC_SetCompareCH2(TIM1, 0)
#define TEST1_PWMC_DUTY_ZERO            LL_TIM_OC_SetCompareCH3(TIM1, 0)
#define TEST1_PWMA_DUTY_FIXED           LL_TIM_OC_SetCompareCH1(TIM1, TEST1_PWMx_DUTY)
#define TEST1_PWMB_DUTY_FIXED           LL_TIM_OC_SetCompareCH2(TIM1, TEST1_PWMx_DUTY)
#define TEST1_PWMC_DUTY_FIXED           LL_TIM_OC_SetCompareCH3(TIM1, TEST1_PWMx_DUTY)

void bldc_test1_invariant_duty(void) {
    // phrase1
    TEST1_PWMA_DUTY_FIXED;
    TEST1_PWMB_DUTY_ZERO;
    TEST1_PWMC_DUTY_ZERO;
    LL_mDelay(10);
    // phrase2
    TEST1_PWMA_DUTY_ZERO;
    TEST1_PWMB_DUTY_FIXED;
    TEST1_PWMC_DUTY_ZERO;
    LL_mDelay(10);
    // phrase3
    TEST1_PWMA_DUTY_ZERO;
    TEST1_PWMB_DUTY_ZERO;
    TEST1_PWMC_DUTY_FIXED;
    LL_mDelay(10);
}
//* TEST1 end

//* TEST2 start
#include "foc.h"
static float electric_angle = 0.0f;
void bldc_test2_svpwm(void) {
    setPhaseVoltage(1.0f, 0.0f, electric_angle); // Uq < 2.0
    electric_angle = qfp_fadd(electric_angle, 0.1f);
    LL_mDelay(1);
}

//* TEST2 end