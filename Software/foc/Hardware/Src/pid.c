#include "pid.h"
#include "config.h"
#include "qfplib-m3.h"
#include "foc.h"

// ref: https://docs.simplefoc.com/docs_chinese/pi_controller

TorCtrlParam g_tor_ctrl;
VelCtrlParam g_vel_ctrl;
AngCtrlParam g_ang_ctrl;
CurrCtrlParam g_Iq_ctrl;
CurrCtrlParam g_Id_ctrl;

float PID_torque(float err) {
    float proportional, output;

    // u_p  = P *e(k)
    proportional = qfp_fmul(g_tor_ctrl.pid.Kp, err);

    output = proportional;
    if (g_foc.torque_type_ == FOC_Torque_Type_Voltage) {
        output = constrain(output, -g_tor_ctrl.voltage_limit, g_tor_ctrl.voltage_limit);
    } else if (g_foc.torque_type_ == FOC_Torque_Type_Current) {
        output = constrain(output, -g_tor_ctrl.current_limit, g_tor_ctrl.current_limit);
    } else {
        output = 0.0f;
    }

    return output;
}

/**
 * @note If we use 'Torque_Voltage', PID_velocity will directly calculate the
 *       output voltage. So the constrain is the voltage.
 *       If we use 'Torque_Current', PID_velocity will calculate the output
 *       current. So the constrain is the current.
 */
float PID_velocity(float err) {
    float proportional, integral, derivative, output;
    float output_rate;

    // u_p  = P *e(k)
    proportional = qfp_fmul(g_vel_ctrl.pid.Kp, err);

    // Tustin transform of the integral part
    // u_ik = u_ik_1  + I*Ts/2*(ek + ek_1)
    integral =
        qfp_fadd(
            g_vel_ctrl.prev_data.integral,
            qfp_fmul(
                qfp_fmul(g_vel_ctrl.pid.Ki, g_vel_ctrl.ctrl_rate),
                qfp_fmul(qfp_fadd(err, g_vel_ctrl.prev_data.err), 0.5f) // filter
            )
        );

    // avoid integral saturation (diff Uq and Iq)
    if (g_foc.torque_type_ == FOC_Torque_Type_Voltage) {
        integral = constrain(integral, -g_vel_ctrl.voltage_limit, g_vel_ctrl.voltage_limit);
    } else if (g_foc.torque_type_ == FOC_Torque_Type_Current) {
        integral = constrain(integral, -g_vel_ctrl.current_limit, g_vel_ctrl.current_limit);
    }

    // u_dk = D(ek - ek_1)/Ts
    derivative = qfp_fdiv(qfp_fmul(g_vel_ctrl.pid.Kd, qfp_fsub(err, g_vel_ctrl.prev_data.err)), g_vel_ctrl.ctrl_rate);

    output = qfp_fadd(qfp_fadd(proportional, integral), derivative);
    if (g_foc.torque_type_ == FOC_Torque_Type_Voltage) {
        output = constrain(output, -g_vel_ctrl.voltage_limit, g_vel_ctrl.voltage_limit);
        // limit the acceleration by ramping the output
        output_rate = qfp_fdiv(qfp_fsub(output, g_vel_ctrl.prev_data.output), g_vel_ctrl.ctrl_rate);
        if(output_rate > g_vel_ctrl.voltage_ramp) {
            output = qfp_fadd(g_vel_ctrl.prev_data.output, qfp_fmul(g_vel_ctrl.voltage_ramp, g_vel_ctrl.ctrl_rate));
        } else if (output_rate < - g_vel_ctrl.voltage_ramp) {
            output = qfp_fsub(g_vel_ctrl.prev_data.output, qfp_fmul(g_vel_ctrl.voltage_ramp, g_vel_ctrl.ctrl_rate));
        }
    } else if (g_foc.torque_type_ == FOC_Torque_Type_Current) {
        output = constrain(output, -g_vel_ctrl.current_limit, g_vel_ctrl.current_limit);
    }

    g_vel_ctrl.prev_data.integral     = integral;
    g_vel_ctrl.prev_data.output       = output;
    g_vel_ctrl.prev_data.err          = err;

    return output;
}

// P controller
float PID_angle(float err) {
    float proportional, output;

    // If the 'err' is too small, I don't want the motor to adjust itself.
    if (abs(err) < 0.05f) {
        return 0.0f;
    }

    // u_p  = P *e(k)
    proportional = qfp_fmul(g_ang_ctrl.pid.Kp, err);

    output = proportional;
    output = constrain(output, -g_ang_ctrl.velocity_limit, g_ang_ctrl.velocity_limit);

    return output;
}

