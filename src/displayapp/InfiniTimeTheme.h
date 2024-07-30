#pragma once

#include <lvgl/lvgl.h>

namespace Colors {
  static constexpr lv_color_t deepOrange = LV_COLOR_MAKE(0xff, 0x40, 0x0);
  static constexpr lv_color_t orange = LV_COLOR_MAKE(0xff, 0xb0, 0x0);
  static constexpr lv_color_t green = LV_COLOR_MAKE(0x0, 0xb0, 0x0);
  static constexpr lv_color_t blue = LV_COLOR_MAKE(0x0, 0x50, 0xff);
  static constexpr lv_color_t lightGray = LV_COLOR_MAKE(0xb0, 0xb0, 0xb0);

  static constexpr lv_color_t bg = LV_COLOR_MAKE(0x5d, 0x69, 0x7e);
  static constexpr lv_color_t bgAlt = LV_COLOR_MAKE(0x38, 0x38, 0x38);
  static constexpr lv_color_t bgDark = LV_COLOR_MAKE(0x18, 0x18, 0x18);
  static constexpr lv_color_t highlight = green;
  static constexpr lv_color_t ui_tan = LV_COLOR_MAKE(0xE2, 0xDE, 0xD3);
  static constexpr lv_color_t ui_sheika_blue_light = LV_COLOR_MAKE(0x00, 0xFF, 0xE7);
  static constexpr lv_color_t ui_sheika_blue_dark = LV_COLOR_MAKE(0x1E, 0x65, 0x8D);
  static constexpr lv_color_t ui_sheika_purple = LV_COLOR_MAKE(0xC4, 0x66, 0xCB);
  static constexpr lv_color_t ui_font_small_dark = LV_COLOR_MAKE(0x3C, 0xD3, 0xFC);
  static constexpr lv_color_t heart_red = LV_COLOR_MAKE(0xF1, 0x36, 0x2F);

};

/**
 * Initialize the default
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param flags ORed flags starting with `LV_THEME_DEF_FLAG_...`
 * @param font_small pointer to a small font
 * @param font_normal pointer to a normal font
 * @param font_subtitle pointer to a large font
 * @param font_title pointer to a extra large font
 * @return a pointer to reference this theme later
 */
lv_theme_t* lv_pinetime_theme_init();
