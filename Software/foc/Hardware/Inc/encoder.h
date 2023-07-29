/**
 * @file encoder.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief SC60228DC magnetic encoder
 * @version 0.1
 * @date 2023-07-17
 * @ref https://github.com/simplefoc/Arduino-FOC-drivers/tree/master/src/encoders/sc60228
 * @ref https://blog.csdn.net/weixin_43593122/article/details/119253544
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

#ifndef __ENCODER__H__
#define __ENCODER__H__

#include "utils.h"

#define ENCODER_GPIO_PORT               GPIOB
#define ENCODER_CS_PIN                  LL_GPIO_PIN_12
#define ENCODER_SPI_SCK_PIN             LL_GPIO_PIN_13
#define ENCODER_SPI_MISO_PIN            LL_GPIO_PIN_14
#define ENCODER_SPI_MOSI_PIN            LL_GPIO_PIN_15
#define ENCODER_RESOLUTION              4096
#define ENCODER_DATA_LENGTH             12

typedef union {
    struct {
        uint16_t parity:1;
        uint16_t df2:1;
        uint16_t df1:1;
        uint16_t err:1;
        uint16_t angle:12;
    };
    uint16_t reg;
} SC60228Angle;


typedef enum {
    CW      = 1,  // clockwise
    CCW     = -1, // counter-clockwise
    UNKNOWN = 0   // not yet known or invalid state
} Direction;

typedef struct Encoder {
    Direction dir_;

    float (* get_angle)(void);
    float (* get_shaft_angle)(void);        ///< read the cumulative magnetic absolute angle value
    float (* get_shaft_velocity)(void);     ///< get current motor shaft velocity
    BOOL (* is_error)(void);                ///< check if the senor is too far away with the magnetic
} Encoder;
extern Encoder g_encoder;


void encoder_init(void);
void encoder_reset(void);
// You need to ensure the 'encoder_init' has been called.
void encoder_test(void);

#endif  //!__ENCODER__H__