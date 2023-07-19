#include "vofa_usart.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_usart.h"
#include "utils.h"
#include "led.h"

SendFrame_t send_frame;
RecvFrame_t recv_frame;
float g_target_motor_vel = 0;

static BOOL set_vofa_frame_tail(uint32_t tail) {
    if (tail > VOFA_SEND_FRAME_TAIL) {
        return FALSE;
    }
    // add tail frame
    send_frame.data_group[tail].chars[0] = 0x00;
    send_frame.data_group[tail].chars[1] = 0x00;
    send_frame.data_group[tail].chars[2] = 0x80;
    send_frame.data_group[tail].chars[3] = 0x7F;
    return TRUE;
}

void usart_tx_dma_config(void) {
    /* Clear previous value to avoid disturbance */
    LL_DMA_DeInit(DMA1, USARTx_DMAx_Tx_CHANNEL);
    LL_DMA_DisableChannel(DMA1, USARTx_DMAx_Tx_CHANNEL);

    LL_DMA_InitTypeDef USARTx_DMA_InitStruct_Tx = {0};

    /* USART1_TX DMA Init */
    USARTx_DMA_InitStruct_Tx.PeriphOrM2MSrcAddress  = LL_USART_DMA_GetRegAddr(USARTx_INSTANCE);
    USARTx_DMA_InitStruct_Tx.MemoryOrM2MDstAddress  = (uint32_t)send_frame.data_group;
    USARTx_DMA_InitStruct_Tx.Direction              = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    USARTx_DMA_InitStruct_Tx.Mode                   = LL_DMA_MODE_NORMAL;
    USARTx_DMA_InitStruct_Tx.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
    USARTx_DMA_InitStruct_Tx.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
    USARTx_DMA_InitStruct_Tx.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
    USARTx_DMA_InitStruct_Tx.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
    USARTx_DMA_InitStruct_Tx.NbData                 = 0;
    USARTx_DMA_InitStruct_Tx.Priority               = LL_DMA_PRIORITY_MEDIUM;
    LL_DMA_Init(DMA1, USARTx_DMAx_Tx_CHANNEL, &USARTx_DMA_InitStruct_Tx);
    /* Enable DMA transfer complete interrupts Tx */
    LL_DMA_EnableIT_TC(DMA1, USARTx_DMAx_Tx_CHANNEL);
    LL_DMA_ClearFlag_TC4(DMA1);
    /* Enable DMA Channel Tx */
    LL_DMA_EnableChannel(DMA1, USARTx_DMAx_Tx_CHANNEL);
}

void usart_rx_dma_config(void) {
    /* Clear previous value to avoid disturbance */
    LL_DMA_DeInit(DMA1, USARTx_DMAx_Rx_CHANNEL);
    LL_DMA_DisableChannel(DMA1, USARTx_DMAx_Rx_CHANNEL);
    LL_DMA_InitTypeDef USARTx_DMA_InitStruct_Rx = {0};

    /* USART1_RX DMA Init */
    USARTx_DMA_InitStruct_Rx.PeriphOrM2MSrcAddress  = LL_USART_DMA_GetRegAddr(USARTx_INSTANCE);
    USARTx_DMA_InitStruct_Rx.MemoryOrM2MDstAddress  = (uint32_t)recv_frame.data_group;
    USARTx_DMA_InitStruct_Rx.Direction              = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    USARTx_DMA_InitStruct_Rx.Mode                   = LL_DMA_MODE_NORMAL;
    USARTx_DMA_InitStruct_Rx.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
    USARTx_DMA_InitStruct_Rx.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
    USARTx_DMA_InitStruct_Rx.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
    USARTx_DMA_InitStruct_Rx.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
    USARTx_DMA_InitStruct_Rx.NbData                 = 8;
    USARTx_DMA_InitStruct_Rx.Priority               = LL_DMA_PRIORITY_MEDIUM;
    LL_DMA_Init(DMA1, USARTx_DMAx_Rx_CHANNEL, &USARTx_DMA_InitStruct_Rx);
    /* Enable DMA transfer complete interrupts Rx */
    LL_DMA_EnableIT_TC(DMA1, USARTx_DMAx_Rx_CHANNEL);
    LL_DMA_ClearFlag_TC5(DMA1);

    /* Enable DMA Channel Rx */
    LL_DMA_EnableChannel(DMA1, USARTx_DMAx_Rx_CHANNEL);
}

void usart_rx_dma_reload(void) {
    LL_DMA_DisableChannel(DMA1, USARTx_DMAx_Rx_CHANNEL);
    LL_DMA_SetMemoryAddress(DMA1, USARTx_DMAx_Rx_CHANNEL, (uint32_t)recv_frame.data_group);
    LL_DMA_SetDataLength(DMA1, USARTx_DMAx_Rx_CHANNEL, 8);
    LL_DMA_EnableChannel(DMA1, USARTx_DMAx_Rx_CHANNEL);
}

