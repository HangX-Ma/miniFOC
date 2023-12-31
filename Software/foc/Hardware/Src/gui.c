#include "gui.h"
#include "config.h"
#include "qfplib-m3.h"
#include "foc.h"
#include "foc_app.h"
#include "pid.h"
#include <math.h>

#include "stm32f1xx_ll_utils.h"

#define X_PADDING           (64)
#define X_VAL_PADDING       (34)

page_t* pages[PAGE_ID_LAST] = {0};
volatile PageID page_selected;  // index of rendering

#define PAGE_REGISTER(ID, page) pages[ID] = (page_t*)&page

/* ----------------- EFFECT ------------- */
void effect_win10_loading() {
    uint8_t r   = 16;
    uint8_t cnt = 6;  // number of points

    Tween tween = g_tween_handler.create(
            TWEEN_MODE_REPEAT_NTIMES(2) | TWEEN_DIR_FORWARD,
            easing_Out_Quart,
            0.0f,
            15,
            30
        );

    g_tween_handler.start_absolute(&tween, 0.0f, 1.0f);  // generate linear time order

    while (!g_tween_handler.finished(&tween)) {
        g_tween_handler.update(&tween);
        g_gui_base.clear();
        for (uint8_t i = 0; i < cnt; ++i) {
            float t = qfp_fsub(tween.current_, qfp_fmul((float)i, 0.05f));
            if (t < 0) {
                t = qfp_fadd(t, 1.0f);
            }
            // angle (rad, 3.14rad -> 180°)
            float rad = qfp_fadd(qfp_fmul(-easing_InOut_Cubic(t), _2PI), _PI);
            g_gui_base.draw_circle_full_center(
                    qfp_fmul((float)r, qfp_fsin(rad)),
                    qfp_fmul((float)r, qfp_fcos(rad)),
                    1
                );
        }
        g_gui_base.update();
    }
}

void effect_rotating_rect() {
    Tween tween = g_tween_handler.create(
            TWEEN_MODE_REPEAT_NTIMES(2) | TWEEN_DIR_YOYO,
            easing_Out_Quart,
            0,
            15,
            120
        );

    g_tween_handler.start_absolute(&tween, 0, 1);  // generate linear time order

    float w, h;
    while (!g_tween_handler.finished(&tween)) {
        g_tween_handler.update(&tween);
        g_gui_base.clear();

        w = h = qfp_fmul(g_tween_handler.curr_pixel_pos(&tween), 10.0f);

        g_gui_base.draw_rotated_rect(64, 32, w, h, qfp_fmul(g_tween_handler.curr_pixel_pos(&tween), _PI_2));
        g_gui_base.draw_rotated_rect(64, 32, w, h, qfp_fmul(qfp_fmul(g_tween_handler.curr_pixel_pos(&tween), _PI_2), 1.5f));
        g_gui_base.update();
    }
    g_gui_base.set_color(1);
}

void effect_center_string(MenuList *pMenuList, const char* str, int16_t x_offset) {
    // range check
    x_offset = x_offset < -56 ? -56 : x_offset > 72 ? 72 : x_offset;

    // Blur the background
    for (uint16_t i = 0; i < g_u8g2_buf_len; i++)
        g_u8g2_buf_ptr[i] &= (i % 2 == 0 ? 0x55 : 0xAA);

    g_gui_base.update();

    g_gui_base.set_color(2);
    g_gui_base.draw_fill_rect(16, 16, 96, 31);

    g_gui_base.set_color(1);
    g_gui_base.draw_rect(16, 16, 96, 31);
    g_gui_base.set_color(2);
    g_gui_base.draw_rect(17, 17, 94, 29);
    g_gui_base.set_color(1);

    g_gui_base.draw_str(56 + x_offset, 32, str);
    g_gui_base.update();

    while (vkey_scan() == VKEY_ID_NONE) {}

    pMenuList->repaint_ = TRUE;
}

/* ---------------- LOGO ---------------- */
// normalize angle to range [0, 2PI]
static float normalize(float angle) {
    float _fmod = fmodf(angle, _2PI);
    return _fmod >= 0 ? _fmod : qfp_fadd(_fmod, _2PI);
}

