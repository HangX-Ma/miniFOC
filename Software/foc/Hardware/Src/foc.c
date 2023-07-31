#include "foc.h"
#include "config.h"
#include "bldc_config.h"
#include "encoder.h"
#include "qfplib-m3.h"
#include "vofa_usart.h"
#include "led.h"
#include "pid.h"
#include <math.h>
#include "current_monitor.h"

#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_tim.h"

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

// Encoder DIR needs to be reserved. Otherwise, the FOC will stick into one
// position if you call any function that rely on the encoder feedback.
#define ESTIMATE_ENCODER_DIR    (1)
static void align_sensor(void) {
    // electrical direction needs to be correspond to the mechanical angle
    float e_angle; // electrical angle
    float forward_angle, back_angle, delta_abs_angle; // mechanic angle

    printf("\n[Motor]: Start sensor alignment...\r\n");

    // we need to start pwm output first
    LED_STATE_ON();
    g_bldc.start_pwm();
    LL_mDelay(500);

    /* We want to ensure the sensor direction and the pole pairs number */
    // forward 2PI electrical angle
    for(int i = 0; i <= 100; i++) {
        e_angle = qfp_fadd(_3PI_2, qfp_fdiv(qfp_fmul(_2PI, i), 100.0f));
        g_foc.set_phase_voltage(SENSOR_ALIGN_VOLTAGE, 0, e_angle);
        LL_mDelay(20);
    }
    forward_angle = g_encoder.get_angle();

    // turn back
    for(int i = 100; i >= 0; i--) {
        e_angle = qfp_fadd(_3PI_2, qfp_fdiv(qfp_fmul(_2PI, i), 100.0f));
        g_foc.set_phase_voltage(SENSOR_ALIGN_VOLTAGE, 0, e_angle);
        LL_mDelay(20);
    }
    back_angle = g_encoder.get_angle();

    printf("[Motor]: Forward angle is degree %d\r\n", (int)qfp_fmul(qfp_fdiv(forward_angle, _PI), 180.0f));
    printf("[Motor]: Back angle is degree %d\r\n", (int)qfp_fmul(qfp_fdiv(back_angle, _PI), 180.0f));

    // Try to stop motor at zero point
    g_foc.set_phase_voltage(0, 0, 0);
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

#if ESTIMATE_ENCODER_DIR
    //* determine the sensor direction
    if(forward_angle < back_angle) {
        printf("[Motor]: Encoder dir is CCW\r\n");
        g_encoder.dir_ = CCW;
    }
    else {
        printf("[Motor]: Encoder dir is CW\r\n");
        g_encoder.dir_ = CW;
    }
#else
    g_encoder.dir_ = CCW;
#endif

#if ESTIMATE_POLE_PAIRS
    //* calculate motor pole pairs
    printf("[Motor]: Pole pairs checking...\r\n");
    // 0.5 is arbitrary number it can be lower or higher!
    // If the default pole pairs isn't correct, we use the calculated one!
    if( abs(qfp_fsub(qfp_fmul(delta_abs_angle, g_foc.property_.pole_pairs), _2PI)) > 0.5f ) {
        g_foc.property_.pole_pairs = qfp_fadd(qfp_fdiv(_2PI, delta_abs_angle), 0.5f);
        if (g_foc.property_.pole_pairs > MOTOR_POLE_PAIRS + 1 || g_foc.property_.pole_pairs < MOTOR_POLE_PAIRS - 1) {
            g_foc.property_.pole_pairs = MOTOR_POLE_PAIRS;
            printf("[Motor]: Estimate pole pairs error.\n");
            LED_STATE_OFF();
            // stop pwm output. Motor will be stopped and this also can avoid emergency situation
            g_bldc.stop_pwm();
            return;
        } else {
            printf("[Motor]: Estimated pole pairs = %d\r\n", g_foc.property_.pole_pairs);
        }
    } else {
        printf("[Motor]: Ok!\r\n");
    }
#endif

    g_foc.set_phase_voltage(SENSOR_ALIGN_VOLTAGE, 0, _3PI_2);
    LL_mDelay(1000);

    // collect the current mechanical angle to calculate the zero electrical angle offset
    g_foc.property_.zero_electrical_angle_offset =
            normalize_angle(qfp_fmul(g_encoder.get_shaft_angle(), g_foc.property_.pole_pairs));
    printf("[Motor]: Zero electrical angle is degree %d\r\n",
            (int)qfp_fmul(qfp_fdiv(g_foc.property_.zero_electrical_angle_offset, _PI), 180.0f));
    LL_mDelay(20);
    // Try to stop motor at zero point
    g_foc.set_phase_voltage(0, 0, 0);
    LL_mDelay(200);

    LED_STATE_OFF();
    // stop pwm output. Motor will be stopped and this also can avoid emergency situation
    g_bldc.stop_pwm();
}

