#include "base.h"
#include "qfplib-m3.h"

#include <stdio.h>

MenuBase g_menu_base;
static char str_buf[16];

static void get_str_width(const char *str) {
    u8g2_GetStrWidth(&u8g2, str);
}

static void draw_str(u8g2_uint_t x, u8g2_uint_t y, const char *str) {
    u8g2_DrawStr(&u8g2, x, y, str);
}

static void draw_num(u8g2_uint_t x, u8g2_uint_t y, float num) {
    sprintf(str_buf, "%.2f", num);
    u8g2_DrawStr(&u8g2, x, y, str_buf);
}

static void draw_pixel(u8g2_uint_t x, u8g2_uint_t y) {
    u8g2_DrawPixel(&u8g2, x, y);
}

static void draw_hline(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w) {
    u8g2_DrawHLine(&u8g2, x, y, w);
}

static void draw_vline(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t h) {
    u8g2_DrawVLine(&u8g2, x, y, h);
}

static void draw_line(u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2, u8g2_uint_t y2) {
    u8g2_DrawLine(&u8g2, x1, y1, x2, y2);
}

static void draw_rect(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h) {
    u8g2_DrawBox(&u8g2, x, y, w, h);
}

static void draw_round_rect(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r) {
    u8g2_DrawRBox(&u8g2, x, y, w, h, r);
}

static void draw_fill_rect(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h) {
    u8g2_DrawFrame(&u8g2, x, y, w, h);
}
static void draw_fill_round_rect(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r) {
    u8g2_DrawRFrame(&u8g2, x, y, w, h, r);
}

static void draw_circle(u8g2_uint_t cx, u8g2_uint_t cy, u8g2_uint_t r, uint8_t option) {
    u8g2_DrawCircle(&u8g2, cx, cy, r, option);
}

static void draw_circle_full(u8g2_uint_t cx, u8g2_uint_t cy, u8g2_uint_t r) {
    u8g2_DrawCircle(&u8g2, cx, cy, r, U8G2_DRAW_ALL);
}

static void draw_circle_full_center(u8g2_uint_t cx, u8g2_uint_t cy, u8g2_uint_t r) {
    draw_circle_full(OLED_WIDTH / 2 + cx, OLED_HEIGHT / 2 + cy, r);
}

/**
 * @param x0 x axis value of rect center
 * @param y0 y axis value of rect center
 */
static void draw_rotated_rect(float x0, float y0, float w, float h, float rad) {
    float c  = qfp_fcos(rad);
    float s  = qfp_fsin(rad);
    float ws = qfp_fmul(w, s);
    float wc = qfp_fmul(w, c);
    float hs = qfp_fmul(h, s);
    float hc = qfp_fmul(h, c);

    u8g2_ClearPolygonXY();
    u8g2_AddPolygonXY(&u8g2, qfp_fadd(qfp_fadd(-wc,  hs), x0), qfp_fadd(qfp_fadd(-ws, -hc), y0));
    u8g2_AddPolygonXY(&u8g2, qfp_fadd(qfp_fadd( wc,  hs), x0), qfp_fadd(qfp_fadd( ws, -hc), y0));
    u8g2_AddPolygonXY(&u8g2, qfp_fadd(qfp_fadd( wc, -hs), x0), qfp_fadd(qfp_fadd( ws,  hc), y0));
    u8g2_AddPolygonXY(&u8g2, qfp_fadd(qfp_fadd(-wc, -hs), x0), qfp_fadd(qfp_fadd(-ws,  hc), y0));
    u8g2_DrawPolygon(&u8g2);
}

static void set_color(uint8_t c) {
    u8g2_SetDrawColor(&u8g2, c);
}

static void clear(void) {
    u8g2_ClearBuffer(&u8g2);
}

static void update(void) {
    u8g2_SendBuffer(&u8g2);
}

void menu_init(void) {
    g_menu_base.get_str_width           = get_str_width;
    g_menu_base.draw_str                = draw_str;
    g_menu_base.draw_num                = draw_num;
    g_menu_base.draw_pixel              = draw_pixel;
    g_menu_base.draw_hline              = draw_hline;
    g_menu_base.draw_vline              = draw_vline;
    g_menu_base.draw_line               = draw_line;
    g_menu_base.draw_rect               = draw_rect;
    g_menu_base.draw_round_rect         = draw_round_rect;
    g_menu_base.draw_fill_rect          = draw_fill_rect;
    g_menu_base.draw_fill_round_rect    = draw_fill_round_rect;
    g_menu_base.draw_circle             = draw_circle;
    g_menu_base.draw_circle_full        = draw_circle_full;
    g_menu_base.draw_circle_full_center = draw_circle_full_center;
    g_menu_base.draw_rotated_rect       = draw_rotated_rect;
    g_menu_base.set_color               = set_color;
    g_menu_base.clear                   = clear;
    g_menu_base.update                  = update;
}