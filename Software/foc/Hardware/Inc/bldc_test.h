/**
 * @file bldc_config.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief BLDC test functions, used to confirm BLDC performing conditions
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

#ifndef __BLDC_TEST__H__
#define __BLDC_TEST__H__

/* ----------------- TEST CODE BELOW ----------------- */

/**
 * @brief simplest rotation test
 * @details We give three phrases with sequential PWM output in a specific duty,
 *          so the BLDC motor can rotate without any control algorithm.
 * @warning Don't change the duty over 100 or too high. It's really dangerous.
 */
void bldc_test1_invariant_duty(void);

#endif //!__BLDC_TEST__H__