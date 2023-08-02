/**
 * @file easing.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief easing function
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

#ifndef __EASING__H__
#define __EASING__H__

#include "utils.h"

/**
 * @details 缓动函数介绍：
 *  easeIn： 速度从0开始加速的缓动
 *  easeOut： 减速到0的缓动
 *  easeInOut： 速度的前半段从0开始加速，后半段减速到0的缓动。
 * @ref https://github.com/rynmth/ctween
 */

// Linear 匀速
float easing_Linear(float step);

// Quadratic 二次方的缓动 （t^2）
float easing_In_Quad(float step);
float easing_Out_Quad(float step);
float easing_InOut_Quad(float step);
float easing_OutIn_Quad(float step);

// Cubic三次方的缓动 （t^3）
float easing_In_Cubic(float step);
float easing_Out_Cubic(float step);
float easing_InOut_Cubic(float step);
float easing_OutIn_Cubic(float step);

// Quartic四次方的缓动 （t^4）
float easing_In_Quart(float step);
float easing_Out_Quart(float step);
float easing_InOut_Quart(float step);
float easing_OutIn_Quart(float step);

// Quintic五次方的缓动 (t^5)
float easing_In_Quint(float step);
float easing_Out_Quint(float step);
float easing_InOut_Quint(float step);
float easing_OutIn_Quint(float step);

// Sinusoidal正弦曲线的缓动（sin(t)）
float easing_In_Sine(float step);
float easing_Out_Sine(float step);
float easing_InOut_Sine(float step);
float easing_OutIn_Sine(float step);

// Exponential指数曲线的缓动（2^t）
float easing_In_Expo(float step);
float easing_Out_Expo(float step);
float easing_InOut_Expo(float step);
float easing_Out_InExpo(float step);

// Circular圆形曲线的缓动sqrt(1-t^2)
float easing_In_Circ(float step);
float easing_Out_Circ(float step);
float easing_InOut_Circ(float step);
float easing_OutIn_Circ(float step);

// Elastic指数衰减的正弦曲线缓动
float easing_In_Elastic(float step);
float easing_Out_Elastic(float step);
float easing_InOut_Elastic(float step);
float easing_OutIn_Elastic(float step);

// Back超过范围的三次方缓动（(s+1)*t^3 - s*t^2）
float easing_In_Back(float step);
float easing_Out_Back(float step);
float easing_InOut_Back(float step);
float easing_OutIn_Back(float step);

// Bounce指数衰减的反弹缓动
float easing_Out_Bounce(float step);
float easing_In_Bounce(float step);
float easing_InOut_Bounce(float step);
float easing_OutIn_Bounce(float step);

typedef float (*easing_t)(float);

#endif  //!__EASING__H__