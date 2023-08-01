/**
 * @file utils.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief basic utilities
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

#ifndef __UTILS__H__
#define __UTILS__H__

#include "stm32f103xb.h"
#include <stdio.h>

typedef uint8_t BOOL;
typedef enum {
    FALSE = 0,
    TRUE = !FALSE,
} BOOL_INNER;

#define ERROR_CODE          0xDEADDEAD

#define _PI                 3.14159265359f
#define _PI_2               1.57079632679f
#define _PI_3               1.0471975512f
#define _2PI                6.28318530718f
#define _3PI_2              4.71238898038f
#define _PI_6               0.52359877559f
#define _SQRT3              1.73205080757f
#define _1_SQRT3            0.57735026919f
#define _2_SQRT3            1.15470053838f

// print with newline
#define println(fmt, ...) printf(fmt "\r\n", ##__VA_ARGS__)

#define abs(x) ((x) > 0 ? (x) : (-(x)))
#define constrain(val, low, high) ((val) < (low) ? (low) : ((val) > (high) ? (high) : (val)))

// number of elements in an array
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
// byte offset of member in structure
#define MEMBER_OFFSET(structure, member) ((int)&(((structure*)0)->member))
// size of a member of a structure
#define MEMBER_SIZE(structure, member) (sizeof(((structure*)0)->member))

#define INLINE __inline__ __attribute__((always_inline))


#endif  //!__UTILS__H__