// PI controller
float PID_current(CurrCtrlParam *ctrl, float err) {
    float proportional, integral, output;
    float output_rate;

    // u_p  = P *e(k)
    proportional = qfp_fmul(ctrl->pid.Kp, err);

    // Tustin transform of the integral part
    // u_ik = u_ik_1  + I*Ts/2*(ek + ek_1)
    integral =
        qfp_fadd(
            ctrl->prev_data.integral,
            qfp_fmul(
                qfp_fmul(ctrl->pid.Ki, ctrl->ctrl_rate),
                qfp_fmul(qfp_fadd(err, ctrl->prev_data.err), 0.5f) // filter
            )
        );

    // avoid integral saturation (diff Uq and Iq)
    integral = constrain(integral, -ctrl->voltage_limit, ctrl->voltage_limit);

    output = qfp_fadd(proportional, integral);
    output = constrain(output, -ctrl->voltage_limit, ctrl->voltage_limit);

    // limit the acceleration by ramping the output
    output_rate = qfp_fdiv(qfp_fsub(output, ctrl->prev_data.output), ctrl->ctrl_rate);
    if(output_rate > ctrl->voltage_ramp) {
        output = qfp_fadd(ctrl->prev_data.output, qfp_fmul(ctrl->voltage_ramp, ctrl->ctrl_rate));
    } else if (output_rate < - ctrl->voltage_ramp) {
        output = qfp_fsub(ctrl->prev_data.output, qfp_fmul(ctrl->voltage_ramp, ctrl->ctrl_rate));
    }

    ctrl->prev_data.integral     = integral;
    ctrl->prev_data.output       = output;
    ctrl->prev_data.err          = err;

    return output;
}

void pid_init(void) {
    pid_clear_history();
    // init torque control parameters
    g_tor_ctrl.pid.Kp = 0.0f;
    g_tor_ctrl.pid.Ki = 0.0f;
    g_tor_ctrl.pid.Kd = 0.0f;
    // torque needs to be greater than 0.6 under current loop control
    g_tor_ctrl.target_torque = 0.0f;
    g_tor_ctrl.voltage_limit = FOC_VOLTAGE_LIMIT;
    g_tor_ctrl.current_limit = FOC_CURRENT_LIMIT;

    // init velocity control parameters
    g_vel_ctrl.pid.Kp = 0.05f;
    g_vel_ctrl.pid.Ki = 1.0f;
    g_vel_ctrl.pid.Kd = 0.001f;

    g_vel_ctrl.voltage_limit = FOC_VOLTAGE_LIMIT;
    g_vel_ctrl.current_limit = FOC_CURRENT_LIMIT;
    g_vel_ctrl.target_speed  = 0.0f;
    g_vel_ctrl.voltage_ramp  = FOC_VOLTAGE_RAMP;
    g_vel_ctrl.ctrl_rate     = FOC_CONTROL_RATE; // TIM2 is set to 1 KHz


    // init angle control parameters
    g_ang_ctrl.pid.Kp = 4.0f;
    g_ang_ctrl.pid.Ki = 0.0f; // unused
    g_ang_ctrl.pid.Kd = 0.0f; // unused

    g_ang_ctrl.velocity_limit = FOC_ANG_SPEED_LIMIT;
    g_ang_ctrl.target_angle   = 0.0f;
    g_ang_ctrl.ctrl_rate      = FOC_CONTROL_RATE;


    // init current control parameters
    // Iq
    g_Iq_ctrl.pid.Kp = 0.6f;
    g_Iq_ctrl.pid.Ki = 0.0f;
    g_Iq_ctrl.pid.Kd = 0.0f;

    g_Iq_ctrl.voltage_limit = FOC_VOLTAGE_LIMIT;
    g_Iq_ctrl.voltage_ramp  = FOC_VOLTAGE_RAMP;
    g_Iq_ctrl.ctrl_rate     = FOC_CONTROL_RATE;


    // Id
    g_Id_ctrl.pid.Kp = 0.6f;
    g_Id_ctrl.pid.Ki = 0.0f;
    g_Id_ctrl.pid.Kd = 0.0f;

    g_Id_ctrl.voltage_limit = FOC_VOLTAGE_LIMIT;
    g_Id_ctrl.voltage_ramp  = FOC_VOLTAGE_RAMP;
    g_Id_ctrl.ctrl_rate     = FOC_CONTROL_RATE;
}

void pid_clear_history(void) {
    g_vel_ctrl.prev_data.err        = 0.0f;
    g_vel_ctrl.prev_data.integral   = 0.0f;
    g_vel_ctrl.prev_data.derivative = 0.0f;
    g_vel_ctrl.prev_data.output     = 0.0f;

    g_Iq_ctrl.prev_data.err         = 0.0f;
    g_Iq_ctrl.prev_data.integral    = 0.0f;
    g_Iq_ctrl.prev_data.derivative  = 0.0f;
    g_Iq_ctrl.prev_data.output      = 0.0f;

    g_Id_ctrl.prev_data.err         = 0.0f;
    g_Id_ctrl.prev_data.integral    = 0.0f;
    g_Id_ctrl.prev_data.derivative  = 0.0f;
    g_Id_ctrl.prev_data.output      = 0.0f;
}