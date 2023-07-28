#include "foc.h"
#include "config.h"
#include "bldc_config.h"
#include "encoder.h"
#include "qfplib-m3.h"
#include "vofa_usart.h"
#include "led.h"
#include <math.h>

#include "stm32f1xx_ll_utils.h"

FOC g_foc;

// normalize angle to range [0, 2PI]
static float normalize_angle(float angle) {
    float _fmod = fmodf(angle, _2PI);
    return _fmod >= 0 ? _fmod : qfp_fadd(_fmod, _2PI);
}

static float get_electrical_angle(float shaft_angle) {
    return normalize_angle(
        qfp_fsub(
            qfp_fmul(shaft_angle, (float)g_foc.property_.pole_pairs),
            g_foc.property_.zero_electrical_angle_offset
        )
    );
}


static void set_phase_voltage(float Uq, float Ud, float e_angle) {
    float U_ref, U_alpha, U_beta;
    float T0, T1, T2;
    float Ta, Tb, Tc;
    uint8_t sector;

    // invert park
    U_alpha = qfp_fsub(qfp_fmul(Ud, qfp_fcos(e_angle)), qfp_fmul(Uq, qfp_fsin(e_angle)));
    U_beta  = qfp_fadd(qfp_fmul(Ud, qfp_fsin(e_angle)), qfp_fmul(Uq, qfp_fcos(e_angle)));

    // normalized U_ref range between [-1, 1]
    U_ref = qfp_fdiv(qfp_fsqrt(qfp_fadd(qfp_fmul(U_alpha, U_alpha), qfp_fmul(U_beta, U_beta))), MOTOR_VM);

    // SVPWM maximum distortion-free rotation voltage vector: sqrt(3)/3
    // The inscribed circle of a hexagon.
    if (U_ref > 0.577f) {
        U_ref = 0.577f;
    }
    if (U_ref < -0.577f) {
        U_ref = -0.577f;
    }

    // We need to calculate the rotor's electric angle. THe rotor is _PI_2 head of the stator.
    if (Uq > 0) {
        e_angle = normalize_angle(qfp_fadd(e_angle, _PI_2));
    } else {
        e_angle = normalize_angle(qfp_fsub(e_angle, _PI_2));
    }

    // calculate the sector
    sector = (uint8_t) qfp_fdiv(e_angle, _PI_3) + 1;

    // calculate the neighbour voltage vector action time

    float m = qfp_fmul(_SQRT3, U_ref);
    T1 = qfp_fmul(m, qfp_fsin(qfp_fsub(qfp_fmul(sector, _PI_3), e_angle)));
    T2 = qfp_fmul(m, qfp_fsin(qfp_fsub(e_angle, qfp_fmul(qfp_fsub(sector, 1.0f), _PI_3))));
    T0 = qfp_fsub(qfp_fsub(1.0f, T1), T2);

    float T0_2 = qfp_fdiv(T0, 2.0f);

    switch(sector)
    {
        case 1:
            Ta = qfp_fadd(qfp_fadd(T1, T2), T0_2);
            Tb = qfp_fadd(T2, T0_2);
            Tc = T0_2;
            break;
        case 2:
            Ta = qfp_fadd(T1, T0_2);
            Tb = qfp_fadd(qfp_fadd(T1, T2), T0_2);
            Tc = T0_2;
            break;
        case 3:
            Ta = T0_2;
            Tb = qfp_fadd(qfp_fadd(T1, T2), T0_2);
            Tc = qfp_fadd(T2, T0_2);
            break;
        case 4:
            Ta = T0_2;
            Tb = qfp_fadd(T1, T0_2);
            Tc = qfp_fadd(qfp_fadd(T1, T2), T0_2);
            break;
        case 5:
            Ta = qfp_fadd(T2, T0_2);
            Tb = T0_2;
            Tc = qfp_fadd(qfp_fadd(T1, T2), T0_2);
            break;
        case 6:
            Ta = qfp_fadd(qfp_fadd(T1, T2), T0_2);
            Tb = T0_2;
            Tc = qfp_fadd(T1, T0_2);
            break;
        default:  // possible error state
            Ta = 0.0f;
            Tb = 0.0f;
            Tc = 0.0f;
    }
    g_bldc.set_pwm_a_duty((uint32_t)qfp_fmul(Ta, (float)PWM_RELOAD_PERIOD));
    g_bldc.set_pwm_b_duty((uint32_t)qfp_fmul(Tb, (float)PWM_RELOAD_PERIOD));
    g_bldc.set_pwm_c_duty((uint32_t)qfp_fmul(Tc, (float)PWM_RELOAD_PERIOD));
}

