#include "foc_app.h"
#include "config.h"
#include "qfplib-m3.h"
#include "pid.h"
#include "foc.h"
#include "encoder.h"

// attractor angle variable
static float attract_angle = 0.0f;
// distance between attraction points
static float attractor_distance = 60.0f / 180.f * _PI; // dimp each 45 degrees
static float normalized_angle = 0.0f;

// normalize angle to range [0, 2PI]
static float normalize(float angle) {
    float _fmod = fmodf(angle, _2PI);
    return _fmod >= 0 ? _fmod : qfp_fadd(_fmod, _2PI);
}

float find_attractor(float angle){
    uint32_t idx = (uint32_t)qfp_fadd(qfp_fdiv(angle, attractor_distance), 0.5f);
    return qfp_fmul((float)idx, attractor_distance);
}

void torque_ratchet_mode(void) {
    normalized_angle = normalize(g_foc.state_.shaft_angle);
    g_tor_ctrl.target_torque = attract_angle - normalized_angle;
    attract_angle = find_attractor(normalized_angle);
}