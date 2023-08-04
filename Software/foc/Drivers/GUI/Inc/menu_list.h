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
#include "tween.h"

typedef struct {
    uint8_t id;
    const char* title;
    void* arg;
} menu_item_t;

typedef struct {
    BOOL repaint_;
    callback_t painter;
    callback_t handler;

    uint16_t width_;
    uint16_t height_;

    menu_item_t* items_;
    uint8_t      item_num_;

    // configurations
    uint8_t display_num_;       // number of items that can be displayed per page
    float   height_line_;       // each line height
    float   height_slider_;     // each scroll height

    // index
    uint8_t masked_index_;      // [0, min(item_num_, display_count)), index of the masked item in page(光标)
    uint8_t selected_index_;    // [0, item_num_), current selected item indexes

    // pixel position and easing
    uint8_t x_padding_;
    float   y_padding_;
    Tween   width_mask_;
    Tween   y_mask_;
    uint8_t x_slider_;
    Tween   y_slider_;
    Tween   y_title_offset_;
} MenuList;

MenuList menu_list_init(
    const menu_item_t items[],
    const uint8_t     item_num,
    const uint8_t     display_count, // default = 1
    const callback_t  painter,
    const callback_t  handler
);

void menu_list_callback_painter_update_easing(MenuList* pMenuList);
void menu_list_callback_painter_draw_items(MenuList* pMenuList);
void menu_list_callback_painter_draw_scroll(MenuList* pMenuList);
void menu_list_callback_painter_draw_item_mask(MenuList* pMenuList);
void menu_list_callback_painter_by_default(MenuList* pMenuList);

void menu_list_callback_handler_switch_to_prev(MenuList* pMenuList);
void menu_list_callback_handler_switch_to_next(MenuList* pMenuList);
void menu_list_callback_handler_by_default(MenuList* pMenuList);

#endif  //!__MENU_LIST__H__