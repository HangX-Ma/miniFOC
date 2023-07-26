/**
 * @file oled.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief oled fundamental functions
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

#ifndef __OLED__H__
#define __OLED__H__

#include "utils.h"
#include "u8g2.h"

#define OLED_SCK_MOSI_GPIO_PORT  GPIOA
#define OLED_SCK_PIN                LL_GPIO_PIN_5
#define OLED_MOSI_PIN               LL_GPIO_PIN_7


#define OLED_RST_DC_CS_GPIO_PORT    GPIOB
#define OLED_RST_PIN                LL_GPIO_PIN_0
#define OLED_DC_PIN                 LL_GPIO_PIN_1
#define OLED_CS_PIN                 LL_GPIO_PIN_10

#define OLED_DMAx_Tx_IRQHandler     DMA1_Channel3_IRQHandler
#define OLED_SPI_DMAx_Tx_CHANNEL    LL_DMA_CHANNEL_3

#define OLED_WIDTH                  128
#define OLED_HEIGHT                 64

extern u8g2_t u8g2;
extern uint8_t* g_u8g2_buf_ptr;
extern uint16_t g_u8g2_buf_len;

void oled_init(void);
void oled_test(void);

#endif  //!__OLED__H__