/**
 * @file foc.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief FOC core
 * @version 0.1
 * @date 2023-07-22
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


#ifndef __FOC__H__
#define __FOC__H__

#include "utils.h"

typedef struct {
    uint8_t pole_pairs;
} FOCProperty;

typedef struct {
    FOCProperty property_;
    // methods
    void (*set_phase_voltage)(float, float, float);
    float (*get_electrical_angle)(float);
    void (*align_sensor)(void);
} FOC;

extern FOC g_foc;

void foc_init();

#endif  //!__FOC__H__