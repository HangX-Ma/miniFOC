/**
 * @file gui.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief OLED GUI
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

#ifndef __GUI__H__
#define __GUI__H__

#include "utils.h"
#include "base.h"
#include "vkey.h"
#include "easing.h"
#include "tween.h"
#include "menu_list.h"

typedef enum PageID {
    PAGE_ID_NONE = -1,

    PAGE_ID_FIRST = 0,

    PAGE_ID_MAIN_MENU_LIST = PAGE_ID_FIRST,
        PAGE_ID_MOTOR_APP_MENU_LIST,
    PAGE_ID_MOTOR_MODE_MENU_LIST,
    PAGE_ID_TOR_EDITOR_MENU_LIST,
    PAGE_ID_VEL_EDITOR_MENU_LIST,
    PAGE_ID_ANG_EDITOR_MENU_LIST,

    PAGE_ID_LOGO,
    PAGE_ID_EASING_CHART,
    PAGE_ID_ABOUT,

    PAGE_ID_LAST,

    FUNC_ID_RETURN = PAGE_ID_LAST + 1,
} PageID;

typedef struct SliderBase {
    char* description;
    float min;
    float max;
    float curr;
    float step;
} SliderBase;

void gui_init(void);
void gui_render(void);
void gui_switch(uint8_t id);

#endif  //!__GUI__H__