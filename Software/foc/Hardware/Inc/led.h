/**
 * @file led.h
 * @author MContour (m-contour@qq.com)
 * @brief led fundamental functions
 * @version 0.1
 * @date 2022-08-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __LED__H__
#define __LED__H__

#include "stm32f1xx.h"

/**
 * @brief Toggle periods for various blinking modes
 */
#define LED_BLINK_FAST  200
#define LED_BLINK_SLOW  500
#define LED_BLINK_ERROR 1000

#define LED_GPIO_PORT           GPIOC

#define LED_GPIO_PIN            LL_GPIO_PIN_13

#define LED_GPIO_CLK_ENABLE()   LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC)

#define LED_STATE_ON()          LL_GPIO_ResetOutputPin(LED_GPIO_PORT, LED_GPIO_PIN)

#define LED_STATE_OFF()         LL_GPIO_SetOutputPin(LED_GPIO_PORT, LED_GPIO_PIN)

#define LED_STATE_TOGGLE()      LL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN)

/* FUNCTION */
void LED_GPIO_Config(void);

/**
 * @brief  Set LED to Blinking mode for an infinite loop (toggle period based on value provided as input parameter).
 * @param  Period : Period of time (in ms) between each toggling of LED
 *   This parameter can be user defined values. Pre-defined values used in that example are :
 *     @arg LED_BLINK_FAST : Fast Blinking
 *     @arg LED_BLINK_SLOW : Slow Blinking
 *     @arg LED_BLINK_ERROR : Error specific Blinking
 * @retval None
 */
void LED_Blinking(uint32_t Period);

#endif //!__LED__H__
