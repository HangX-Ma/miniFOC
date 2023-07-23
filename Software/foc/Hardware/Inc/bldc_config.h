/**
 * @file bldc_config.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief BLDC essential configurations
 * @version 0.1
 * @date 2023-07-21
 * @ref https://github.com/simplefoc/Arduino-FOC-drivers/tree/master/src/encoders/sc60228
 * @ref https://blog.csdn.net/weixin_43593122/article/details/119253544
 *
 * @copyright Copyright (c) 2023
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

#ifndef __BLDC_CONFIG__H__
#define __BLDC_CONFIG__H__

#include "utils.h"

#define BLDC_DRV_EN_GPIO_PORT       GPIOA
#define BLDC_DRV_EN_PIN             LL_GPIO_PIN_11

#define BLDC_PWMx_GPIO_PORT         GPIOA
#define BLDC_PWMA_PIN               LL_GPIO_PIN_8
#define BLDC_PWMB_PIN               LL_GPIO_PIN_9
#define BLDC_PWMC_PIN               LL_GPIO_PIN_10

typedef struct BLDC {
    void (*set_pwm_a_duty)(uint32_t);
    void (*set_pwm_b_duty)(uint32_t);
    void (*set_pwm_c_duty)(uint32_t);
    void (*start_pwm)(void);
    void (*stop_pwm)(void);
} BLDC;
extern BLDC g_bldc;

void bldc_init();

#endif  //!__BLDC_CONFIG__H__