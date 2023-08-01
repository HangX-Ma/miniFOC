#include "easing.h"
#include "qfplib-m3.h"

#define QFP_QUAD(x)         qfp_fmul(x, x)
#define QFP_CUBIC(x)        qfp_fmul(QFP_QUAD(x), x)
#define QFP_QUART(x)        qfp_fmul(QFP_CUBIC(x), x)
#define QFP_QUINT(x)        qfp_fmul(QFP_QUART(x), x)


// Linear 匀速
float easing_Linear(float step) {
    return step;
}

// Quadratic 二次方的缓动 （t^2）
float easing_In_Quad(float step) {
    return QFP_QUAD(step);
}

float easing_Out_Quad(float step) {
    return qfp_fsub(1.0f, QFP_QUAD(qfp_fsub(1.0f, step)));
}

float easing_InOut_Quad(float step) {
    return (step < 0.5f)
        ? qfp_fmul(2.0f, QFP_QUAD(step))
        : qfp_fsub(1.0, qfp_fdiv(QFP_QUAD(qfp_fadd(qfp_fmul(-2.0f, step), 2.0f)), 2.0f));
}

float easing_OutIn_Quad(float step) {
    return (step < 0.5f)
        ? qfp_fmul(0.5f, qfp_fsub(1.0f, QFP_QUAD(qfp_fsub(1.0f, qfp_fmul(2.0f, step)))))
        : qfp_fadd(qfp_fmul(0.5f, QFP_QUAD(qfp_fsub(qfp_fmul(step, 2.0f), 1.0f))), 0.5f);
}

// Cubic三次方的缓动 （t^3）
float easing_In_Cubic(float step) {
    return QFP_CUBIC(step);
}

float easing_Out_Cubic(float step) {
    return qfp_fsub(1.0f, QFP_CUBIC(qfp_fsub(1.0f, step)));
}

float easing_InOut_Cubic(float step) {
    return (step < 0.5f)
        ? qfp_fmul(4.0f, QFP_CUBIC(step))
        : qfp_fsub(1.0f, qfp_fdiv(QFP_CUBIC(qfp_fadd(qfp_fmul(-2.0f, step), 2.0f)), 2.0f));
}

float easing_OutIn_Cubic(float step) {
    return (step < 0.5f)
        ? qfp_fmul(0.5f, qfp_fsub(1.0f, QFP_CUBIC(qfp_fsub(1.0f, qfp_fmul(2.0f, step)))))
        : qfp_fadd(qfp_fmul(0.5f, QFP_CUBIC(qfp_fsub(qfp_fmul(step, 2.0f), 1.0f))), 0.5f);
}

// Quartic四次方的缓动 （t^4）
float easing_In_Quart(float step) {
    return QFP_QUART(step);
}

float easing_Out_Quart(float step) {
    return qfp_fsub(1.0f, QFP_QUART(qfp_fsub(1.0f, step)));
}

float easing_InOut_Quart(float step) {
    return (step < 0.5f)
        ? qfp_fmul(8.0f, QFP_QUART(step))
        : qfp_fsub(1.0f, qfp_fdiv(QFP_QUART(qfp_fadd(qfp_fmul(-2.0f, step), 2.0f)), 2.0f));
}

float easing_OutIn_Quart(float step) {
    return (step < 0.5f)
        ? qfp_fmul(0.5f, qfp_fsub(1.0f, QFP_QUART(qfp_fsub(1.0f, qfp_fmul(2.0f, step)))))
        : qfp_fadd(qfp_fmul(0.5f, QFP_QUART(qfp_fsub(qfp_fmul(step, 2.0f), 1.0f))), 0.5f);
}

// Quintic五次方的缓动 (t^5)
float easing_In_Quint(float step) {
    return QFP_QUINT(step);
}

float easing_Out_Quint(float step) {
    return qfp_fsub(1.0f, QFP_QUINT(qfp_fsub(1.0f, step)));
}

float easing_InOut_Quint(float step) {
    return (step < 0.5f)
        ? qfp_fmul(16.0f, QFP_QUINT(step))
        : qfp_fsub(1.0f, qfp_fdiv(QFP_QUINT(qfp_fadd(qfp_fmul(-2.0f, step), 2.0f)), 2.0f));
}

float easing_OutIn_Quint(float step) {
    return (step < 0.5f)
        ? qfp_fmul(0.5f, qfp_fsub(1.0f, QFP_QUINT(qfp_fsub(1.0f, qfp_fmul(2.0f, step)))))
        : qfp_fadd(qfp_fmul(0.5f, QFP_QUINT(qfp_fsub(qfp_fmul(step, 2.0f), 1.0f))), 0.5f);
}

// Sinusoidal正弦曲线的缓动（sin(t)）
float easing_In_Sine(float step) {
    return qfp_fsub(1.0f, qfp_fcos(qfp_fdiv(qfp_fmul(step, _PI), 2.0f)));
}

