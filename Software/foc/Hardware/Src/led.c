/**
 * @file led.c
 * @author MContour (m-contour@qq.com)
 * @brief led fundamental functions
 * @version 0.1
 * @date 2022-08-05
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "led.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_utils.h"

void LED_GPIO_Config(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct;

    LED_GPIO_CLK_ENABLE();

    GPIO_InitStruct.Pin = LL_GPIO_PIN_13;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;

    LL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    LED_STATE_OFF();
}

void LED_Blinking(uint32_t Period) {
    /* Toggle LED2 in an infinite loop */
    while (1) {
        LL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        LL_mDelay(Period);
    }
}