void gui_painter_logo(page_t* pg) {
    (void)pg;

    float r = 18.0f;
    float angle_prev = 0.0f, angle = 0.0f;
    float target_torque;

    while (vkey_scan() == VKEY_ID_NONE) {
        g_gui_base.clear();

        g_gui_base.draw_str(2, 12, " Motor Info ");
        // Motor Info mask
        g_gui_base.set_color(2);
        g_gui_base.draw_round_rect(2, 1, 56, 14, 1);

        g_gui_base.draw_str(X_PADDING, 12, "Mode:");
        g_gui_base.draw_str(X_PADDING, 24, "State:");
        g_gui_base.draw_str(X_PADDING, 36, "Target:");
        g_gui_base.draw_str(X_PADDING, 48, "Vel:");
        g_gui_base.draw_str(X_PADDING, 60, "Ang:");

        // show state
        g_gui_base.draw_str(X_PADDING+ X_VAL_PADDING, 24, g_foc.state_.power_on ? "ON " : "OFF");
        // velocity
        g_gui_base.draw_num(X_PADDING + X_VAL_PADDING, 48, g_foc.state_.shaft_speed);

        switch (g_foc.motion_type_) {
            case FOC_Motion_Type_Torque:
                g_gui_base.draw_str(X_PADDING + X_VAL_PADDING, 12, "TOR-");
                switch (g_foc_app.mode_) {
                    case FOC_App_Ratchet_Mode:
                        target_torque = g_foc_app.ratchet_.torque_ctrl_.target_torque;
                        g_gui_base.draw_str(X_PADDING + X_VAL_PADDING + 22, 12, "T");
                        break;
                    case FOC_App_Rebound_Mode:
                        target_torque = g_foc_app.rebound_.torque_ctrl_.target_torque;
                        g_gui_base.draw_str(X_PADDING + X_VAL_PADDING + 22, 12, "R");
                        break;
                    case FOC_App_Damp_Mode:
                        target_torque = g_foc_app.damp_.torque_ctrl_.target_torque;
                        g_gui_base.draw_str(X_PADDING + X_VAL_PADDING + 22, 12, "D");
                        break;
                    case FOC_App_Normal_Mode:
                    default:
                        target_torque = g_foc_app.normal_.torque_ctrl_.target_torque;
                        g_gui_base.draw_str(X_PADDING + X_VAL_PADDING + 22, 12, "N");
                        break;
                }
                g_gui_base.draw_num(X_PADDING + X_VAL_PADDING, 36, target_torque);
                // normalized angle
                g_gui_base.draw_num(X_PADDING + X_VAL_PADDING, 60, normalize(g_foc.state_.shaft_angle));
                break;
            case FOC_Motion_Type_Velocity:
                g_gui_base.draw_str(X_PADDING + X_VAL_PADDING, 12, "VEL");
                g_gui_base.draw_num(X_PADDING + X_VAL_PADDING, 36, g_vel_ctrl.target_speed);
                // normalized angle
                g_gui_base.draw_num(X_PADDING + X_VAL_PADDING, 60, normalize(g_foc.state_.shaft_angle));
                break;
            case FOC_Motion_Type_Angle:
                g_gui_base.draw_str(X_PADDING + X_VAL_PADDING, 12, "ANG");
                g_gui_base.draw_num(X_PADDING + X_VAL_PADDING, 36, g_ang_ctrl.target_angle);
                // shaft angle
                g_gui_base.draw_num(X_PADDING + X_VAL_PADDING, 60, g_foc.state_.shaft_angle);
                break;
            default:
                g_gui_base.draw_str(X_PADDING + X_VAL_PADDING, 12, "NAN");
                break;
        }

        // draw central circle
        g_gui_base.draw_disc_full(32, 38, 10);
        g_gui_base.update();

        // draw the outer ball location according to the current shaft angle
        if (g_foc.state_.power_on) {
            angle = normalize(g_foc.state_.shaft_angle);
            angle_prev = angle;
        } else {
            // keep the outer ball stopped at previous position
            angle = angle_prev;
        }
        g_gui_base.draw_disc_full(
            32 + qfp_fmul(r, qfp_fcos(angle)),
            38 + qfp_fmul(r, qfp_fsin(angle)),
            3
        );
        g_gui_base.update();
    }
    g_gui_base.set_color(1);
    g_gui_base.effect_disappear();
    gui_switch(PAGE_ID_MAIN_MENU_LIST);
}

void gui_handler_logo(page_t* pg) {
    (void)pg;
    switch (vkey_scan()) {
        case VKEY_ID_NONE:
            break;
        default:
            g_gui_base.effect_disappear();
            gui_switch(PAGE_ID_MAIN_MENU_LIST);
            break;
    }
}


