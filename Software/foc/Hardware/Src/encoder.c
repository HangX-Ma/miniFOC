#include "encoder.h"
#include "delay.h"
#include "qfplib-m3.h"

#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_bus.h"


Encoder g_encoder;
Direction g_sensor_dir;

typedef struct {
    BOOL error_check;   // indicate the SC60228 state
    BOOL parity_check;  // if the received data has error or not
} data_check_t;

static data_check_t data_check = {
    .error_check = FALSE,
    .parity_check = FALSE,
};


static uint32_t vel_sample_timestamp = 0;
static float raw_angle_data_prev = 0; // last raw angle value, MAX = ENCODER_RESOLUTION
static float angle_prev = 0; // last angle value(radian)
static float rotation_turns_angles = 0; // how may turns the motor runs(radian)


static uint16_t spi2_transmit_rw(uint16_t outdata) {
    // wait until the SPI Tx buffer to be empty
    while (LL_SPI_IsActiveFlag_TXE(SPI2) == RESET) {}
    LL_SPI_TransmitData16(SPI2, outdata);
    // wait for 16 bits data receiving complete
    while (LL_SPI_IsActiveFlag_RXNE(SPI2) == RESET) {}
    return LL_SPI_ReceiveData16(SPI2);
}

static void select_chip(void) {
    LL_GPIO_ResetOutputPin(ENCODER_GPIO_PORT, ENCODER_CS_PIN);
    // min delay 250 ns
    delay_nus_72MHz(2);
}

static void deselect_chip(void) {
    // min delay here: clock period / 2, our baud rate period is 222ns
    delay_nus_72MHz(2);
    LL_GPIO_SetOutputPin(ENCODER_GPIO_PORT, ENCODER_CS_PIN);
    // min delay until next read: 250ns
    delay_nus_72MHz(2);
}

static BOOL is_error(void) {
    return data_check.error_check;
}

static BOOL is_valid(void) {
    return data_check.parity_check;
}

/**
 * @brief Check parity
 * @param data data needs to be checked
 * @return if the number of '1' is odd, return '1', else return '0'
 * @ref https://blog.csdn.net/ftswsfb/article/details/104321901
 */
static BOOL parity_check(uint16_t data)
{
    BOOL parity = RESET;
    while (data) {
        parity = !parity;
        data &= data - 1;
    }
    return parity;
}

static uint16_t read_raw_angle(void) {
    SC60228Angle result;
    select_chip();
    result.reg = spi2_transmit_rw(0x0000);
    deselect_chip();
    data_check.error_check = (result.err == 1);
    data_check.parity_check = parity_check(result.angle);

    return result.angle;
}

static float get_angle(void) {
    uint8_t count = 0;
    float raw_angle_data = read_raw_angle();
    // invalid angle is a small probability event
    while (!is_valid() && count < 10) {
        raw_angle_data = read_raw_angle();
        count++;
    }

    float d_raw_angle = raw_angle_data - raw_angle_data_prev;
    // record the whole turns angles(according to d_raw_angle's direction)
    if(abs(d_raw_angle) > qfp_fmul(0.8, (float)ENCODER_RESOLUTION)) {
        rotation_turns_angles = qfp_fadd(d_raw_angle > 0 ? -_2PI : _2PI, rotation_turns_angles);
    }
    raw_angle_data_prev = raw_angle_data;
    // return current angle(rad)
    return qfp_fadd(qfp_fmul(qfp_fdiv(raw_angle_data, (float)ENCODER_RESOLUTION), _2PI), rotation_turns_angles);
}

