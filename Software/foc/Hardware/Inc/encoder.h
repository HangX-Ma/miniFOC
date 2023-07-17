/**
 * @file encoder.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief SC60228DC Magnetic encoder
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

#ifndef __GMR__H__
#define __GMR__H__

#include "config.h"

#define ENCODER_RESOLUTION          4096
#define ENCODER_DATA_LENGTH         12

typedef struct {
    void (*select_chip)();
} Encoder_t;

void encoder_init();

#endif  //!__GMR__H__