/* ---------------- ABOUT ---------------- */
static void gui_painter_about(page_t* pg) {
    (void)pg;
    g_gui_base.clear();
    g_gui_base.set_color(1);
    g_gui_base.draw_str(2, 12, "MCU : STM32F103C8T6");
    g_gui_base.draw_str(2, 28, "FLASH : 64 KB");
    g_gui_base.draw_str(2, 44, "SRAM : 20 KB");
    g_gui_base.update();
}

static void gui_handler_about(page_t* pg) {
    (void)pg;
    switch (vkey_scan()) {
        case VKEY_ID_NONE:
            break;
        default:
            g_gui_base.effect_disappear();
            gui_switch(PAGE_ID_MAIN_MENU_LIST);
            break;
    }
}

/* ---------------- Main ---------------- */
const menu_item_t menu_main[] = {
    {PAGE_ID_LOGO,                      "MainUI",               0},
    {PAGE_ID_MOTOR_MODE_MENU_LIST,      "- Switch Motor Mode",  0},
    {PAGE_ID_TOR_EDITOR_MENU_LIST,      "- Torque Editor",      0},
    {PAGE_ID_VEL_EDITOR_MENU_LIST,      "- Velocity Editor",    0},
    {PAGE_ID_ANG_EDITOR_MENU_LIST,      "- Angle Editor",       0},
    {PAGE_ID_EASING_CHART,              "- Easing Chart",       0},
    {PAGE_ID_ABOUT,                     "- About",              0},
};

void gui_handler_main(MenuList* pMenuList)
{
    switch (vkey_scan()) {
        case VKEY_ID_NONE:
            break;
        case VKEY_ID_PREV:
            menu_list_callback_handler_switch_to_prev(pMenuList);
            break;
        case VKEY_ID_NEXT:
            menu_list_callback_handler_switch_to_next(pMenuList);
            break;
        case VKEY_ID_OK:

            switch (pMenuList->items_[pMenuList->selected_index_].id) {
                case PAGE_ID_LOGO:
                case PAGE_ID_EASING_CHART:
                case PAGE_ID_ABOUT: {
                    g_gui_base.effect_disappear();
                    gui_switch(pMenuList->items_[pMenuList->selected_index_].id);
                    break;
                }
                case PAGE_ID_MOTOR_MODE_MENU_LIST:
                case PAGE_ID_TOR_EDITOR_MENU_LIST:
                case PAGE_ID_VEL_EDITOR_MENU_LIST:
                case PAGE_ID_ANG_EDITOR_MENU_LIST: {
                    // effect_win10_loading();
                    // effect_rotating_rect();
                    gui_switch(pMenuList->items_[pMenuList->selected_index_].id);
                    break;
                }
                default:
                    break;
            }

            pMenuList->repaint_ = TRUE;
            break;
        default:
            break;
    }
}


/* ---------------- MOTOR APP ---------------- */
enum {
    FUNC_ID_APP_NORMAL_MODE = FUNC_ID_RETURN + 1,
    FUNC_ID_APP_RATCHET_MODE,
    FUNC_ID_APP_REBOUND_MODE,
    FUNC_ID_APP_DAMP_MODE,
};


const menu_item_t menu_motor_app[] = {
    {FUNC_ID_APP_NORMAL_MODE,  "- Normal Mode",  0},
    {FUNC_ID_APP_RATCHET_MODE, "- Ratchet Mode", 0},
    {FUNC_ID_APP_REBOUND_MODE, "- Rebound Mode", 0},
    {FUNC_ID_APP_DAMP_MODE,    "- Damp Mode",    0},
    {FUNC_ID_RETURN,           "Return",         0},
};

void gui_handler_motor_app(MenuList* pMenuList) {
    switch (vkey_scan()) {
        case VKEY_ID_NONE:
            break;
        case VKEY_ID_PREV:
            menu_list_callback_handler_switch_to_prev(pMenuList);
            break;
        case VKEY_ID_NEXT:
            menu_list_callback_handler_switch_to_next(pMenuList);
            break;
        case VKEY_ID_OK:
            switch (pMenuList->items_[pMenuList->selected_index_].id) {
                case FUNC_ID_APP_RATCHET_MODE:
                    g_foc_app.mode_ = FOC_App_Ratchet_Mode;
                    effect_center_string(pMenuList, "OK!", 0);
                    break;
                case FUNC_ID_APP_REBOUND_MODE:
                    g_foc_app.mode_ = FOC_App_Rebound_Mode;
                    effect_center_string(pMenuList, "OK!", 0);
                    break;
                case FUNC_ID_APP_DAMP_MODE:
                    g_foc_app.mode_ = FOC_App_Damp_Mode;
                    effect_center_string(pMenuList, "OK!", 0);
                    break;
                case FUNC_ID_APP_NORMAL_MODE:
                    g_foc_app.mode_ = FOC_App_Normal_Mode;
                    effect_center_string(pMenuList, "OK!", 0);
                    break;
                case FUNC_ID_RETURN: {
                    g_gui_base.effect_disappear();
                    gui_switch(PAGE_ID_MAIN_MENU_LIST);
                    break;
                }
            }
            break;
        default:
            break;
    }
}


