#include "menu_list.h"
#include "oled.h"
#include "vkey.h"
#include "qfplib-m3.h"

#define MENU_LIST_FRAME_NUM         (4)

void menu_list_callback_painter_update_easing(MenuList* pMenuList) {
    g_tween_handler.update(&pMenuList->y_title_offset_);
    g_tween_handler.update(&pMenuList->width_mask_);
    g_tween_handler.update(&pMenuList->y_mask_);
    g_tween_handler.update(&pMenuList->y_slider_);

    pMenuList->repaint_ =
        !(
            g_tween_handler.finished(&pMenuList->width_mask_)
            && g_tween_handler.finished(&pMenuList->y_mask_)
            && g_tween_handler.finished(&pMenuList->y_title_offset_)
            && g_tween_handler.finished(&pMenuList->y_slider_)
        );
}

void menu_list_callback_painter_draw_items(MenuList* pMenuList) {
    uint8_t index = qfp_fdiv(g_tween_handler.curr_pixel_pos(&pMenuList->y_title_offset_), pMenuList->height_line_);
    float y_offset =
        qfp_fsub(
            qfp_fmul((float)index, pMenuList->height_line_),
            g_tween_handler.curr_pixel_pos(&pMenuList->y_title_offset_)
        );

    while (index < pMenuList->item_num_ && (uint16_t)y_offset < pMenuList->height_) {
        g_gui_base.draw_str(
            (u8g2_uint_t)pMenuList->x_padding_,
            (u8g2_uint_t)qfp_fadd(y_offset, pMenuList->y_padding_),
            pMenuList->items_[index].title
        );
        index += 1;
        y_offset = qfp_fadd(y_offset, pMenuList->height_line_);
    }
}

void menu_list_callback_painter_draw_scroll(MenuList* pMenuList) {
    // draw vertical axis
    g_gui_base.draw_vline(pMenuList->x_slider_, 0, pMenuList->height_);

    float y = 0.0f;

    // draw scale
    for (uint8_t i = 0; i < pMenuList->item_num_; i++) {
        g_gui_base.draw_pixel(pMenuList->x_slider_ - 1, y);
        g_gui_base.draw_pixel(pMenuList->x_slider_ + 1, y);
        y = qfp_fadd(y, pMenuList->height_slider_);
    }
    g_gui_base.draw_pixel(pMenuList->x_slider_ - 1, pMenuList->height_ - 1);
    g_gui_base.draw_pixel(pMenuList->x_slider_ + 1, pMenuList->height_ - 1);

    // draw slider
    float y_slider_pixel_pos = g_tween_handler.curr_pixel_pos(&pMenuList->y_slider_);
    uint8_t y_slider_pixel_pos_truncated = (uint8_t)y_slider_pixel_pos;

    float h = qfp_fadd(pMenuList->height_slider_, (qfp_fsub(y_slider_pixel_pos, (float)y_slider_pixel_pos_truncated)) > 0.5f);
    g_gui_base.draw_vline(pMenuList->x_slider_ - 1, (u8g2_uint_t)y_slider_pixel_pos, (u8g2_uint_t)h);
    g_gui_base.draw_vline(pMenuList->x_slider_ + 1, (u8g2_uint_t)y_slider_pixel_pos, (u8g2_uint_t)h);
}

void menu_list_callback_painter_draw_item_mask(MenuList* pMenuList) {
    float y_mask_pixel_pos = g_tween_handler.curr_pixel_pos(&pMenuList->y_mask_);
    uint8_t y_mask_pixel_pos_truncated = (uint8_t)y_mask_pixel_pos;
    g_gui_base.set_color(2);
    g_gui_base.draw_round_rect(
        0,
        (u8g2_uint_t)y_mask_pixel_pos,
        (u8g2_uint_t)g_tween_handler.curr_pixel_pos(&pMenuList->width_mask_) + pMenuList->x_padding_ * 2,
        (u8g2_uint_t)pMenuList->height_line_ + (u8g2_uint_t)((qfp_fsub(y_mask_pixel_pos, (float)y_mask_pixel_pos_truncated)) > 0.5f),
        1
    );
}

void menu_list_callback_painter_by_default(MenuList* pMenuList) {
    menu_list_callback_painter_update_easing(pMenuList);
    // new frame
    g_gui_base.clear();
    // title
    menu_list_callback_painter_draw_items(pMenuList);
    // scroll
    menu_list_callback_painter_draw_scroll(pMenuList);
    // mask
    menu_list_callback_painter_draw_item_mask(pMenuList);
    // update screen
    g_gui_base.update();
}


