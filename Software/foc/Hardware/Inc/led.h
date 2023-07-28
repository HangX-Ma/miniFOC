/**
 * @file led.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief led fundamental functions
 * @version 0.1
 * @date 2022-08-05
 *
 * @copyright Copyright (c) 2022 - 2023
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __LED__H__
#define __LED__H__

#include "utils.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_utils.h"

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
void led_init(void);

/**
 * @brief  Set LED to Blinking mode for an infinite loop (toggle period based on value provided as input parameter).
 * @param  Period : Period of time (in ms) between each toggling of LED
 *   This parameter can be user defined values. Pre-defined values used in that example are :
 *     @arg LED_BLINK_FAST : Fast Blinking
 *     @arg LED_BLINK_SLOW : Slow Blinking
 *     @arg LED_BLINK_ERROR : Error specific Blinking
 * @retval None
 */
void led_blinking(uint32_t Period);

#endif //!__LED__H__