/* ---------------- MOTOR MODE ---------------- */
enum {
    FUNC_ID_MOTOR_TORQUE_MODE = FUNC_ID_RETURN + 1,
    FUNC_ID_MOTOR_VELOCITY_MODE,
    FUNC_ID_MOTOR_ANGLE_MODE,
};

const menu_item_t menu_motor_mode[] = {
    {FUNC_ID_MOTOR_TORQUE_MODE,     "- Torque Mode",   0},
    {FUNC_ID_MOTOR_VELOCITY_MODE,   "- Velocity Mode", 0},
    {FUNC_ID_MOTOR_ANGLE_MODE,      "- Angle Mode",    0},
    {FUNC_ID_RETURN,                "Return",          0},
};

void gui_handler_motor_mode(MenuList* pMenuList) {
    switch (vkey_scan()) {
        case VKEY_ID_NONE:
            break;
        case VKEY_ID_PREV:
            menu_list_callback_handler_switch_to_prev(pMenuList);
            break;
        case VKEY_ID_NEXT:
            menu_list_callback_handler_switch_to_next(pMenuList);
            break;
        case VKEY_ID_OK:

            switch (pMenuList->items_[pMenuList->selected_index_].id) {
                case FUNC_ID_MOTOR_TORQUE_MODE:
                    g_foc.motion_type_ = FOC_Motion_Type_Torque;
                    gui_switch(PAGE_ID_MOTOR_APP_MENU_LIST);
                    break;
                case FUNC_ID_MOTOR_VELOCITY_MODE:
                    g_foc.motion_type_ = FOC_Motion_Type_Velocity;
                    effect_center_string(pMenuList, "OK!", 0);
                    break;
                case FUNC_ID_MOTOR_ANGLE_MODE:
                    g_foc.motion_type_ = FOC_Motion_Type_Angle;
                    effect_center_string(pMenuList, "OK!", 0);
                    break;
                case FUNC_ID_RETURN: {
                    g_gui_base.effect_disappear();
                    gui_switch(PAGE_ID_MAIN_MENU_LIST);
                    break;
                }
            }
        default:
            break;
    }
}

/* ---------------- PID BASE ---------------- */
enum {
    FUNC_ID_EDIT_KP = FUNC_ID_RETURN + 1,
    FUNC_ID_EDIT_KI,
    FUNC_ID_EDIT_KD,
    FUNC_ID_TORQUE_TARGET,
    FUNC_ID_VELOCITY_TARGET,
    FUNC_ID_ANGLE_TARGET,
    // about mode
    FUNC_ID_RATCHET_ATTRACTOR_NUM,
    FUNC_ID_REBOUND_OUTPUT_RATIO,
    FUNC_ID_REBOUND_INIT_ANGLE,
};


static SliderBase Torque_tar                   = {"Editing Torque", 0.0f, 3.0f, 0.0f, 0.1f};
static SliderBase Torque_Kp                    = {"Editing Kp", 0.0f, 1.2f, 0.0f, 0.02f};
static SliderBase Torque_Kd                    = {"Editing Kd", 0.0f, 0.5f, 0.0f, 0.01f};
static SliderBase Torque_ratchet_attractor_num = {"Ratchet Number", 2.0f, 18.0f, 6.0f, 2.0f};
static SliderBase Torque_rebound_angle         = {"Rebound Angle", -3.14, 3.14, 0.0f, 0.02f};
static SliderBase Torque_rebound_output_ratio  = {"Rebound Output Ratio", 1.0f, 20.0f, 1.0f, 1.0f};

static SliderBase Velocity_tar                 = {"Editing Velocity", -50.0f, 50.0f, 0.0f, 5.0f};
static SliderBase Velocity_Kp                  = {"Editing Kp", 0.0f, 0.5f, 0.05f, 0.01f};
static SliderBase Velocity_Ki                  = {"Editing Ki", 0.0f, 2.0f, 1.0f, 0.1f};

