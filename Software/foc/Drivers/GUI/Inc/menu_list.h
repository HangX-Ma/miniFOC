/**
 * @file menu_list.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief u8g2 menu list components
 * @version 0.1
 * @date 2023-07-027
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

#ifndef __MENU_LIST__H__
#define __MENU_LIST__H__

#include "utils.h"
#include "base.h"

typedef struct {
    uint8_t id;
    const char* title;
    void* arg;
} menu_item_t;

typedef struct {

} MenuList;

#endif  //!__MENU_LIST__H__