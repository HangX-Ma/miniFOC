/**
 * @file c_monitor.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief INA199 current-shunt monitors
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

#ifndef __C_MONITOR__H__
#define __C_MONITOR__H__

#include "utils.h"

#define C_MONITOR_ADC_GPIO_PORT         GPIOA
#define C_MONITOR_ADCx_IN0_PIN          LL_GPIO_PIN_0
#define C_MONITOR_ADCx_IN1_PIN          LL_GPIO_PIN_1

typedef struct CurrentMonitor {
    uint32_t ch1;
    uint32_t ch2;
} CurrentMonitor;

extern CurrentMonitor g_current_monitor;

void c_mointor_init();

#endif  //!__C_MONITOR__H__