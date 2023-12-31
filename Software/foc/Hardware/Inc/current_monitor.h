/**
 * @file current_monitor.h
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

#ifndef __CURRENT_MONITOR__H__
#define __CURRENT_MONITOR__H__

#include "utils.h"

#define ADCx_VOLTAGE_REFERENCE                  3.3f
#define ADCx_VOLTAGE_BIAS                       1.69f
#define ADCx_RESOLUTION                         4095

#define CURRENT_SENSE_REGISTER                  0.01f
#define INA199x1_GAIN                           50

#define CURRENT_MONITOR_ADC_GPIO_PORT           GPIOA
#define CURRENT_MONITOR_ADCx_IN0_PIN            LL_GPIO_PIN_0
#define CURRENT_MONITOR_ADCx_IN1_PIN            LL_GPIO_PIN_1
#define CURRENT_MONITOR_ADCx_IN2_PIN            LL_GPIO_PIN_2
#define CURRENT_MONITOR_ADCx_DMAx_CHANNEL       LL_DMA_CHANNEL_1
#define CURRENT_MONITOR_ADCx_IRQHandler         DMA1_Channel1_IRQHandler

#define ADCx_CHANNEL_NUM                        3

typedef struct CurrentMonitorADC {
    uint16_t chx[ADCx_CHANNEL_NUM];
} CurrentMonitorADC;

typedef struct PhaseCurrent {
    float Ia;
    float Ib;
    float Ic;
} PhaseCurrent;

typedef struct RotorStatorCurrent {
    float Id;
    float Iq;
} RotorStatorCurrent;

extern RotorStatorCurrent g_RS_current;

RotorStatorCurrent get_RS_current(float e_angle);
void current_monitor_reset(void);
void current_mointor_init(void);
void current_monitor_test(float e_angle);

#endif  //!__CURRENT_MONITOR__H__