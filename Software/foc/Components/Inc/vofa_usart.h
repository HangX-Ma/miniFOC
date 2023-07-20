/**
 * @file vofa_usart.h
 * @author HangX-Ma (contour.9x@gmail.com)
 * @brief USART with DMA, communicating with VOFA plus
 * @version 0.1
 * @date 2023-07-18
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


#ifndef __VOFA_USART__H__
#define __VOFA_USART__H__

#include "utils.h"

/* definitions about USART1 and DMA1 settings */
#define USARTx_INSTANCE                 USART1
#define USARTx_TX_RX_GPIO_PORT          GPIOB
#define USARTx_TX_PIN                   LL_GPIO_PIN_6
#define USARTx_RX_PIN                   LL_GPIO_PIN_7
#define USARTx_DMAx_Tx_CHANNEL          LL_DMA_CHANNEL_4
#define USARTx_DMAx_Rx_CHANNEL          LL_DMA_CHANNEL_5

#define USARTx_DMA_TX_IRQHandler        DMA1_Channel4_IRQHandler
#define USARTx_DMA_RX_IRQHandler        DMA1_Channel5_IRQHandler


// define the float data number for send frame, but the last
// channel needs to store the frame tail
#define VOFA_SEND_CH_COUNT              4
#define VOFA_RECV_CH_COUNT              2
// send frame tail position
#define VOFA_SEND_FRAME_TAIL            (VOFA_SEND_CH_COUNT - 1)

/**
 * @brief Use union to decompose the float_data into 4 bytes char.
 *        Because the USART only transfer bytes.
 */
typedef union Format{
    float    fdata;    // little-endian float
    uint8_t  chars[4];
} Format_Typedef;

typedef struct SendFrame {
    Format_Typedef data_group[VOFA_SEND_CH_COUNT];
} SendFrame_t;

typedef struct RecvFrame {
    Format_Typedef data_group[VOFA_RECV_CH_COUNT];
} RecvFrame_t;

extern float g_target_motor_vel;

void vofa_usart_init();
void vofa_usart_dma_send_config(float *buffer, const uint32_t len);

#endif  //!__VOFA_USART__H__