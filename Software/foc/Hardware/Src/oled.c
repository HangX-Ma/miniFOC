#include "oled.h"

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_dma.h"

static void oled_gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);

    /* OLED RST and DC pins */
    LL_GPIO_ResetOutputPin(OLED_GPIO_PORT, OLED_RST_PIN | OLED_DC_PIN);
    GPIO_InitStruct.Pin = OLED_RST_PIN | OLED_DC_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(OLED_GPIO_PORT, &GPIO_InitStruct);
}

static void oled_spi1_init() {
    LL_SPI_InitTypeDef SPI_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

    /** SPI1 GPIO Configuration
        => PA4   ------> SPI1_NSS
        => PA5   ------> SPI1_SCK
        => PA7   ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin        = OLED_CS_PIN | OLED_SCK_PIN | OLED_MOSI_PIN;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
    SPI_InitStruct.Mode              = LL_SPI_MODE_MASTER;
    SPI_InitStruct.DataWidth         = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.ClockPolarity     = LL_SPI_POLARITY_HIGH;
    SPI_InitStruct.ClockPhase        = LL_SPI_PHASE_2EDGE;
    SPI_InitStruct.NSS               = LL_SPI_NSS_HARD_OUTPUT;
    SPI_InitStruct.BaudRate          = LL_SPI_BAUDRATEPRESCALER_DIV4;
    SPI_InitStruct.BitOrder          = LL_SPI_MSB_FIRST;
    SPI_InitStruct.CRCCalculation    = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.CRCPoly           = 10;
    LL_SPI_Init(SPI1, &SPI_InitStruct);

    // Don't remember to enable SPI1
    LL_SPI_Enable(SPI1);
}

static void oled_spi_dma_tx_init(uint8_t *mem_addr, uint32_t mem_size) {
    LL_DMA_InitTypeDef DMA_InitStruct;

    /* DMA controller clock enable */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    /* DMA interrupt init */
    NVIC_SetPriority(DMA1_Channel3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);

    LL_DMA_DeInit(DMA1, OLED_SPI_DMAx_Tx_CHANNEL);
    LL_DMA_DisableChannel(DMA1, OLED_SPI_DMAx_Tx_CHANNEL);
    DMA_InitStruct.PeriphOrM2MSrcAddress  = LL_SPI_DMA_GetRegAddr(SPI1);
    DMA_InitStruct.MemoryOrM2MDstAddress  = (uint32_t)mem_addr;
    DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
    DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
    DMA_InitStruct.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
    DMA_InitStruct.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
    DMA_InitStruct.Direction              = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    DMA_InitStruct.Priority               = LL_DMA_PRIORITY_MEDIUM;
    DMA_InitStruct.Mode                   = LL_DMA_MODE_NORMAL;
    DMA_InitStruct.NbData                 = mem_size;
    LL_DMA_Init(DMA1, OLED_SPI_DMAx_Tx_CHANNEL, &DMA_InitStruct);

    /* open transfer complete interrupt */
    LL_DMA_EnableIT_TC(DMA1, OLED_SPI_DMAx_Tx_CHANNEL);
    LL_DMA_ClearFlag_TC3(DMA1);

    /* Enable Tx buffer DMA */
    LL_SPI_EnableDMAReq_TX(SPI1);
    /* Enable DMA Channels Tx */
    LL_DMA_EnableChannel(DMA1, OLED_SPI_DMAx_Tx_CHANNEL);
}

void OLED_DMAx_Tx_IRQHandler(void) {
    if (LL_DMA_IsActiveFlag_TC3(DMA1) != RESET) {
        LL_DMA_ClearFlag_TC3(DMA1);
        LL_DMA_DisableChannel(DMA1, OLED_SPI_DMAx_Tx_CHANNEL);
    }
}

void oled_dma_tx_reload() {

}

void oled_init() {
    oled_gpio_init();
    // TODO: Not complete, temp place holder
    oled_spi_dma_tx_init(0, 0);
    oled_spi1_init();
}