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
#include "tim.h"

#include "encoder.h"
#include "led.h"
#include "oled.h"
#include "qfplib-m3.h"
#include "vofa_usart.h"

#define duty (uint32_t)10
#define A0 LL_TIM_OC_SetCompareCH1(TIM1, 0)
#define B0 LL_TIM_OC_SetCompareCH2(TIM1, 0)
#define C0 LL_TIM_OC_SetCompareCH3(TIM1, 0)

#define A1 LL_TIM_OC_SetCompareCH1(TIM1, duty)
#define B1 LL_TIM_OC_SetCompareCH2(TIM1, duty)
#define C1 LL_TIM_OC_SetCompareCH3(TIM1, duty)


void SystemClock_Config(void);

float buf[3];

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
    MX_TIM1_Init();

    led_init();
    vofa_usart_init();
    oled_init();
    // encoder_init();

    // TIM1 PWM Generation
    // LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);
    // LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2);
    // LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH3);
    // LL_TIM_EnableCounter(TIM1);
    // LL_TIM_EnableAllOutputs(TIM1);
    //* Initialize all configured peripherals end

    /* Infinite loop */
    float cnt1 = 0.0f;
    float cnt2 = 0.0f;
    while (1) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
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
        // A1;
        // B0;
        // C0;
        // LL_mDelay(10);
        // A0;
        // B1;
        // C0;
        // LL_mDelay(10);
        // A0;
        // B0;
        // C1;
        // LL_mDelay(10);
        // LL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
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
