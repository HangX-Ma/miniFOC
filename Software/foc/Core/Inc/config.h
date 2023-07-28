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


/**
 * @dir Hardware/Inc/encoder.h
 * @param g_encoder
 * @brief Use 'encoder_init' at initialization stage. Afterwards, you can use
 *        g_encoder to get motor angle and velocity.
 */

/**
 * @dir Components/Inc/vofa_usart.h
 * @param g_target_motor_vel
 * @brief Target motor rotation velocity, controlled by host using USART.
 */


/* ----------------  FOC Configurations ---------------- */
#define MOTOR_VM                        11.1f
#define MOTOR_POLE_PAIRS                7
#define SENSOR_ALIGN_VOLTAGE            0.8f
// PWM frequency 20 KHz, but we use center aligned mode, so 72MHz/1800 = 40 KHz
// can satisfy our need.
#define PWM_RELOAD_PERIOD               1800

#endif //!__CONFIG__H__