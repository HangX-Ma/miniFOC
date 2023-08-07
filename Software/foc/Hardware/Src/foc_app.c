#include "foc_app.h"
#include "config.h"
#include "qfplib-m3.h"
#include "pid.h"
#include "foc.h"
#include "encoder.h"
#include <math.h>

FOCApp g_foc_app;
static FOCRatchet *foc_ratchet;
static FOCRebound *foc_rebound;

// normalize angle to range [0, 2PI]
static float normalize(float angle) {
    float _fmod = fmodf(angle, _2PI);
    return _fmod >= 0 ? _fmod : qfp_fadd(_fmod, _2PI);
}

float find_attractor(float angle){
    // angle dist according to attractor num
    float attractor_dist = qfp_fdiv(_2PI, foc_ratchet->attractor_num_);
    uint32_t idx = (uint32_t)qfp_fadd(qfp_fdiv(angle, attractor_dist), 0.5f);
    return qfp_fmul((float)idx, attractor_dist);
}

static float ratchet_normalized_angle = 0.0f; // normalized mechanical angle
static float ratchet_attract_angle    = 0.0f; // current attract angle
TorCtrlParam* torque_ratchet_mode(void) {
    ratchet_normalized_angle = normalize(g_foc.state_.shaft_angle);
    ratchet_attract_angle = find_attractor(ratchet_normalized_angle);
    foc_ratchet->torque_ctrl_.target_torque =
        qfp_fsub(ratchet_attract_angle, ratchet_normalized_angle);

    return &foc_ratchet->torque_ctrl_;
}

TorCtrlParam* torque_rebound_mode(void) {
    foc_rebound->torque_ctrl_.target_torque =
        qfp_fsub(foc_rebound->rebound_angle_, g_foc.state_.shaft_angle);

    return &foc_rebound->torque_ctrl_;
}


static void foc_app_reset_torque_ctrl(TorCtrlParam *pTorCtrl) {
    pTorCtrl->pid.Kp   = 0.0f;
    pTorCtrl->pid.Ki   = 0.0f; // unused in torque mode
    pTorCtrl->pid.Kd   = 0.0f; // unused in torque mode
    pTorCtrl->prev_err = 0.0f;
    // torque needs to be greater than 0.6 under current loop control
    pTorCtrl->target_torque = 0.0f;
    pTorCtrl->voltage_limit = FOC_VOLTAGE_LIMIT;
    pTorCtrl->current_limit = FOC_CURRENT_LIMIT;
    pTorCtrl->ctrl_rate     = FOC_CONTROL_RATE;
}

static void update_output_ratio(float output_ratio) {
    foc_rebound->output_ratio_ = output_ratio;
    foc_rebound->torque_ctrl_.voltage_limit = qfp_fdiv((float)FOC_VOLTAGE_LIMIT, foc_rebound->output_ratio_);
    foc_rebound->torque_ctrl_.current_limit = qfp_fdiv((float)FOC_CURRENT_LIMIT, foc_rebound->output_ratio_);
}

void foc_app_init(void) {
    // init torque normal mode
    foc_app_reset_torque_ctrl(&g_foc_app.normal_.torque_ctrl_);

    // init ratchet and set default value
    foc_app_reset_torque_ctrl(&g_foc_app.ratchet_.torque_ctrl_);
    foc_ratchet = &g_foc_app.ratchet_;
    foc_ratchet->attractor_num_             = 6.0f; // 60 degree per attractor
    foc_ratchet->torque_ctrl_.pid.Kp        = 0.1f;

    // set rebound angle to current shaft angle to avoid sudden motor rotation
    foc_app_reset_torque_ctrl(&g_foc_app.rebound_.torque_ctrl_);
    foc_rebound = &g_foc_app.rebound_;
    foc_rebound->torque_ctrl_.pid.Kp        = 0.1f;
    foc_rebound->torque_ctrl_.pid.Kd        = 0.0f;
    foc_rebound->rebound_angle_             = g_foc.state_.shaft_angle;
    foc_rebound->update_output_ratio          = update_output_ratio;
    update_output_ratio(1.0f /* output ratio */);

    g_foc_app.mode_ = FOC_App_Normal_Mode;
}