static SliderBase Angle_tar                    = {"Editing Angle", -50.0f, 50.0f, 0.0f, 5.0f};
static SliderBase Angle_Kp                     = {"Editing Kp", 0.0f, 8.0f, 4.0f, 0.5f};

const menu_item_t menu_tor_editor[] = {
    {FUNC_ID_TORQUE_TARGET,         "- Torque Target",      &Torque_tar},
    {FUNC_ID_EDIT_KP,               "- Proportional",       &Torque_Kp},
    {FUNC_ID_EDIT_KD,               "- Derivative",         &Torque_Kd},
    {FUNC_ID_RATCHET_ATTRACTOR_NUM, "- Attractor Number",   &Torque_ratchet_attractor_num},
    {FUNC_ID_REBOUND_INIT_ANGLE,    "- Rebound Angle",      &Torque_rebound_angle},
    {FUNC_ID_REBOUND_OUTPUT_RATIO,  "- Output Ratio",         &Torque_rebound_output_ratio},
    {FUNC_ID_RETURN,                "Return",               0},
};

const menu_item_t menu_vel_editor[] = {
    {FUNC_ID_VELOCITY_TARGET,       "- Velocity Target",    &Velocity_tar},
    {FUNC_ID_EDIT_KP,               "- Proportional",       &Velocity_Kp},
    {FUNC_ID_EDIT_KI,               "- Integral",           &Velocity_Ki},
    {FUNC_ID_RETURN,                "Return",               0},
};

const menu_item_t menu_ang_editor[] = {
    {FUNC_ID_ANGLE_TARGET,          "- Angle Target",       &Angle_tar},
    {FUNC_ID_EDIT_KP,               "- Proportional",       &Angle_Kp},
    {FUNC_ID_RETURN,                "Return",               0},
};