void usart_tx_dma_reload(uint32_t len) {
    LL_DMA_DisableChannel(DMA1, USARTx_DMAx_Tx_CHANNEL);
    LL_DMA_SetMemoryAddress(DMA1, USARTx_DMAx_Tx_CHANNEL, (uint32_t)send_frame.data_group);
    LL_DMA_SetDataLength(DMA1, USARTx_DMAx_Tx_CHANNEL, len);
    LL_DMA_EnableChannel(DMA1, USARTx_DMAx_Tx_CHANNEL);
}

void vofa_usart_init(void) {
    /* --------------- USART1 init ----------------- */
    LL_USART_InitTypeDef USART_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);

    /**USART1 GPIO Configuration
    PB6   ------> USART1_TX
    PB7   ------> USART1_R
    */
    GPIO_InitStruct.Pin        = USARTx_TX_PIN;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_UP;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(USARTx_TX_RX_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin        = USARTx_RX_PIN;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_FLOATING;
    LL_GPIO_Init(USARTx_TX_RX_GPIO_PORT, &GPIO_InitStruct);

    LL_GPIO_AF_EnableRemap_USART1();

    /* Configure NVIC for USARTx transfer interrupts */
    NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(USART1_IRQn);

    USART_InitStruct.BaudRate            = 115200;
    USART_InitStruct.DataWidth           = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits            = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity              = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection   = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling        = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USARTx_INSTANCE, &USART_InitStruct);
    LL_USART_ConfigAsyncMode(USARTx_INSTANCE);
    LL_USART_Enable(USARTx_INSTANCE);

    /* Enable DMA RX Interrupt */
    LL_USART_EnableDMAReq_RX(USARTx_INSTANCE);
    /* Enable DMA TX Interrupt */
    LL_USART_EnableDMAReq_TX(USARTx_INSTANCE);

    /* Configure NVIC for USARTx transfer interrupts */
    NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 1));
    NVIC_EnableIRQ(USART1_IRQn);

    /* DMA interrupt init */
    NVIC_SetPriority(DMA1_Channel4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);
    NVIC_SetPriority(DMA1_Channel5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(DMA1_Channel5_IRQn);

    /* ------------------ USART1 DMA Init ------------------ */
    /* DMA controller clock enable */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    usart_rx_dma_config();
    usart_tx_dma_config();
}

/* Handle the DMA interrupt */

void USARTx_DMA_TX_IRQHandler(void) {
    // If data sending complete, clear the TC4 flag
    if (LL_DMA_IsActiveFlag_TC4(DMA1) != RESET) {
        LL_DMA_ClearFlag_TC4(DMA1);
    }
}

void USARTx_DMA_RX_IRQHandler(void) {
    // If data receiving complete, clear the TC5 flag
    if (LL_DMA_IsActiveFlag_TC5(DMA1) != RESET) {
        LL_DMA_ClearFlag_TC5(DMA1);
        // receive frame header: 0xAA 0xFF
        Format_Typedef recv_header = recv_frame.data_group[0];
        if ((recv_header.chars[0] == 0xAA) && (recv_header.chars[1] == 0xFF)) {
            char cmd = recv_header.chars[3];
            Format_Typedef recv_data = recv_frame.data_group[1];

            switch (cmd) {
            case 0x01:                                // control motor start/stop
                if (recv_data.chars[0] == 1) {        // start
                    LED_STATE_TOGGLE();
                } else if (recv_data.chars[0] == 0) { // stop
                    ;
                }
                break;
            case 0x02: // set velocity
                g_target_motor_vel = recv_data.fdata;
                break;
            case 0x03: // set Speed Kp
                break;
            case 0x04: // set Speed Ki
                break;
            case 0x05: // set Current Kp
                break;
            case 0x06: // set Current Ki
                break;
            case 0x07: // set Current Kp
                break;
            case 0x08: // set Current Ki
                break;
            default:
                break;
            }
        }
        // we need to disable dma rx channel and reset the transfer length
        usart_rx_dma_reload();
    }
}

void vofa_usart_dma_send_config(float *buffer, const uint32_t len) {
    // no data or data length overflows
    if (len == 0 || len > VOFA_SEND_FRAME_TAIL /* reserve frame tail*/) {
        return;
    }

    const uint32_t frame_tail_index = len;
    set_vofa_frame_tail(frame_tail_index);

    for (uint32_t i = 0; i < len; i++) {
        send_frame.data_group[i].fdata = buffer[i];
    }
    usart_tx_dma_reload((len + 1) * 4);
}