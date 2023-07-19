#include "spi.h"
#include "encoder.h"
#include "config.h"
#include "delay.h"
#include "qfplib-m3.h"

#define abs(x) ((x) > 0 ? (x) : (-(x)))
#define _2PI 6.28318530718

Encoder g_encoder;

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
static float cumulative_angle_prev = 0; // last angle value(radian)
static float rotation_turns_angles = 0; // how may turns the motor runs(radian)

static void select_chip() {
    LL_GPIO_SetOutputPin(ENCODER_GPIO_PORT, ENCODER_CS_PIN);
    // min delay 250 ns
    delay_nus_72MHz(1);
}

static void deselect_chip() {
    // min delay here: clock period / 2, our baud rate period is 222ns
    delay_nus_72MHz(1);
    LL_GPIO_ResetOutputPin(ENCODER_GPIO_PORT, ENCODER_CS_PIN);
    // min delay until next read: 250ns
    delay_nus_72MHz(1);
}

static BOOL is_error() {
    return data_check.error_check;
}

static BOOL is_valid() {
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

static uint16_t read_raw_angle() {
    SC60228Angle result;
    select_chip();
    result.reg = spi_transmit_receive();
    deselect_chip();
    data_check.error_check = (result.err == 1);
    data_check.parity_check = parity_check(result.angle);

    return result.angle;
}

static float get_angle() {
    if (is_valid()) {
        float raw_angle_data = read_raw_angle();
        float d_raw_angle = raw_angle_data - raw_angle_data_prev;
        if(abs(d_raw_angle) > (0.8 * ENCODER_RESOLUTION)) {
            rotation_turns_angles += (d_raw_angle > 0 ? -_2PI : _2PI);
        }
        raw_angle_data_prev = raw_angle_data;
        // return current angle(rad)
        return qfp_fmul(qfp_fdiv(raw_angle_data, (float)ENCODER_RESOLUTION), _2PI);
    }
    return raw_angle_data_prev;
}

static float get_velocity() {
    float timestamp, cumulative_angle_curr, vel;

    uint32_t tick_now_us = SysTick->VAL; // 72 MHz clock rate -> SysTick (HCLK/8) is set to 9 MHz
    if (tick_now_us < vel_sample_timestamp) {
        timestamp = qfp_fmul(qfp_fdiv((float)(vel_sample_timestamp - tick_now_us), 9), 1e-6); // convert to sec
    } else {
        // SysTick->VAL only use 24 LSB bits, counting down
        timestamp = qfp_fmul(qfp_fdiv(((uint32_t)0xFFFFFF - tick_now_us + vel_sample_timestamp), 9), 1e-6);
    }
    // fix strange cases (overflow)
    if(timestamp == 0 || timestamp > 0.5) {
        timestamp = 1e-3;
    }

    cumulative_angle_curr = rotation_turns_angles + get_angle();
    vel = qfp_fdiv(cumulative_angle_curr - cumulative_angle_prev, timestamp);

    // prepare for next calculation
    cumulative_angle_prev = cumulative_angle_curr;
    vel_sample_timestamp = tick_now_us;

    return vel;
}

void encoder_init() {
    deselect_chip();
    LL_SPI_Enable(SPI1);
    // init g_encoder
    g_encoder.get_angle = get_angle;
    g_encoder.get_velocity = get_velocity;
    g_encoder.is_error = is_error;
    // get the initial motor magnetic angle position
    raw_angle_data_prev = (float)read_raw_angle();
}
