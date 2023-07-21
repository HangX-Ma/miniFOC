#include "oled.h"
#include "u8g2.h"
#include "led.h"

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_utils.h"


u8g2_t u8g2;

static void oled_gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

    /* OLED RST and DC pins */
    LL_GPIO_ResetOutputPin(OLED_RST_DC_GPIO_PORT, OLED_RST_PIN | OLED_DC_PIN);
    GPIO_InitStruct.Pin = OLED_RST_PIN | OLED_DC_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(OLED_RST_DC_GPIO_PORT, &GPIO_InitStruct);

    /* OELD CS pin */
    LL_GPIO_SetOutputPin(OLED_CS_SCK_MOSI_GPIO_PORT, OLED_CS_PIN);
    GPIO_InitStruct.Pin = OLED_CS_PIN;
    LL_GPIO_Init(OLED_CS_SCK_MOSI_GPIO_PORT, &GPIO_InitStruct);

}

static void oled_spi1_init() {
    LL_SPI_InitTypeDef SPI_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

    /** SPI1 GPIO Configuration
        => PA5   ------> SPI1_SCK
        => PA7   ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin        = OLED_SCK_PIN | OLED_MOSI_PIN;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(OLED_CS_SCK_MOSI_GPIO_PORT, &GPIO_InitStruct);

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

static void oled_spi_dma_tx_init() {
    LL_DMA_InitTypeDef DMA_InitStruct;

    /* DMA controller clock enable */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    /* DMA interrupt init */
    NVIC_SetPriority(DMA1_Channel3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);

    LL_DMA_DeInit(DMA1, OLED_SPI_DMAx_Tx_CHANNEL);
    LL_DMA_DisableChannel(DMA1, OLED_SPI_DMAx_Tx_CHANNEL);
    DMA_InitStruct.PeriphOrM2MSrcAddress  = LL_SPI_DMA_GetRegAddr(SPI1);
    DMA_InitStruct.MemoryOrM2MDstAddress  = 0; // use oled_dma_tx_transfer to set this
    DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
    DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
    DMA_InitStruct.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
    DMA_InitStruct.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
    DMA_InitStruct.Direction              = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    DMA_InitStruct.Priority               = LL_DMA_PRIORITY_LOW;
    DMA_InitStruct.Mode                   = LL_DMA_MODE_NORMAL;
    DMA_InitStruct.NbData                 = 0; // use oled_dma_tx_transfer to set this
    LL_DMA_Init(DMA1, OLED_SPI_DMAx_Tx_CHANNEL, &DMA_InitStruct);
    // Don't enable DMA Tx channel and buffer here
}

void oled_dma_tx_transfer(uint8_t *mem_addr, uint32_t mem_size) {
    LL_SPI_DisableDMAReq_TX(SPI1);
    LL_DMA_DisableChannel(DMA1, OLED_SPI_DMAx_Tx_CHANNEL);

    LL_DMA_SetMemoryAddress(DMA1, OLED_SPI_DMAx_Tx_CHANNEL, (uint32_t)mem_addr);
    LL_DMA_SetDataLength(DMA1, OLED_SPI_DMAx_Tx_CHANNEL, mem_size);

    LL_DMA_EnableChannel(DMA1, OLED_SPI_DMAx_Tx_CHANNEL);
    LL_SPI_EnableDMAReq_TX(SPI1);
}

/*官方提供的Logo绘制demo*/
void draw(u8g2_t *u8g2)
{
    u8g2_SetFontMode(u8g2, 1); /*字体模式选择*/
    u8g2_SetFontDirection(u8g2, 0); /*字体方向选择*/
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf); /*字库选择*/
    u8g2_DrawStr(u8g2, 0, 20, "U");

    u8g2_SetFontDirection(u8g2, 1);
    u8g2_SetFont(u8g2, u8g2_font_inb30_mn);
    u8g2_DrawStr(u8g2, 21, 8, "8");

    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf);
    u8g2_DrawStr(u8g2, 51, 30, "g");
    u8g2_DrawStr(u8g2, 67, 30, "\xb2");

    u8g2_DrawHLine(u8g2, 2, 35, 47);
    u8g2_DrawHLine(u8g2, 3, 36, 47);
    u8g2_DrawVLine(u8g2, 45, 32, 12);
    u8g2_DrawVLine(u8g2, 46, 33, 12);

    u8g2_SetFont(u8g2, u8g2_font_4x6_tr);
    u8g2_DrawStr(u8g2, 1, 54, "github.com/olikraus/u8g2");
}


uint8_t u8x8_byte_4wire_hw_spi(
    U8X8_UNUSED u8x8_t *u8x8,
    uint8_t msg,
    uint8_t arg_int,
    void *arg_ptr)
{
    switch (msg)
    {
        case U8X8_MSG_BYTE_SEND: // Use SPI to send 'arg_int' bytes
            oled_dma_tx_transfer((uint8_t *)arg_ptr, (uint32_t)arg_int);
            // while(LL_DMA_GetDataLength(DMA1, OLED_SPI_DMAx_Tx_CHANNEL)) {}
            break;
        case U8X8_MSG_BYTE_INIT: // init SPI and DMA
            break;
        case U8X8_MSG_BYTE_SET_DC: // Setup DC pin, which determines CMD or Data transferred through SPI
            if (arg_int) {
                LL_GPIO_SetOutputPin(OLED_RST_DC_GPIO_PORT, OLED_DC_PIN); // Data
            } else {
                LL_GPIO_ResetOutputPin(OLED_RST_DC_GPIO_PORT, OLED_DC_PIN); // CMD
            }
            break;
        case U8X8_MSG_BYTE_START_TRANSFER: // Software CS is needed. (select)
            LL_GPIO_ResetOutputPin(OLED_CS_SCK_MOSI_GPIO_PORT, OLED_CS_PIN);
            u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:  // Software CS is needed. (deselect)
            u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
            LL_GPIO_SetOutputPin(OLED_CS_SCK_MOSI_GPIO_PORT, OLED_CS_PIN);
            break;
        default:
            return 0;
    }
    return 1;
}

uint8_t u8x8_stm32_gpio_and_delay(
    U8X8_UNUSED u8x8_t *u8x8,
    U8X8_UNUSED uint8_t msg,
    uint8_t arg_int,
    U8X8_UNUSED void *arg_ptr)
{
    switch (msg)
    {
        case U8X8_MSG_DELAY_MILLI: // delay function
            LL_mDelay(arg_int);
            break;
        case U8X8_MSG_GPIO_RESET: // gpio reset
            break;
    }
    return 1;
}

void u8g2_init(u8g2_t *u8g2) {
    u8g2_Setup_ssd1306_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_4wire_hw_spi, u8x8_stm32_gpio_and_delay);
    u8g2_InitDisplay(u8g2); // init ssd1306, the screen is off
    u8g2_SetPowerSave(u8g2, 0); // turn on the screen
    u8g2_ClearBuffer(u8g2);

    LL_mDelay(1000);

    u8g2_DrawLine(u8g2, 0, 0, 127, 63);
    u8g2_DrawLine(u8g2, 127, 0, 0, 63);
    u8g2_SendBuffer(u8g2);
    LL_mDelay(1000);
}

void oled_init() {
    oled_gpio_init();
    oled_spi_dma_tx_init();
    oled_spi1_init();

    u8g2_init(&u8g2);

    do
    {
        LED_STATE_TOGGLE();
        draw(&u8g2);
        LL_mDelay(100);
    }
    while (1);
}