float easing_Out_Sine(float step) {
    return qfp_fsin(qfp_fdiv(qfp_fmul(step, _PI), 2.0f));
}

float easing_InOut_Sine(float step) {
    return -qfp_fdiv(qfp_fsub(qfp_fcos(qfp_fmul(step, _PI)), 1.0f), 2.0f);
}

float easing_OutIn_Sine(float step) {
    return (step < 0.5f)
        ? qfp_fmul(0.5f, qfp_fsub(1.0f, easing_In_Sine(qfp_fsub(1.0f, qfp_fmul(2.0f, step)))))
        : qfp_fadd(qfp_fmul(0.5f, easing_In_Sine(qfp_fsub(qfp_fmul(step, 2.0f), 1.0f))), 0.5f);
}

// Exponential指数曲线的缓动（2^t）
float easing_In_Expo(float step) {
    return (step == 0.0f)
        ? 0.0f
        : qfp_fexp(qfp_fsub(qfp_fmul(10.0f, step), 10.0));
}

float easing_Out_Expo(float step) {
    return qfp_fsub(1.0f, qfp_fexp(qfp_fmul(-8.0f, step)));
}

float easing_InOut_Expo(float step) {
    return (step == 0.0f)
        ? 0.0f
        : (step == 1.0f)
        ? 1.0f
        : (step < 0.5f)
        ? qfp_fdiv(qfp_fexp(qfp_fsub(qfp_fmul(20.0f, step), 10.0f)), 2.0)
        : qfp_fdiv(qfp_fsub(2.0f, qfp_fexp(qfp_fadd(qfp_fmul(-20.0f, step), 10.0f))), 2.0f);
}

float easing_Out_InExpo(float step) {
    return (step < 0.5)
        ? qfp_fmul(0.5f, qfp_fsub(1.0f, easing_In_Expo(qfp_fsub(1.0f, qfp_fmul(2.0f, step)))))
        : qfp_fadd(qfp_fmul(0.5f, easing_In_Expo(qfp_fsub(qfp_fmul(2.0f, step), 1.0f))), 0.5f);
}

// Circular圆形曲线的缓动sqrt(1-t^2)
float easing_In_Circ(float step) {
    return qfp_fsub(1.0f, qfp_fsqrt(qfp_fsub(1.0, QFP_QUAD(step))));
}

float easing_Out_Circ(float step) {
    return qfp_fsqrt(qfp_fsub(1.0f, QFP_QUAD(qfp_fsub(step, 1.0f))));
}

float easing_InOut_Circ(float step) {
    return (step < 0.5f)
        ? qfp_fdiv(qfp_fsub(1.0f, qfp_fsqrt(qfp_fsub(1.0f, QFP_QUAD(qfp_fmul(2.0f, step))))), 2.0f)
        : qfp_fdiv(qfp_fadd(qfp_fsqrt(qfp_fsub(1.0, QFP_QUAD(qfp_fadd(qfp_fmul(-2.0, step), 2.0f)))), 1.0f), 2.0f);
}

float easing_OutIn_Circ(float step) {
    return (step < 0.5f)
        ? qfp_fmul(0.5f, qfp_fsub(1.0f, easing_In_Circ(qfp_fsub(1.0f, qfp_fmul(2.0f, step)))))
        : qfp_fadd(qfp_fmul(0.5f, easing_In_Circ(qfp_fsub(qfp_fmul(step, 2.0f), 1.0f))), 0.5f);
}

// Elastic指数衰减的正弦曲线缓动
float easing_In_Elastic(float step) {
    const float c4 = qfp_fdiv(qfp_fmul(2.0f, _PI), 3.0f);

    return (step == 0.0f)
        ? 0.0f
        : (step == 1.0f)
        ? 1.0f
        : qfp_fmul(-qfp_fexp(qfp_fsub(qfp_fmul(10.0f, step), 10.0f)), qfp_fsin(qfp_fmul(qfp_fsub(qfp_fmul(step, 10.0f), 10.75f), c4)));
}

float easing_Out_Elastic(float step) {
    const float c4 = qfp_fdiv(qfp_fmul(2.0f, _PI), 3.0f);

    return (step == 0.0f)
        ? 0.0f
        : (step == 1.0f)
        ? 1.0f
        : qfp_fadd(qfp_fmul(qfp_fexp(qfp_fmul(-10.0f, step)), qfp_fsin(qfp_fmul(qfp_fsub(qfp_fmul(step, 10.0f), 0.75f), c4))), 1.0f);
}

