#include "tween.h"
#include "qfplib-m3.h"

// const easing_t g_easing_array[11][4] = { // [TweenTransitionType][TweenEaseType]
//     {easing_Linear,     easing_Linear,      easing_Linear,        easing_Linear},
//     {easing_In_Quad,    easing_Out_Quad,    easing_InOut_Quad,    easing_OutIn_Quad,},
//     {easing_In_Cubic,   easing_Out_Cubic,   easing_InOut_Cubic,   easing_OutIn_Cubic,},
//     {easing_In_Quart,   easing_Out_Quart,   easing_InOut_Quart,   easing_OutIn_Quart,},
//     {easing_In_Quint,   easing_Out_Quint,   easing_InOut_Quint,   easing_OutIn_Quint,},
//     {easing_In_Sine,    easing_Out_Sine,    easing_InOut_Sine,    easing_OutIn_Sine,},
//     {easing_In_Expo,    easing_Out_Expo,    easing_InOut_Expo,    easing_Out_InExpo,},
//     {easing_In_Circ,    easing_Out_Circ,    easing_InOut_Circ,    easing_OutIn_Circ,},
//     {easing_In_Elastic, easing_Out_Elastic, easing_InOut_Elastic, easing_OutIn_Elastic,},
//     {easing_In_Back,    easing_Out_Back,    easing_InOut_Back,    easing_OutIn_Back,},
//     {easing_In_Bounce,  easing_Out_Bounce,  easing_InOut_Bounce,  easing_OutIn_Bounce,},
// };

TweenHandler g_tween_handler;

Tween tween_create(
    TweenMode mode,            // default: TWEEN_TIMES_SINGLE | TWEEN_DIR_FORWARD
    easing_t  easing_callback, // default: 0 (easing_Linear)
    float     offset,          // default: 0 (pixel offset)
    uint16_t  frame_num,       // default: 2 (start/stop fram)
    uint16_t  duration         // default: 0 (None)
)
{
    Tween tween = {
        .mode_         = mode,
        .start_        = 0.0f,
        .end_          = 0.0f,
        .delta_        = 0.0f,
        .current_      = 0.0f,
        .offset_       = offset,

        .frame_num_    = (frame_num < 2) ? 2 : frame_num,
        .frame_idx_    = 0,

        .easing        = (easing_callback == 0)
                            ? easing_Linear
                            : easing_callback,
        .step_         = 0.0f,

        .repeat_times_ = 0,
        .duration_     = duration,
        .dir_          = mode & TWEEN_DIR_BACKWARD,
    };

    return tween;
}

void tween_start_absolute(Tween* pTween, float from, float to) {
    pTween->start_     = from;
    pTween->end_       = to;
    pTween->delta_     = to - from;
    pTween->step_      = 0.0f;
    pTween->frame_idx_ = 0; // first frame at first
    pTween->dir_       = pTween->mode_ & TWEEN_DIR_BACKWARD;

    if (pTween->mode_ & TWEEN_TIMES_INFINITE) {
        pTween->repeat_times_ = -1;
    } else {
        pTween->repeat_times_ =
            (pTween->mode_ & TWEEN_TIMES_REPEAT) ? (pTween->mode_ >> TWEEN_MODE_BIT_CNT) : 1;
        if (pTween->mode_ & TWEEN_DIR_YOYO) {
            pTween->repeat_times_ *= 2;
        }
    }
}

void tween_start_relative(Tween* pTween, float dist) {
    tween_start_absolute(pTween, pTween->current_, qfp_fadd(pTween->end_, dist));
}

void tween_stop(Tween* pTween, float current) {
    pTween->repeat_times_ = 0;
    pTween->current_ = current;
}

void tween_update(Tween* pTween) {
    // finish
    if (pTween->repeat_times_ == 0) {
        return;
    }

    // next frame
    pTween->frame_idx_ += 1;
    // check index overflow
    if (pTween->frame_idx_ > pTween->frame_num_) {
        if (pTween->mode_ & TWEEN_DIR_YOYO) {
            // reverse direction
            pTween->dir_ = !pTween->dir_;
            // skip start frame or stop frame
            pTween->frame_idx_ = 2;
        } else {
            pTween->frame_idx_ = 1;
        }
    }
    // check last frame
    if (pTween->frame_idx_ == pTween->frame_num_) {
        // at last frame
        pTween->step_ = 1.0f;
        pTween->current_ = pTween->dir_ ? pTween->start_ : pTween->end_;
        // decrease repeated times
        if (!(pTween->mode_ & TWEEN_TIMES_INFINITE)) {
            pTween->repeat_times_ -= 1;
            if (pTween->repeat_times_) {
                return;
            }
        }
    } else {
        // calculate the step
        pTween->step_ = qfp_fdiv((float)(pTween->frame_idx_ - 1), (float)(pTween->frame_num_ - 1));
        // calculate the pixel position
        pTween->current_ = pTween->dir_
                            ? qfp_fsub(pTween->end_, qfp_fmul(pTween->delta_, pTween->easing(pTween->step_)))
                            : qfp_fadd(pTween->start_, qfp_fmul(pTween->delta_, pTween->easing(pTween->step_)));
    }
}

BOOL tween_finished(Tween* pTween) {
    return pTween->repeat_times_ == 0;
}

float tween_pixel_position(Tween* pTween) {
    return qfp_fadd(pTween->current_, pTween->offset_);
}

void tween_handler_init(void) {
    g_tween_handler.create         = tween_create;
    g_tween_handler.start_absolute = tween_start_absolute;
    g_tween_handler.start_relative = tween_start_relative;
    g_tween_handler.stop           = tween_stop;
    g_tween_handler.update         = tween_update;
    g_tween_handler.finished       = tween_finished;
    g_tween_handler.curr_pixel_pos = tween_pixel_position;
}