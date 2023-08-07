/**
 * @file foc.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief FOC applications
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

#ifndef __FOC_APP__H__
#define __FOC_APP__H__

#include "utils.h"
#include "pid.h"

typedef struct FOCNormal {
    TorCtrlParam torque_ctrl_;
} FOCNormal;

typedef struct FOCRatchet {
    // torque target is the attractor angle
    TorCtrlParam torque_ctrl_;

    float attractor_num_;    ///< total attractor number in one circle
} FOCRatchet;

typedef struct FOCRebound {
    TorCtrlParam torque_ctrl_; // increase 'Kd' will add the damp intensity

    float rebound_angle_;
    float output_ratio_;        ///< output ratio, range [1, N]

    void (*update_output_ratio)(float /* output ratio */);
} FOCRebound;

typedef enum FOCAppMode {
    FOC_App_Normal_Mode,
    FOC_App_Ratchet_Mode,
    FOC_App_Rebound_Mode,
} FOCAppMode;

typedef struct FOCApp {
    FOCNormal normal_;
    FOCRatchet ratchet_;
    FOCRebound rebound_;

    FOCAppMode mode_;
} FOCApp;
extern FOCApp g_foc_app;

TorCtrlParam* torque_ratchet_mode(void);
TorCtrlParam* torque_rebound_mode(void);

void foc_app_init(void);


#endif  //!__FOC_APP__H__