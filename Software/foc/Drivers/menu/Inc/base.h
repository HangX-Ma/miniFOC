/**
 * @file base.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief u8g2 menu base function set
 * @version 0.1
 * @date 2022-08-05
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

#include "utils.h"
#include "oled.h"

typedef struct MenuBase {
    void (*get_str_width)(const char *);
    void (*draw_str)(u8g2_uint_t, u8g2_uint_t, const char *);
    void (*draw_num)(u8g2_uint_t, u8g2_uint_t, float);
    void (*draw_pixel)(u8g2_uint_t, u8g2_uint_t);
    void (*draw_hline)(u8g2_uint_t, u8g2_uint_t, u8g2_uint_t);
    void (*draw_vline)(u8g2_uint_t, u8g2_uint_t, u8g2_uint_t);
    void (*draw_line)(u8g2_uint_t, u8g2_uint_t, u8g2_uint_t, u8g2_uint_t);
    void (*draw_rect)(u8g2_uint_t, u8g2_uint_t, u8g2_uint_t, u8g2_uint_t);
    void (*draw_round_rect)(u8g2_uint_t, u8g2_uint_t, u8g2_uint_t, u8g2_uint_t, u8g2_uint_t);
    void (*draw_fill_rect)(u8g2_uint_t, u8g2_uint_t, u8g2_uint_t, u8g2_uint_t);
    void (*draw_fill_round_rect)(u8g2_uint_t, u8g2_uint_t, u8g2_uint_t, u8g2_uint_t, u8g2_uint_t);
    void (*draw_circle)(u8g2_uint_t, u8g2_uint_t, u8g2_uint_t, uint8_t);
    void (*draw_circle_full)(u8g2_uint_t, u8g2_uint_t, u8g2_uint_t);
    void (*draw_circle_full_center)(u8g2_uint_t, u8g2_uint_t, u8g2_uint_t);
    void (*draw_rotated_rect)(float, float, float, float, float);
    void (*set_color)(uint8_t c);
    void (*clear)(void);
    void (*update)(void);
} MenuBase;
extern MenuBase g_menu_base;