void gui_handler_editor(MenuList* pMenuList)
{
    switch (vkey_scan()) {
        case VKEY_ID_NONE:
            break;
        case VKEY_ID_PREV:
            menu_list_callback_handler_switch_to_prev(pMenuList);
            break;
        case VKEY_ID_NEXT:
            menu_list_callback_handler_switch_to_next(pMenuList);
            break;
        case VKEY_ID_OK:
            switch (pMenuList->items_[pMenuList->selected_index_].id) {
                case FUNC_ID_RATCHET_ATTRACTOR_NUM:
                    if (g_foc_app.mode_ != FOC_App_Ratchet_Mode) {
                        effect_center_string(pMenuList, "Forbidden!", -18);
                        break;
                    }
                    goto editor;
                case FUNC_ID_REBOUND_INIT_ANGLE:
                case FUNC_ID_REBOUND_OUTPUT_RATIO:
                    if (g_foc_app.mode_ != FOC_App_Rebound_Mode) {
                        effect_center_string(pMenuList, "Forbidden!", -18);
                        break;
                    }
                    goto editor;
                case FUNC_ID_TORQUE_TARGET: // torque target only takes effect under torque normal mode
                    if (!(g_foc_app.mode_ == FOC_App_Normal_Mode || g_foc_app.mode_ == FOC_App_Damp_Mode)) {
                        effect_center_string(pMenuList, "Forbidden!", -18);
                        break;
                    }
                    goto editor;
                case FUNC_ID_EDIT_KP:
                    if (g_foc_app.mode_ == FOC_App_Damp_Mode) {
                        effect_center_string(pMenuList, "Forbidden!", -18);
                        break;
                    }
                    /* fall through */
editor:
                case FUNC_ID_EDIT_KI:
                case FUNC_ID_EDIT_KD:
                case FUNC_ID_VELOCITY_TARGET:
                case FUNC_ID_ANGLE_TARGET:
                {
                    BOOL repaint = TRUE;
                    // Blur the background
                    for (uint16_t i = 0; i < g_u8g2_buf_len; i++)
                        g_u8g2_buf_ptr[i] &= (i % 2 == 0 ? 0x55 : 0xAA);

                    g_gui_base.update();
                    SliderBase* slider;
                    while (1) {
                        switch (vkey_scan()) {
                            case VKEY_ID_NONE:
                                break;
                            case VKEY_ID_PREV:
                                slider = (SliderBase*)(pMenuList->items_[pMenuList->selected_index_].arg);
                                if (slider->curr > qfp_fadd(slider->min, slider->step)) {
                                    slider->curr = qfp_fsub(slider->curr, slider->step);
                                } else {
                                    slider->curr = slider->min;
                                }
                                repaint = TRUE;
                                break;
                            case VKEY_ID_NEXT:
                                slider = (SliderBase*)(pMenuList->items_[pMenuList->selected_index_].arg);
                                if (slider->curr < qfp_fsub(slider->max, slider->step)) {
                                    slider->curr = qfp_fadd(slider->curr, slider->step);
                                } else {
                                    slider->curr = slider->max;
                                }
                                repaint = TRUE;
                                break;
                            case VKEY_ID_OK:
                                pMenuList->repaint_ = TRUE;
                                // update torque parameters
                                switch (g_foc_app.mode_) {
                                case FOC_App_Ratchet_Mode:
                                    g_foc_app.ratchet_.torque_ctrl_.pid.Kp = Torque_Kp.curr;
                                    g_foc_app.ratchet_.torque_ctrl_.pid.Kd = Torque_Kd.curr;
                                    g_foc_app.ratchet_.attractor_num_      = Torque_ratchet_attractor_num.curr;
                                    break;
                                case FOC_App_Rebound_Mode:
                                    g_foc_app.rebound_.torque_ctrl_.pid.Kp = Torque_Kp.curr;
                                    g_foc_app.rebound_.torque_ctrl_.pid.Kd = Torque_Kd.curr;
                                    g_foc_app.rebound_.rebound_angle_      = Torque_rebound_angle.curr;
                                    g_foc_app.rebound_.update_output_ratio(Torque_rebound_output_ratio.curr);
                                    break;
                                case FOC_App_Damp_Mode:
                                    // damp mode has no Kp config
                                    g_foc_app.damp_.torque_ctrl_.target_torque = Torque_tar.curr;
                                    g_foc_app.damp_.torque_ctrl_.pid.Kd = Torque_Kd.curr;
                                    break;
                                case FOC_App_Normal_Mode:
                                    /* fall through */
                                default:
                                    g_foc_app.normal_.torque_ctrl_.target_torque = Torque_tar.curr;
                                    g_foc_app.normal_.torque_ctrl_.pid.Kp        = Torque_Kp.curr;
                                    g_foc_app.normal_.torque_ctrl_.pid.Kd        = Torque_Kd.curr;
                                    break;
                                }
                                // update velocity parameters
                                g_vel_ctrl.target_speed = Velocity_tar.curr;
                                g_vel_ctrl.pid.Kp       = Velocity_Kp.curr;
                                g_vel_ctrl.pid.Ki       = Velocity_Ki.curr;
                                // update angle parameters

                                g_ang_ctrl.target_angle = Angle_tar.curr;
                                g_ang_ctrl.pid.Kp       = Angle_Kp.curr;
                                return;
                            default:
                                break;
                        }

                        if (repaint) {
                            repaint = FALSE;

                            g_gui_base.set_color(2);
                            g_gui_base.draw_fill_rect(16, 16, 96, 31);

                            g_gui_base.set_color(1);
                            g_gui_base.draw_rect(16, 16, 96, 31);
                            g_gui_base.set_color(2);
                            g_gui_base.draw_rect(17, 17, 94, 29);
                            g_gui_base.set_color(1);

                            g_gui_base.draw_fill_rect(18, 36, 60, 8);

                            slider = (SliderBase*)(pMenuList->items_[pMenuList->selected_index_].arg);

                            g_gui_base.draw_rect(
                                20, // x
                                38, // y
                                qfp_fmul(
                                    qfp_fdiv(
                                        qfp_fsub(slider->curr, slider->min),
                                        qfp_fsub(slider->max, slider->min)
                                    ),
                                    56.0f
                                ),  // w
                                4   // h
                            );
                            g_gui_base.draw_str(22, 30, slider->description);
                            g_gui_base.draw_num(81, 44, slider->curr);

                            g_gui_base.update();
                        }
                    }
                    break;
                }
                case FUNC_ID_RETURN: {
                    g_gui_base.effect_disappear();
                    gui_switch(PAGE_ID_MAIN_MENU_LIST);
                    break;
                }
                default:
                    break;
            }
        default:
            break;
    }
}

/* ---------------- EASING FUNC ---------------- */
enum {
    FUNC_ID_EASING = FUNC_ID_RETURN + 1,
};

float easing_Customized(const float t) {
    if (t < 0.3f) {
        return t;
    }
    if (t < 0.6f) {
        return qfp_fmul(t, t);
    }
    return t;
}

