#include "gui.h"
#include "config.h"
#include "qfplib-m3.h"

#define X_PADDING          (4)

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

/* ---------------- LOGO ---------------- */
void gui_painter_logo(page_t* pg) {
    (void)pg;
    g_gui_base.clear();
    g_gui_base.set_color(0);
    g_gui_base.draw_str(X_PADDING, 12, "Hello World");
    g_gui_base.set_color(1);
    g_gui_base.draw_str(X_PADDING, 24, "Hello World");
    g_gui_base.set_color(2);
    g_gui_base.draw_str(X_PADDING, 36, "Hello World");
    g_gui_base.update();
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
    {PAGE_ID_LOGO,              "MainUI",                0},
    {PAGE_ID_TOR_PID_MENU_LIST, "- Torque PID Editor",   0},
    {PAGE_ID_VEL_PID_MENU_LIST, "- Velocity PID Editor", 0},
    {PAGE_ID_ANG_PID_MENU_LIST, "- Angle PID Editor",    0},
    {PAGE_ID_EASING_CHART,      "- Easing Chart",        0},
    {PAGE_ID_ABOUT,             "- About",               0},
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
                case PAGE_ID_TOR_PID_MENU_LIST:
                case PAGE_ID_VEL_PID_MENU_LIST:
                case PAGE_ID_ANG_PID_MENU_LIST: {
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

/* ---------------- PID BASE ---------------- */
enum {
    FUNC_ID_EDIT_KP = FUNC_ID_RETURN + 1,
    FUNC_ID_EDIT_KI,
};

PIDSliderBase Torque_Kp = {"Editing Kp", 0.0f, 2.5f, 0.0f, 0.1f};
// PIDSliderBase Torque_Ki = {"Forbidden", 0.0f, 0.0f, 0.0f, 0.0f};

PIDSliderBase Velocity_Kp = {"Editing Kp", 0.0f, 0.5f, 0.05f, 0.01f};
PIDSliderBase Velocity_Ki = {"Editing Ki", 0.0f, 2.0f, 1.0f, 0.1f};

PIDSliderBase Angle_Kp = {"Editing Kp", 0.0f, 8.0f, 4.0f, 0.5f};
// PIDSliderBase Angle_Ki = {"Forbidden", 0.0f, 0.0f, 0.0f, 0.0f};

const menu_item_t menu_tor_pid[] = {
    {FUNC_ID_EDIT_KP, "- Proportion", &Torque_Kp},
    // {FUNC_ID_EDIT_KI, "- Integral",   &Torque_Ki},
    {FUNC_ID_RETURN, "Return", 0},
};

const menu_item_t menu_vel_pid[] = {
    {FUNC_ID_EDIT_KP, "- Proportion", &Velocity_Kp},
    {FUNC_ID_EDIT_KI, "- Integral",   &Velocity_Ki},
    {FUNC_ID_RETURN, "Return", 0},
};

const menu_item_t menu_ang_pid[] = {
    {FUNC_ID_EDIT_KP, "- Proportion", &Angle_Kp},
    // {FUNC_ID_EDIT_KI, "- Integral",   &Angle_Ki},
    {FUNC_ID_RETURN, "Return", 0},
};

void gui_hander_pid(MenuList* pMenuList)
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
                case FUNC_ID_EDIT_KP:
                case FUNC_ID_EDIT_KI: {
                    BOOL repaint = TRUE;
                    // Blur the background
                    for (uint16_t i = 0; i < g_u8g2_buf_len; i++)
                        g_u8g2_buf_ptr[i] &= (i % 2 == 0 ? 0x55 : 0xAA);

                    g_gui_base.update();
                    PIDSliderBase* pid_slider;
                    while (1) {
                        switch (vkey_scan()) {
                            case VKEY_ID_NONE:
                                break;
                            case VKEY_ID_PREV:
                                pid_slider = (PIDSliderBase*)(pMenuList->items_[pMenuList->selected_index_].arg);
                                if (pid_slider->curr > qfp_fadd(pid_slider->min, pid_slider->step)) {
                                    pid_slider->curr = qfp_fsub(pid_slider->curr, pid_slider->step);
                                } else {
                                    pid_slider->curr = pid_slider->min;
                                }
                                repaint = TRUE;
                                break;
                            case VKEY_ID_NEXT:
                                pid_slider = (PIDSliderBase*)(pMenuList->items_[pMenuList->selected_index_].arg);
                                if (pid_slider->curr < qfp_fsub(pid_slider->max, pid_slider->step)) {
                                    pid_slider->curr = qfp_fadd(pid_slider->curr, pid_slider->step);
                                } else {
                                    pid_slider->curr = pid_slider->max;
                                }
                                repaint = TRUE;
                                break;
                            case VKEY_ID_OK:
                                pMenuList->repaint_ = TRUE;
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

                            pid_slider = (PIDSliderBase*)(pMenuList->items_[pMenuList->selected_index_].arg);

                            g_gui_base.draw_rect(
                                20, // x
                                38, // y
                                qfp_fmul(
                                    qfp_fdiv(
                                        qfp_fsub(pid_slider->curr, pid_slider->min),
                                        qfp_fsub(pid_slider->max, pid_slider->min)
                                    ),
                                    56.0f
                                ),  // w
                                4   // h
                            );
                            g_gui_base.draw_str(22, 30, pid_slider->description);
                            g_gui_base.draw_num(81, 44, pid_slider->curr);

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
static MenuList menu_list_tor_pid;
static MenuList menu_list_vel_pid;
static MenuList menu_list_ang_pid;
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

    menu_list_main    = menu_list_init(menu_main, ARRAY_SIZE(menu_main), 4, 0, gui_handler_main);
    menu_list_tor_pid = menu_list_init(menu_tor_pid, ARRAY_SIZE(menu_tor_pid), 4, 0, gui_hander_pid);
    menu_list_vel_pid = menu_list_init(menu_vel_pid, ARRAY_SIZE(menu_vel_pid), 4, 0, gui_hander_pid);
    menu_list_ang_pid = menu_list_init(menu_ang_pid, ARRAY_SIZE(menu_ang_pid), 4, 0, gui_hander_pid);
    menu_list_easing  = menu_list_init(menu_easing, ARRAY_SIZE(menu_easing), 5, 0, gui_handler_easing_chart);

    PAGE_REGISTER(PAGE_ID_MAIN_MENU_LIST, menu_list_main);
    PAGE_REGISTER(PAGE_ID_TOR_PID_MENU_LIST, menu_list_tor_pid);
    PAGE_REGISTER(PAGE_ID_VEL_PID_MENU_LIST, menu_list_vel_pid);
    PAGE_REGISTER(PAGE_ID_ANG_PID_MENU_LIST, menu_list_ang_pid);
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