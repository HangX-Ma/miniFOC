/**
 * @file pid.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief PID control set
 * @version 0.1
 * @date 2023-07-29
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

#ifndef __PID__H__
#define __PID__H__

#include "utils.h"

typedef struct PIDParam {
    float Kp;
    float Ki;
    float Kd;
} PIDParam;

typedef struct PIDPrevData {
    float err;
    float integral;
    float derivative;
    float output;
} PIDPrevData;

typedef struct TorCtrlParam {
    PIDParam pid;

    float target_torque;
    float voltage_limit;
    float current_limit;
} TorCtrlParam;
extern TorCtrlParam g_tor_ctrl;

typedef struct VelCtrlParam {
    PIDParam pid;
    PIDPrevData prev_data;

    float voltage_limit;
    float current_limit;
    float voltage_ramp;
    float target_speed;
    float ctrl_rate;
} VelCtrlParam;
extern VelCtrlParam g_vel_ctrl;

typedef struct AngCtrlParam {
    PIDParam pid;

    float velocity_limit;
    float target_angle;
    float ctrl_rate;
} AngCtrlParam;
extern AngCtrlParam g_ang_ctrl;

typedef struct CurrCtrlParam {
    PIDParam pid;
    PIDPrevData prev_data;

    float voltage_limit;
    float voltage_ramp;
    float ctrl_rate;
} CurrCtrlParam;
extern CurrCtrlParam g_Iq_ctrl;
extern CurrCtrlParam g_Id_ctrl;

void pid_init(void);
float PID_torque(float err);
float PID_velocity(float err);
float PID_angle(float err);
float PID_current(CurrCtrlParam *pCtrl, float err);

void pid_clear_history(void);

#endif  //!__PID__H__