/**
 * @file config.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief MCU and other devices configurations. It is clear to manage those
 *        hardware definitions in one single file. However, I cannot manage
 *        the auto-generated STM32CubeMX code.
 * @version 0.1
 * @date 2023-07-17
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

#ifndef __CONFIG__H__
#define __CONFIG__H__


//* If you want to use the global symbols below,
//* you need to include the'*.h' file first!


// Contorl Period
//  =>  Current Sensor:   10 KHz
//  =>  Velocity Control: 1 KHz
//  =>  Position Control: 1 KHz

/**
 * @dir Hardware/Inc/encoder.h
 * @param g_encoder Encoder
 * @brief Use 'encoder_init' at initialization stage. Afterwards, you can use
 *        g_encoder to get motor angle and velocity.
 */

/**
 * @dir Hardware/Inc/bldc_config.h
 * @param g_bldc BLDC
 * @brief 'g_bldc' manages the PWMA, PWMB, PWMC output duty. Except for this,
 *        It can control all PWM channels output as well.
 */

/**
 * @dir Hardware/Inc/foc.h
 * @param g_foc
 * @brief FOC related functions and properties. We can align the motor and sensor
 *        using this 'g_foc'.
 */

/**
 * @dir Hardware/Inc/pid.h
 * @param g_tor_ctrl TorCtrlParam
 * @param g_vel_ctrl VelCtrlParam
 * @param g_ang_ctrl AngCtrlParam
 * @param g_Iq_ctrl CurrCtrlParam
 * @param g_Id_ctrl CurrCtrlParam
 * @brief You can set the PID parameters for all this control sets.
 */


/* ----------------  FOC Configurations ---------------- */
#define MOTOR_VM                        12.0f   // V
#define MOTOR_POLE_PAIRS                7
#define SENSOR_ALIGN_VOLTAGE            0.8f    // V

/* ----------------  FOC PID Configurations ---------------- */
#define FOC_VOLTAGE_LIMIT               2.0f    // V
#define FOC_VOLTAGE_RAMP                100.0f  // V/s
#define FOC_CURRENT_LIMIT               1.0f    // A
#define FOC_ANG_SPEED_LIMIT             30.0f   // set it low if you just start to test the motor
#define FOC_CONTROL_RATE                0.001f  // sec
#define FOC_KV                          900     // Motor property

// PWM frequency 20 KHz, but we use center aligned mode, so 72MHz/1800 = 40 KHz
// can satisfy our need.
#define PWM_RELOAD_PERIOD               1800

#endif //!__CONFIG__H__