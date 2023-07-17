/**
 * @file config.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief MCU and other devices configurations
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

//? SC60228DC Magnetic encoder
#define ENCODER_GPIO_PORT       GPIOA
#define ENCODER_CS              LL_GPIO_PIN_4

#endif  //!__CONFIG__H__