static void vel_ctrl_tim2_init(void) {
    LL_TIM_InitTypeDef TIM_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

    /* TIM2 interrupt Init */
    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(TIM2_IRQn);

        // Velocity control frequency: 72 MHz / 72000 = 1 KHz
    TIM_InitStruct.Prescaler     = 36 - 1;
    TIM_InitStruct.CounterMode   = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload    = 2000 - 1;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM2, &TIM_InitStruct);

    LL_TIM_EnableARRPreload(TIM2);
    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM2);
}

//! ------------------- FOC CONTROL -------------------
#include "foc_app.h"
static float shaft_speed;
static float target_q;
static RotorStatorCurrent RS_current;

void FOC_CTRL_IRQHandler(void) {
    if (LL_TIM_IsActiveFlag_UPDATE(TIM2) != RESET) {
        g_foc.state_.shaft_angle      = g_encoder.get_shaft_angle();
        g_foc.state_.electrical_angle = g_foc.get_electrical_angle(g_foc.state_.shaft_angle);
        g_foc.state_.shaft_speed      = g_encoder.get_shaft_velocity();

        switch (g_foc.motion_type_) {
            case FOC_Motion_Type_Torque:
                ratchet_mode();
                target_q = PID_torque(g_tor_ctrl.target_torque);
                break;
            case FOC_Motion_Type_Velocity:
                target_q = PID_velocity(qfp_fsub(g_vel_ctrl.target_speed, g_foc.state_.shaft_speed));
                break;
            case FOC_Motion_Type_Angle:
                shaft_speed = PID_angle(qfp_fsub(g_ang_ctrl.target_angle, g_foc.state_.shaft_angle));
                target_q    = PID_velocity(qfp_fsub(shaft_speed, g_foc.state_.shaft_speed /* actual speed */));
                break;
            default:
                // Nothing happens...
                goto out;
        }
        if (g_foc.torque_type_ == FOC_Torque_Type_Voltage) {
            g_foc.voltage_.q = target_q;
            g_foc.voltage_.d = 0.0f;
        } else if (g_foc.torque_type_ == FOC_Torque_Type_Current) {
            RS_current = get_RS_current(g_foc.state_.electrical_angle);
            g_foc.voltage_.q = PID_current(&g_Iq_ctrl, qfp_fsub(target_q, RS_current.Iq));
            g_foc.voltage_.d = PID_current(&g_Id_ctrl, /* target_d = 0.0f */ -RS_current.Id);
            // save 'd' 'q' state for easy debug
            // g_foc.state_.q   = RS_current.Iq;
            // g_foc.state_.d   = RS_current.Id
            g_foc.state_.q   = g_foc.voltage_.q;
            g_foc.state_.d   = g_foc.voltage_.d;
        } else {
            // avoid undefined situation. Prohibit the motor running.
            g_foc.voltage_.q = 0.0f;
            g_foc.voltage_.d = 0.0f;
        }
        g_foc.set_phase_voltage(g_foc.voltage_.q, g_foc.voltage_.d, g_foc.state_.electrical_angle);
out:
        LL_TIM_ClearFlag_UPDATE(TIM2);
    }
}

static void foc_start(void) {
    // We only reset those values when we switch motion type.
    // Otherwise, this will lead to a motor jump.
    if (g_foc.state_.switch_type == TRUE) {
        g_foc.voltage_.d = 0.0f;
        g_foc.voltage_.q = 0.0f;
        pid_clear_history();
        current_monitor_reset();

        // reset switch flag
        g_foc.state_.switch_type = FALSE;
        encoder_reset();
    }

    g_bldc.start_pwm();
    // enable TIM2 update
    LL_TIM_EnableIT_UPDATE(TIM2);
    // start TIM2 counter
    LL_TIM_EnableCounter(TIM2);
}

static void foc_stop(void) {
    g_bldc.stop_pwm();
    // start TIM2 counter
    LL_TIM_DisableCounter(TIM2);
    // enable TIM2 update
    LL_TIM_DisableIT_UPDATE(TIM2);
}

//! YOU MUST CALL ENCODER INIT FIRST
void foc_init(void) {
    g_foc.property_.pole_pairs = MOTOR_POLE_PAIRS;
    g_foc.property_.zero_electrical_angle_offset = 0.0f;

    g_foc.state_.shaft_angle      = 0.0f;
    g_foc.state_.shaft_speed      = 0.0f;
    g_foc.state_.electrical_angle = 0.0f;
    g_foc.state_.q                = 0.0f;
    g_foc.state_.d                = 0.0f;
    g_foc.state_.switch_type      = TRUE;

    g_foc.ctrl_.start             = foc_start;
    g_foc.ctrl_.stop              = foc_stop;

    g_foc.motion_type_            = FOC_Motion_Type_Torque;
    g_foc.torque_type_            = FOC_Torque_Type_Voltage;
    g_foc.voltage_.d              = 0.0f;
    g_foc.voltage_.q              = 0.0f;

    g_foc.set_phase_voltage       = set_phase_voltage;
    g_foc.get_electrical_angle    = get_electrical_angle;
    g_foc.align_sensor            = align_sensor;

    vel_ctrl_tim2_init();
}

