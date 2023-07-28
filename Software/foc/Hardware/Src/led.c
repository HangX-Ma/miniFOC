#include "led.h"

void led_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct;

    LED_GPIO_CLK_ENABLE();

    GPIO_InitStruct.Pin = LL_GPIO_PIN_13;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;

    LL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    LED_STATE_OFF();
}

void led_blinking(uint32_t Period) {
    /* Toggle LED2 in an infinite loop */
    while (1) {
        LL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        LL_mDelay(Period);
    }
}