void menu_list_callback_handler_switch_to_prev(MenuList* pMenuList) {
    if (pMenuList->selected_index_ > 0) {
        pMenuList->selected_index_ -= 1;
        // check if masked index is at top
        if (pMenuList->masked_index_ > 0) {
            pMenuList->masked_index_ -= 1;
            // decrease mask y pixel one line height
            g_tween_handler.start_relative(&pMenuList->y_mask_, -pMenuList->height_line_);
        } else {
            // decrease text y pixel one line height
            g_tween_handler.start_relative(&pMenuList->y_title_offset_, -pMenuList->height_line_);
        }
        // decrease scroll y pixel one slider height
        g_tween_handler.start_relative(&pMenuList->y_slider_, -pMenuList->height_slider_);
        // change mask width
        g_tween_handler.start_absolute(
            &pMenuList->width_mask_,
            pMenuList->width_mask_.current_,
            g_gui_base.get_str_width(pMenuList->items_[pMenuList->selected_index_].title)
        );
        // repaint
        pMenuList->repaint_ = TRUE;
    }
}

void menu_list_callback_handler_switch_to_next(MenuList* pMenuList) {
    if (pMenuList->selected_index_ < pMenuList->item_num_ - 1) {
        pMenuList->selected_index_ += 1;
        // check if masked index is at bottom
        if (pMenuList->masked_index_ < pMenuList->display_num_ - 1) {
            pMenuList->masked_index_ += 1;
            // increase mask y pixel one line height
            g_tween_handler.start_relative(&pMenuList->y_mask_, pMenuList->height_line_);
        } else {
            // increase text y pixel one line height
            g_tween_handler.start_relative(&pMenuList->y_title_offset_, pMenuList->height_line_);
        }
        // increase scroll y pixel one slider height
        g_tween_handler.start_relative(&pMenuList->y_slider_, pMenuList->height_slider_);
        // change mask width
        g_tween_handler.start_absolute(
            &pMenuList->width_mask_,
            pMenuList->width_mask_.current_,
            g_gui_base.get_str_width(pMenuList->items_[pMenuList->selected_index_].title)
        );
        // repaint
        pMenuList->repaint_ = TRUE;
    }
}

void menu_list_callback_handler_by_default(MenuList* pMenuList) {
    switch (vkey_scan()) {
        case VKEY_ID_NONE:
            break;
        case VKEY_ID_PREV:
            menu_list_callback_handler_switch_to_prev(pMenuList);
            break;
        case VKEY_ID_NEXT:
            menu_list_callback_handler_switch_to_next(pMenuList);
            break;
        default:
            printf("%s\r\n", pMenuList->items_[pMenuList->selected_index_].title);
            break;
    }
}

MenuList menu_list_init(
    const menu_item_t items[],
    const uint8_t     item_num,
    const uint8_t     display_num, /* default = 1 */
    const callback_t  painter,
    const callback_t  handler
)
{
    // no item
    if (item_num == 0) {
        MenuList menu_none = {0};
        return menu_none;
    }

    MenuList menu = {
        .repaint_ = TRUE,
        .painter  = (painter == 0)
                    ? (callback_t)menu_list_callback_painter_by_default
                    : painter,
        .handler  = (handler == 0)
                    ? (callback_t)menu_list_callback_handler_by_default
                    : handler,

        .width_  = (uint16_t)OLED_WIDTH,
        .height_ = (uint16_t)OLED_HEIGHT,

        .items_         = (menu_item_t*)items,
        .item_num_      = item_num,
        .display_num_   = display_num == 0 ? 1 : display_num,
        .height_line_   = (float)(((uint8_t)OLED_HEIGHT) / display_num),
        .height_slider_ = (float)(((uint8_t)OLED_HEIGHT) / item_num),

        .masked_index_   = 0,
        .selected_index_ = 0,

        .x_padding_ = 4,
        .y_padding_ = 8, // font size

        .x_slider_  = (uint8_t)OLED_WIDTH - 2,
        .y_slider_  = g_tween_handler.create(
                            TWEEN_MODE_DEFAULT,
                            easing_Out_Cubic,
                            0.0f, /* offset = 0 */
                            MENU_LIST_FRAME_NUM,
                            0 /* duration = 0 */
                        ),

        .width_mask_ = g_tween_handler.create(
                            TWEEN_MODE_DEFAULT,
                            easing_Out_Cubic,
                            0.0f, /* offset = 0 */
                            MENU_LIST_FRAME_NUM,
                            0 /* duration = 0 */
                        ),
        .y_mask_     = g_tween_handler.create(
                            TWEEN_MODE_DEFAULT,
                            easing_Out_Cubic,
                            0.0f, /* offset = 0 */
                            MENU_LIST_FRAME_NUM,
                            0 /* duration = 0 */
                        ),

        .y_title_offset_ = g_tween_handler.create(
                            TWEEN_MODE_DEFAULT,
                            easing_Linear,
                            0.0f, /* offset = 0 */
                            MENU_LIST_FRAME_NUM,
                            0 /* duration = 0 */
                        ),
    };

    menu.width_mask_.current_ = (float)g_gui_base.get_str_width(items[0].title);
    menu.y_padding_       = qfp_fadd(menu.y_padding_, qfp_fdiv(qfp_fsub(menu.height_line_, menu.y_padding_), 2.0f));

    return menu;
}