const menu_item_t menu_easing[] = {
    {FUNC_ID_EASING, "- Custom",        easing_Customized},
    {FUNC_ID_EASING, "- Linear  ",      easing_Linear},
    {FUNC_ID_EASING, "- InQuad  ",      easing_In_Quad},
    {FUNC_ID_EASING, "- OutQuad",       easing_Out_Quad},
    {FUNC_ID_EASING, "- InOutQuad",     easing_InOut_Quad},
    {FUNC_ID_EASING, "- OutInQuad",     easing_OutIn_Quad},
    {FUNC_ID_EASING, "- InCubic  ",     easing_In_Cubic},
    {FUNC_ID_EASING, "- OutCubic",      easing_Out_Cubic},
    {FUNC_ID_EASING, "- InOutCubic",    easing_InOut_Cubic},
    {FUNC_ID_EASING, "- OutInCubic",    easing_OutIn_Cubic},
    {FUNC_ID_EASING, "- InQuart  ",     easing_In_Quart},
    {FUNC_ID_EASING, "- OutQuart",      easing_Out_Quart},
    {FUNC_ID_EASING, "- InOutQuart",    easing_InOut_Quart},
    {FUNC_ID_EASING, "- OutInQuart",    easing_OutIn_Quart},
    {FUNC_ID_EASING, "- InQuint  ",     easing_In_Quint},
    {FUNC_ID_EASING, "- OutQuint",      easing_Out_Quint},
    {FUNC_ID_EASING, "- InOutQuint",    easing_InOut_Quint},
    {FUNC_ID_EASING, "- OutInQuint",    easing_OutIn_Quint},
    {FUNC_ID_EASING, "- InSine  ",      easing_In_Sine},
    {FUNC_ID_EASING, "- OutSine",       easing_Out_Sine},
    {FUNC_ID_EASING, "- InOutSine",     easing_InOut_Sine},
    {FUNC_ID_EASING, "- OutInSine",     easing_OutIn_Sine},
    {FUNC_ID_EASING, "- InExpo  ",      easing_In_Expo},
    {FUNC_ID_EASING, "- OutExpo",       easing_Out_Expo},
    {FUNC_ID_EASING, "- InOutExpo",     easing_InOut_Expo},
    {FUNC_ID_EASING, "- OutInExpo",     easing_Out_InExpo},
    {FUNC_ID_EASING, "- InCirc  ",      easing_In_Circ},
    {FUNC_ID_EASING, "- OutCirc",       easing_Out_Circ},
    {FUNC_ID_EASING, "- InOutCirc",     easing_InOut_Circ},
    {FUNC_ID_EASING, "- OutInCirc",     easing_OutIn_Circ},
    {FUNC_ID_EASING, "- InBack  ",      easing_In_Back},
    {FUNC_ID_EASING, "- OutBack",       easing_Out_Back},
    {FUNC_ID_EASING, "- InOutBack",     easing_InOut_Back},
    {FUNC_ID_EASING, "- OutInBack",     easing_OutIn_Back},
    {FUNC_ID_EASING, "- InElastic  ",   easing_In_Elastic},
    {FUNC_ID_EASING, "- OutElastic",    easing_Out_Elastic},
    {FUNC_ID_EASING, "- InOutElastic",  easing_InOut_Elastic},
    {FUNC_ID_EASING, "- OutInElastic",  easing_OutIn_Elastic},
    {FUNC_ID_EASING, "- InBounce  ",    easing_In_Bounce},
    {FUNC_ID_EASING, "- OutBounce",     easing_Out_Bounce},
    {FUNC_ID_EASING, "- InOutBounce",   easing_InOut_Bounce},
    {FUNC_ID_EASING, "- OutInBounce",   easing_OutIn_Bounce},
    {FUNC_ID_RETURN, "Return", 0},
};

