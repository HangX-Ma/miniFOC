/**
 * @file vkey.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief virtual keys
 * @version 0.1
 * @date 2023-08-02
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

#ifndef __VKEY__H__
#define __VKEY__H__

#include "utils.h"

typedef enum {
    VKEY_ID_NONE  = 0,
    VKEY_ID_START = 1,
    VKEY_ID_PREV  = 1,
    VKEY_ID_OK    = 2,
    VKEY_ID_NEXT  = 3,
    VKEY_ID_SIZE  = 4,
} VKeyID;
extern BOOL g_vkeys[VKEY_ID_SIZE];

VKeyID vkey_scan(void);
void vkey_init(void);

#endif  //!__VKEY__H__