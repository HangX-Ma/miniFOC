#include "foc_app.h"
#include "config.h"
#include "qfplib-m3.h"
#include "pid.h"
#include "foc.h"

// attractor angle variable
static float attract_angle = 0.0f;
// distance between attraction points
static float attractor_distance = 60.0f / 180.f * _PI; // dimp each 45 degrees

float find_attractor(float angle){
    uint32_t idx = (uint32_t)qfp_fadd(qfp_fdiv(angle, attractor_distance), 0.5f);
    return qfp_fmul((float)idx, attractor_distance);
}

void torque_ratchet_mode(void) {
    g_tor_ctrl.target_torque = attract_angle - g_foc.state_.shaft_angle;
    attract_angle = find_attractor(g_foc.state_.shaft_angle);
}