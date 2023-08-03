/**
 * @file tween.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief tween
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

#ifndef __TWEEN__H__
#define __TWEEN__H__

#include "easing.h"

typedef enum {
    TWEEN_TRANS_LINEAR,
    TWEEN_TRANS_QUAD,
    TWEEN_TRANS_CUBIC,
    TWEEN_TRANS_QUART,
    TWEEN_TRANS_QUINT,
    TWEEN_TRANS_SINE,
    TWEEN_TRANS_EXPO,
    TWEEN_TRANS_CIRC,
    TWEEN_TRANS_ELASTIC,
    TWEEN_TRANS_BACK,
    TWEEN_TRANS_BOUNCE
} TweenTransitionType;

typedef enum {
    TWEEN_EASE_IN,
    TWEEN_EASE_OUT,
    TWEEN_EASE_IN_OUT,
    TWEEN_EASE_OUT_IN
} TweenEaseType;

typedef enum TweenMode {
    TWEEN_MODE_BIT_CNT   = 4,
    TWEEN_MODE_MASK      = (1 << TWEEN_MODE_BIT_CNT) - 1,
    // tween running times
    TWEEN_TIMES_SINGLE   = 0 << 0,  // single(default)
    TWEEN_TIMES_REPEAT   = 1 << 0,  // repeat n times
    TWEEN_TIMES_INFINITE = 1 << 1,  // loop
    // tween running direction
    TWEEN_DIR_FORWARD    = 0 << 0,   // forward (default)
    TWEEN_DIR_BACKWARD   = 1 << 2,   // backward
    TWEEN_DIR_YOYO       = 1 << 3,   // back and forth
} TweenMode;

typedef struct Tween {
    TweenMode mode_;

    // pixel postion
    float start_;           //< The start pixel position
    float end_;             //< The end pixel position
    float delta_;           //< The variation from start_ to end_
    float current_;         //< The pixel position
    float offset_;          //< The pixel position offset, normally 0.

    // Notice that the frames include 'start frame' and 'stop frame'
    uint16_t frame_num_;    //< The total number of the frames, whose minimum value is 2(start, stop).
    uint16_t frame_idx_;    //< Current frame index. Range: [0, frame_num]

    float (*easing)(float); //< easing functions
    float step_;            //< This is the normalized input value for easing functions. Range: [0, 1]

    int16_t repeat_times_;  //< animation repeated times
    uint16_t duration_;     //< each frame minimum duration (ms)
    BOOL dir_;              //< animation direction, 1: backward, 0: forward
} Tween;

typedef struct TweenHandler {
    Tween (*create)(TweenMode, easing_t /* callback */, float /* offset */, uint16_t /* frame num */, uint16_t /* duration */);
    void (*start_absolute)(Tween*, float /* pixel from */, float /* pixel to */);
    void (*start_relative)(Tween*, float /* pixel distance */);
    void (*stop)(Tween*, float /* current pixel pos */);
    void (*update)(Tween*);
    BOOL (*finished)(Tween*);
    float (*curr_pixel_pos)(Tween*);
} TweenHandler;

#define TWEEN_MODE_DEFAULT                  (TWEEN_TIMES_SINGLE | TWEEN_DIR_FORWARD)
#define TWEEN_MODE_REPEAT_NTIMES(n)         (TWEEN_TIMES_REPEAT | (n << TWEEN_MODE_BIT_CNT))

// easing functions array
// extern const easing_t g_easing_array[11][4]; // [TweenTransitionType][TweenEaseType]
extern TweenHandler g_tween_handler;

// Tween tween_create(
//     TweenMode mode,            // default: TWEEN_TIMES_SINGLE | TWEEN_DIR_FORWARD
//     easing_t  easing_callback, // default: 0 (easing_Linear)
//     float     offset,          // default: 0 (pixel offset)
//     uint16_t  frame_num,       // default: 2 (start/stop fram)
//     uint16_t  duration         // default: 0 (None)
// );
// void tween_start_absolute(Tween* pTween, float from, float to);
// void tween_start_relative(Tween* pTween, float dist);
// void tween_stop(Tween* pTween, float current);
// void tween_update(Tween* pTween);
// BOOL tween_finished(Tween* pTween);
// float tween_pixel_position(Tween* pTween);

void tween_handler_init(void);

#endif  //!__TWEEN__H__