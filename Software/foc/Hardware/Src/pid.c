#include "pid.h"
#include "config.h"
#include "qfplib-m3.h"

// ref: https://docs.simplefoc.com/docs_chinese/pi_controller

VelCtrlParam g_vel_ctrl;
AngCtrlParam g_ang_ctrl;

typedef struct PrevData {
    float err;
    float integral;
    float derivative;
    float output;
} PrevData;

static PrevData vel_prev = {0};
float PID_velocity(float err) {
    float proportional, integral, derivative, output;
    float output_rate;

    // u_p  = P *e(k)
    proportional = qfp_fmul(g_vel_ctrl.pid.Kp, err);

    // Tustin transform of the integral part
    // u_ik = u_ik_1  + I*Ts/2*(ek + ek_1)
    integral =
        qfp_fadd(
            vel_prev.integral,
            qfp_fmul(
                qfp_fmul(g_vel_ctrl.pid.Ki, g_vel_ctrl.ctrl_rate),
                qfp_fmul(qfp_fadd(err, vel_prev.err), 0.5f) // filter
            )
        );
    // avoid integral saturation
    integral = constrain(integral, -g_vel_ctrl.voltage_limit, g_vel_ctrl.voltage_limit);

    // u_dk = D(ek - ek_1)/Ts
    derivative = qfp_fdiv(qfp_fmul(g_vel_ctrl.pid.Kd, qfp_fsub(err, vel_prev.err)), g_vel_ctrl.ctrl_rate);

    output = qfp_fadd(qfp_fadd(proportional, integral), derivative);
    output = constrain(output, -g_vel_ctrl.voltage_limit, g_vel_ctrl.voltage_limit);

    // limit the acceleration by ramping the output
    output_rate = qfp_fdiv(qfp_fsub(output, vel_prev.output), g_vel_ctrl.ctrl_rate);
    if(output_rate > g_vel_ctrl.voltage_output_ramp) {
        output = qfp_fadd(vel_prev.output, qfp_fmul(g_vel_ctrl.voltage_output_ramp, g_vel_ctrl.ctrl_rate));
    } else if (output_rate < - g_vel_ctrl.voltage_output_ramp) {
        output = qfp_fsub(vel_prev.output, qfp_fmul(g_vel_ctrl.voltage_output_ramp, g_vel_ctrl.ctrl_rate));
    }

    vel_prev.integral     = integral;
    vel_prev.output       = output;
    vel_prev.err          = err;

    return output;
}

// PI controller
static PrevData ang_prev = {0};
float PID_angle(float err) {
    float proportional, integral, output;

    // u_p  = P *e(k)
    proportional = qfp_fmul(g_ang_ctrl.pid.Kp, err);

    // Tustin transform of the integral part
    // u_ik = u_ik_1  + I*Ts/2*(ek + ek_1)
    integral =
        qfp_fadd(
            ang_prev.integral,
            qfp_fmul(
                qfp_fmul(g_ang_ctrl.pid.Ki, g_ang_ctrl.ctrl_rate),
                qfp_fmul(qfp_fadd(err, ang_prev.err), 0.5f) // filter
            )
        );
    // avoid integral saturation
    integral = constrain(integral, -g_ang_ctrl.voltage_limit, g_ang_ctrl.voltage_limit);

    output = qfp_fadd(proportional, integral);
    output = constrain(output, -g_ang_ctrl.voltage_limit, g_ang_ctrl.voltage_limit);


    ang_prev.integral     = integral;
    ang_prev.err = err;

    return output;
}

void pid_init(void) {
    // init velocity control parameters
    g_vel_ctrl.pid.Kp = 0.05f;
    g_vel_ctrl.pid.Ki = 0.1f;
    g_vel_ctrl.pid.Kd = 0.001f;

    g_vel_ctrl.voltage_limit       = FOC_VOLTAGE_LIMIT;
    g_vel_ctrl.target_speed        = 0.0f;
    g_vel_ctrl.ctrl_rate           = FOC_CONTROL_RATE; // TIM2 is set to 1 KHz
    g_vel_ctrl.voltage_output_ramp = 100.0f;

    // init angle control parameters
    g_ang_ctrl.pid.Kp = 0.0f;
    g_ang_ctrl.pid.Ki = 0.0f;
    g_ang_ctrl.pid.Kd = 0.0f;

    g_ang_ctrl.voltage_limit = FOC_VOLTAGE_LIMIT;
    g_ang_ctrl.target_angle  = 0.0f;
    g_ang_ctrl.ctrl_rate     = FOC_CONTROL_RATE;
}

void pid_clear_history(void) {
    vel_prev.err        = 0.0f;
    vel_prev.integral   = 0.0f;
    vel_prev.derivative = 0.0f;
    vel_prev.output     = 0.0f;

    ang_prev.err        = 0.0f;
    ang_prev.integral   = 0.0f;
    ang_prev.derivative = 0.0f;
    ang_prev.output     = 0.0f;
}