static float get_velocity(void) {
    float timestamp, angle_curr, vel;

    uint32_t tick_now_us = SysTick->VAL; // 72 MHz clock rate -> SysTick (HCLK/8) is set to 9 MHz
    if (tick_now_us < vel_sample_timestamp) {
        timestamp = qfp_fmul(qfp_fdiv((float)(vel_sample_timestamp - tick_now_us), 9.0f), 1e-6f); // convert to sec
    } else {
        // SysTick->VAL only use 24 LSB bits, counting down
        timestamp = qfp_fmul(qfp_fdiv((float)((uint32_t)0xFFFFFF - tick_now_us + vel_sample_timestamp), 9.0f), 1e-6f);
    }
    // fix strange cases (overflow)
    if((timestamp < 1e-6f && timestamp > 1e-6f) || timestamp > 0.5f) {
        timestamp = 1e-3f;
    }

    // current angle
    angle_curr = get_angle();
    // angle change
    float d_angle = qfp_fsub(angle_curr, angle_prev);
    // velocity calculation
    vel = qfp_fdiv(d_angle, timestamp);

    // prepare for next calculation
    angle_prev = angle_curr;
    vel_sample_timestamp = tick_now_us;

    return vel;
}

static float vel_prev = 0;
static float LPF_velocity(float v) {
    float vel_curr = qfp_fadd(qfp_fmul(0.9f, vel_prev), qfp_fmul(0.1f, v));
    vel_prev = vel_curr;
    return vel_curr;
}

static float get_shaft_angle(void) {
    return qfp_fmul((float)g_sensor_dir, get_angle());
}

static float get_shaft_velocity(void) {
    return qfp_fmul((float)g_sensor_dir, LPF_velocity(get_velocity()));
}

static void encoder_gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable GPIOB clock
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);

    // Encoder CS pin Setting
    LL_GPIO_SetOutputPin(ENCODER_GPIO_PORT, ENCODER_CS_PIN);
    GPIO_InitStruct.Pin        = ENCODER_CS_PIN;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_UP;
    LL_GPIO_Init(ENCODER_GPIO_PORT, &GPIO_InitStruct);
}

static void encoder_spi2_init(void) {
    LL_SPI_InitTypeDef SPI_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    /** SPI2 GPIO Configuration
        => PB13   ------> SPI2_SCK
        => PB14   ------> SPI2_MISO
        => PB15   ------> SPI2_MOSI
    */
    GPIO_InitStruct.Pin        = ENCODER_SPI_SCK_PIN | ENCODER_SPI_MOSI_PIN;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(ENCODER_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin        = ENCODER_SPI_MISO_PIN;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_FLOATING;
    LL_GPIO_Init(ENCODER_GPIO_PORT, &GPIO_InitStruct);

    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
    SPI_InitStruct.Mode              = LL_SPI_MODE_MASTER;
    SPI_InitStruct.DataWidth         = LL_SPI_DATAWIDTH_16BIT;
    SPI_InitStruct.ClockPolarity     = LL_SPI_POLARITY_LOW;
    SPI_InitStruct.ClockPhase        = LL_SPI_PHASE_2EDGE;
    SPI_InitStruct.NSS               = LL_SPI_NSS_SOFT;
    SPI_InitStruct.BaudRate          = LL_SPI_BAUDRATEPRESCALER_DIV8;
    SPI_InitStruct.BitOrder          = LL_SPI_MSB_FIRST;
    SPI_InitStruct.CRCCalculation    = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.CRCPoly           = 10;
    LL_SPI_Init(SPI2, &SPI_InitStruct);

    // enable spi2
    LL_SPI_Enable(SPI2);
}


void encoder_init(void) {
    encoder_gpio_init();
    encoder_spi2_init();

    LL_mDelay(10);

    deselect_chip();
    // init g_encoder
    g_encoder.get_shaft_angle    = get_shaft_angle; // needs to check ERROR_CODE
    g_encoder.get_shaft_velocity = get_shaft_velocity;
    g_encoder.is_error           = is_error;

    // Get the initial motor magnetic angle position
    // Ensure the shaft velocity to be zero
    raw_angle_data_prev          = (float)read_raw_angle();

    // electrical direction needs to be correspond to the mechanical angle
    g_sensor_dir = CW;
}


#include "vofa_usart.h"
static float encoder_test_buf[1];
void encoder_test(void) {
    encoder_test_buf[0] = g_encoder.get_shaft_angle();
    if (g_encoder.is_error()) {
        encoder_test_buf[0] = -1.11;
    }
    vofa_usart_dma_send_config(encoder_test_buf, 1);
    LL_mDelay(100);
}