float easing_InOut_Elastic(float step) {
    const float c5 = qfp_fdiv(qfp_fmul(2.0f, _PI), 4.5f);

    return (step == 0.0f)
        ? 0.0f
        : (step == 1.0f)
        ? 1.0f
        : (step < 0.5f)
        ? qfp_fdiv(-qfp_fmul(qfp_fexp(qfp_fsub(qfp_fmul(20.0f, step), 10.0f)), qfp_fsin(qfp_fmul(qfp_fsub(qfp_fmul(20.0f, step), 11.125f), c5))), 2.0f)
        : qfp_fadd(qfp_fdiv(qfp_fmul(qfp_fexp(qfp_add(qfp_fmul(-20.0f, step), 10.0f)), qfp_fsin(qfp_fmul(qfp_fsub(qfp_fmul(20.0f, step), 11.125f), c5))), 2.0f), 1.0f);
}

float easing_OutIn_Elastic(float step) {
    return (step < 0.5f)
        ? qfp_fmul(0.5f, qfp_fsub(1.0f, easing_In_Elastic(qfp_fsub(1.0f, qfp_fmul(2.0f, step)))))
        : qfp_fadd(qfp_fmul(0.5f, easing_In_Elastic(qfp_fsub(qfp_fmul(step, 2.0f), 1.0f))), 0.5f);
}

// Back超过范围的三次方缓动（(s+1)*t^3 - s*t^2）
float easing_In_Back(float step) {
    const float c1 = 1.70158f;
    const float c3 = 2.70158f;

    return qfp_fsub(qfp_fmul(c3, QFP_CUBIC(step)), qfp_fmul(c1, QFP_QUAD(step)));
}

float easing_Out_Back(float step) {
    const float c1 = 1.70158f;
    const float c3 = 2.70158f;
    float step_sub_one = qfp_fsub(step, 1.0f);

    return qfp_add(1.0, qfp_fmul(step_sub_one, qfp_fmul(step, qfp_add(qfp_fmul(c3, step), c1))));
}

float easing_InOut_Back(float step) {
    const float c1 = 1.70158f;
    const float c2 = 3.22658f;
    const float c2_plus_one = 4.22658;

    return (step < 0.5f)
        ? qfp_fdiv(qfp_fmul(QFP_QUAD(qfp_fmul(2.0f, step)), qfp_fsub(qfp_fmul(c2_plus_one, qfp_fmul(2.0f, step)), c2)), 2.0f)
        : qfp_fdiv(
            qfp_add(
                qfp_fmul(
                    QFP_QUAD(qfp_fsub(qfp_fmul(2.0f, step), 2.0f)),
                    qfp_fadd(qfp_fmul(c2_plus_one, qfp_fsub(qfp_fmul(2.0f, step), 2.0f)), c2)
                ),
                2.0f
            ),
            2.0f
        );
}

float easing_OutIn_Back(float step) {
    return (step < 0.5f)
        ? qfp_fmul(0.5f, qfp_fsub(1.0f, easing_In_Back(qfp_fsub(1.0f, qfp_fmul(2.0f, step)))))
        : qfp_fadd(qfp_fmul(0.5f, easing_In_Back(qfp_fsub(qfp_fmul(step, 2.0f), 1.0f))), 0.5f);
}

// Bounce指数衰减的反弹缓动
float easing_In_Bounce(float step) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    float step_tmp = 0.0f;

    if (step < qfp_fdiv(1.0f, d1)) {
        return qfp_fmul(n1, QFP_QUAD(step));
    }
    if (step < qfp_fdiv(2.0f, d1)) {
        step_tmp = qfp_fsub(step, qfp_fdiv(1.5f, d1));
        return qfp_add(qfp_fmul(n1, QFP_QUAD(step_tmp)), 0.75f);
    }
    if (step < qfp_fdiv(2.5f, d1)) {
        step_tmp = qfp_fsub(step, qfp_fdiv(2.25f, d1));
        return qfp_add(qfp_fmul(n1, QFP_QUAD(step_tmp)), 0.9375f);
    }

    step_tmp = qfp_fsub(step, qfp_fdiv(2.625f, d1));
    return qfp_add(qfp_fmul(n1, QFP_QUAD(step_tmp)), 0.984375f);
}

float easing_Out_Bounce(float step) {
    return qfp_fsub(1.0f, easing_In_Bounce(qfp_fsub(1.0f, step)));
}

float easing_InOut_Bounce(float step) {
    return (step < 0.5f)
        ? qfp_fdiv(qfp_fsub(1.0f, easeOutBounce(qfp_fsub(1.0f, qfp_fmul(2.0f, step)))), 2.0f)
        : qfp_fdiv(qfp_add(1.0f, easeOutBounce(qfp_fsub(qfp_fmul(2.0f, step), 1.0f))), 2.0f);
}

float easing_OutIn_Bounce(float step) {
    return (step < 0.5f)
        ? qfp_fmul(0.5f, qfp_fsub(1.0f, easing_In_Bounce(qfp_fsub(1.0f, qfp_fmul(2.0f, step)))))
        : qfp_fadd(qfp_fmul(0.5f, easing_In_Bounce(qfp_fsub(qfp_fmul(step, 2.0f), 1.0f))), 0.5f);
}