void gui_handler_easing_chart(MenuList* pMenuList) {
    switch (vkey_scan()) {
        case VKEY_ID_NONE:
            break;
        case VKEY_ID_PREV:
            menu_list_callback_handler_switch_to_prev(pMenuList);
            break;
        case VKEY_ID_NEXT:
            menu_list_callback_handler_switch_to_next(pMenuList);
            break;
        case VKEY_ID_OK: {
            switch (pMenuList->items_[pMenuList->selected_index_].id) {
                case FUNC_ID_EASING: {
                    Tween tween = g_tween_handler.create(
                            TWEEN_MODE_DEFAULT,
                            pMenuList->items_[pMenuList->selected_index_].arg,
                            0, 36, 0
                        );
                    g_tween_handler.start_relative(&tween, tween.frame_num_);

                    float x_offset = qfp_fdiv(qfp_fsub((float)OLED_WIDTH, tween.end_), 2.0f);
                    float y_offset = qfp_fadd(qfp_fdiv(qfp_fsub((float)OLED_HEIGHT, tween.end_), 2.0f), tween.end_);

                    g_gui_base.clear();

                    float x1 = -1.0f, y1 = -1.0f, x2, y2;
                    while (!g_tween_handler.finished(&tween)) {
                        g_tween_handler.update(&tween);
                        x2 = qfp_fadd(x_offset, (float)tween.frame_idx_);
                        y2 = qfp_fsub(y_offset, g_tween_handler.curr_pixel_pos(&tween));
                        if (x1 == -1.0f) {
                            x1 = x2;
                        }
                        if (y1 == -1.0f) {
                            y1 = y2;
                        }
                        g_gui_base.draw_line(x1, y1, x2, y2);
                        x1 = x2;
                        y1 = y2;
                    }
                    g_gui_base.update();

                    while (vkey_scan() == VKEY_ID_NONE) {}

                    // repaint
                    pMenuList->repaint_ = TRUE;

                    break;
                }
                case FUNC_ID_RETURN: {
                    g_gui_base.effect_disappear();
                    gui_switch(PAGE_ID_MAIN_MENU_LIST);
                    break;
                }
            }
            break;
        }
        default:
            break;
    }
}


static page_t page_logo;
static page_t page_about;
static MenuList menu_list_main;
static MenuList menu_list_motor_app;
static MenuList menu_list_motor_mode;
static MenuList menu_list_tor_editor;
static MenuList menu_list_vel_editor;
static MenuList menu_list_ang_editor;
static MenuList menu_list_easing;

void gui_init(void) {
    //! init process needs in order!
    vkey_init();
    gui_base_init();
    tween_handler_init();

    page_logo  = gui_page_new(gui_painter_logo, gui_handler_logo);
    page_about = gui_page_new(gui_painter_about, gui_handler_about);

    PAGE_REGISTER(PAGE_ID_LOGO, page_logo);
    PAGE_REGISTER(PAGE_ID_ABOUT, page_about);

    menu_list_main       = menu_list_init(menu_main, ARRAY_SIZE(menu_main), 4, 0, gui_handler_main);
    menu_list_motor_app  = menu_list_init(menu_motor_app, ARRAY_SIZE(menu_motor_app), 4, 0, gui_handler_motor_app);
    menu_list_motor_mode = menu_list_init(menu_motor_mode, ARRAY_SIZE(menu_motor_mode), 4, 0, gui_handler_motor_mode);
    menu_list_tor_editor = menu_list_init(menu_tor_editor, ARRAY_SIZE(menu_tor_editor), 4, 0, gui_handler_editor);
    menu_list_vel_editor = menu_list_init(menu_vel_editor, ARRAY_SIZE(menu_vel_editor), 4, 0, gui_handler_editor);
    menu_list_ang_editor = menu_list_init(menu_ang_editor, ARRAY_SIZE(menu_ang_editor), 4, 0, gui_handler_editor);
    menu_list_easing     = menu_list_init(menu_easing, ARRAY_SIZE(menu_easing), 5, 0, gui_handler_easing_chart);

    PAGE_REGISTER(PAGE_ID_MAIN_MENU_LIST, menu_list_main);
    PAGE_REGISTER(PAGE_ID_MOTOR_APP_MENU_LIST, menu_list_motor_app);
    PAGE_REGISTER(PAGE_ID_MOTOR_MODE_MENU_LIST, menu_list_motor_mode);
    PAGE_REGISTER(PAGE_ID_TOR_EDITOR_MENU_LIST, menu_list_tor_editor);
    PAGE_REGISTER(PAGE_ID_VEL_EDITOR_MENU_LIST, menu_list_vel_editor);
    PAGE_REGISTER(PAGE_ID_ANG_EDITOR_MENU_LIST, menu_list_ang_editor);
    PAGE_REGISTER(PAGE_ID_EASING_CHART, menu_list_easing);

    gui_switch(PAGE_ID_FIRST);
}

void gui_render(void) {
    pages[page_selected]->handler(pages[page_selected]);
    if (pages[page_selected]->repaint_) {
        pages[page_selected]->repaint_ = FALSE;
        pages[page_selected]->painter(pages[page_selected]);
    }
}

void gui_switch(uint8_t id) {
    page_selected = id;
    if (pages[page_selected] != 0) {
        pages[page_selected]->repaint_ = TRUE;
    }
}