/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
#include "main.h"
#include "bldc_config.h"
#include "bldc_test.h"
#include "current_monitor.h"
#include "encoder.h"
#include "foc.h"
#include "foc_app.h"
#include "pid.h"
#include "led.h"
#include "oled.h"
#include "qfplib-m3.h"
#include "vofa_usart.h"
#include "gui.h"


#define FOC_MOTOR_INIT          (1)
#define USART_FOC_CONTROL_INFO  (0)
#define USART_FOC_DQ_INFO       (0)
#define USART_FOC_CURRENT_INFO  (0)

void SystemClock_Config(void);

float foc_debugger_buf[3];

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    /* System interrupt init*/
    // NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    /* SysTick_IRQn interrupt configuration */
    // NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

    /** NOJTAG: JTAG-DP Disabled and SW-DP Enabled
     */
    // LL_GPIO_AF_Remap_SWJ_NOJTAG();

    /* Configure the system clock */
    SystemClock_Config();

    //* Initialize all configured peripherals start
    led_init();
    vofa_usart_init();

#if FOC_MOTOR_INIT
    bldc_init();
    encoder_init();
    pid_init();
    foc_init();
    // Motor alignment start!
    LL_mDelay(500);
    g_foc.align_sensor();
    foc_app_init();

    // Current monitor will cause OLED display abnormally
    current_mointor_init();
    g_foc.state_.init_done = TRUE;
#endif
    oled_init();
    gui_init();

    //* Initialize all configured peripherals end

    /* Infinite loop */
#if USART_FLOAT_TEST
    float buf[3];
    float cnt1 = 0.0f;
    float cnt2 = 0.0f;
#endif
    while (1) {
#if USART_FLOAT_TEST
        // ------------ USART test ----------------
        cnt1 = qfp_fadd(cnt1, 0.01);
        cnt2 = qfp_fadd(cnt2, 0.02);
        if (cnt1 > 20.0f) {
            cnt1 = 0.0;
        }
        if (cnt2 > 40.0f) {
            cnt2 = 0.0;
        }
        buf[0] = cnt1;
        buf[1] = cnt2;
        vofa_usart_dma_send_config(buf, 2);
        LL_mDelay(10);
#endif
        // NOTE: close TIM2 interrupt if you want to use the test below
        // ------------ BLDC Motor test -----------------
        // bldc_test1_invariant_duty();
        // bldc_test2_svpwm();
        // bldc_test3_svpwm_with_angle();

        // print out control info
#if USART_FOC_CONTROL_INFO
        foc_debugger_buf[0] = g_foc.state_.shaft_angle;
        foc_debugger_buf[1] = g_foc.state_.shaft_speed;
        foc_debugger_buf[2] = g_foc.state_.electrical_angle;
        vofa_usart_dma_send_config(foc_debugger_buf, 3);
#elif USART_FOC_DQ_INFO
        foc_debugger_buf[0] = g_foc.state_.q;
        foc_debugger_buf[1] = g_foc.state_.d;
        vofa_usart_dma_send_config(foc_debugger_buf, 2);
#elif USART_FOC_CURRENT_INFO
        foc_debugger_buf[0] = g_foc.state_.I.a;
        foc_debugger_buf[1] = g_foc.state_.I.b;
        foc_debugger_buf[2] = g_foc.state_.I.c;
        vofa_usart_dma_send_config(foc_debugger_buf, 3);
#endif
        LL_mDelay(5);
        // ------------ Encoder test ------------
        // encoder_test();
        // ------------ Current Monitor test ------------
        // current_monitor_test(bldc_test2_svpwm());
        // ------------ GUI test ------------
        gui_render();
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2) {
    }
    LL_RCC_HSE_Enable();

    /* Wait till HSE is ready */
    while (LL_RCC_HSE_IsReady() != 1) {
    }
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);
    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL_IsReady() != 1) {
    }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
    }
    LL_Init1msTick(72000000);
    LL_SetSystemCoreClock(72000000);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
