#include "foc.h"
#include "config.h"
#include "bldc_config.h"
#include "qfplib-m3.h"
#include <math.h>

// normalize angle to range [0, 2PI]
static float normalize_angle(float angle) {
    float _fmod = fmodf(angle, _2PI);
    return _fmod >= 0 ? _fmod : qfp_fadd(_fmod, _2PI);
}

float get_electrical_angle(float shaft_angle) {
    return normalize_angle(qfp_fmul(shaft_angle, (float)MOTOR_POLE_PAIRS));
}


void setPhaseVoltage(float Uq, float Ud, float e_angle) {
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
