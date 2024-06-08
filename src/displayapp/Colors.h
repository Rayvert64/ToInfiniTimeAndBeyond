#pragma once

#include <lvgl/src/misc/lv_color.h>
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Applications {
#define PINETIME_COLOR_WHITE   lv_color_make(0xFF, 0xFF, 0xFF)
#define PINETIME_COLOR_SILVER  lv_color_make(0xC0, 0xC0, 0xC0)
#define PINETIME_COLOR_GRAY    lv_color_make(0x80, 0x80, 0x80)
#define PINETIME_COLOR_BLACK   lv_color_make(0x00, 0x00, 0x00)
#define PINETIME_COLOR_RED     lv_color_make(0xFF, 0x00, 0x00)
#define PINETIME_COLOR_MAROON  lv_color_make(0x80, 0x00, 0x00)
#define PINETIME_COLOR_YELLOW  lv_color_make(0xFF, 0xFF, 0x00)
#define PINETIME_COLOR_OLIVE   lv_color_make(0x80, 0x80, 0x00)
#define PINETIME_COLOR_LIME    lv_color_make(0x00, 0xFF, 0x00)
#define PINETIME_COLOR_GREEN   lv_color_make(0x00, 0x80, 0x00)
#define PINETIME_COLOR_CYAN    lv_color_make(0x00, 0xFF, 0xFF)
#define PINETIME_COLOR_TEAL    lv_color_make(0x00, 0x80, 0x80)
#define PINETIME_COLOR_BLUE    lv_color_make(0x00, 0x00, 0xFF)
#define PINETIME_COLOR_NAVY    lv_color_make(0x00, 0x00, 0x80)
#define PINETIME_COLOR_MAGENTA lv_color_make(0xFF, 0x00, 0xFF)
#define PINETIME_COLOR_PURPLE  lv_color_make(0x80, 0x00, 0x80)
#define PINETIME_COLOR_ORANGE  lv_color_make(0xFF, 0xA5, 0x00)

    lv_color_t Convert(Controllers::Settings::Colors color);
  }
}