static void align_sensor(void) {
    // electrical direction needs to be correspond to the mechanical angle
    float e_angle; // electrical angle
    float forward_angle, back_angle, delta_abs_angle; // mechanic angle

    printf("[Motor]: Start sensor alignment...\r\n");

    // we need to start pwm output first
    LED_STATE_OFF();
    g_bldc.start_pwm();

    /* We want to ensure the sensor direction and the pole pairs number */
    // forward 2PI electrical angle
    for(int i = 0; i <= 500; i++) {
        e_angle = qfp_fadd(-_PI_2, qfp_fdiv(qfp_fmul(_2PI, i), 500.0f));
        g_foc.set_phase_voltage(SENSOR_ALIGN_VOLTAGE, 0, e_angle);
        LL_mDelay(2);
    }
    forward_angle = g_encoder.get_angle();

    // turn back
    for(int i = 500; i >= 0; i--) {
        e_angle = qfp_fadd(-_PI_2, qfp_fdiv(qfp_fmul(_2PI, i), 500.0f));
        g_foc.set_phase_voltage(SENSOR_ALIGN_VOLTAGE, 0, e_angle);
        LL_mDelay(2);
    }
    back_angle = g_encoder.get_angle();

    printf("[Motor]: Forward angle is %.3f\r\n", forward_angle);
    printf("[Motor]: Back angle is %.3f\r\n", back_angle);

    // Try to stop motor at zero point
    g_foc.set_phase_voltage(0, 0, -_PI_2);
    LL_mDelay(200);

    // Motor moves none, which indicates motor error.
    delta_abs_angle = abs(qfp_fsub(forward_angle, back_angle));
    if(( delta_abs_angle < 0.02f)) {
        printf("[Motor]: Abnormal state. Please check encoder or motor.\n");
        LED_STATE_OFF();
        // stop pwm output. Motor will be stopped and this also can avoid emergency situation
        g_bldc.stop_pwm();
        return;
    }

    //* determine the sensor direction
    if(forward_angle < back_angle) {
        printf("[Motor]: Encoder dir is CCW\r\n");
        g_encoder.dir_ = CCW;
    }
    else {
        printf("[Motor]: Encoder dir is CW\r\n");
        g_encoder.dir_ = CW;
    }

    //* calculate motor pole pairs
    printf("[Motor]: Pole pairs checking...\r\n");
    // 0.5 is arbitrary number it can be lower or higher!
    // If the default pole pairs isn't correct, we use the calculated one!
    if( abs(qfp_fsub(qfp_fmul(delta_abs_angle, g_foc.property_.pole_pairs), _2PI)) > 0.5f ) {
        g_foc.property_.pole_pairs = qfp_fadd(qfp_fdiv(_2PI, delta_abs_angle), 0.5f);
        printf("[Motor]: Estimated pole pairs = %d\r\n", g_foc.property_.pole_pairs);
    } else {
        printf("[Motor]: Ok!\r\n");
    }

    // lock electrical angle to zero
    g_foc.set_phase_voltage(SENSOR_ALIGN_VOLTAGE, 0, -_PI_2);
    LL_mDelay(500);

    // collect the current mechanical angle to calculate the zero electrical angle offset
    g_foc.property_.zero_electrical_angle_offset = normalize_angle(qfp_fmul(g_encoder.get_shaft_angle(), g_foc.property_.pole_pairs));
    printf("[Motor]: Zero electrical angle: %.3f\r\n", g_foc.property_.zero_electrical_angle_offset);
    LL_mDelay(100);

    // Try to stop motor at zero point
    g_foc.set_phase_voltage(0, 0, -_PI_2);
    LL_mDelay(200);

    LED_STATE_OFF();
    // stop pwm output. Motor will be stopped and this also can avoid emergency situation
    g_bldc.stop_pwm();
}

//! YOU MUST CALL ENCODER INIT FIRST
void foc_init(void) {
    g_foc.property_.pole_pairs = MOTOR_POLE_PAIRS;
    g_foc.property_.zero_electrical_angle_offset = 0.0f;

    g_foc.set_phase_voltage    = set_phase_voltage;
    g_foc.get_electrical_angle = get_electrical_angle;
    g_foc.align_sensor         = align_sensor;
}