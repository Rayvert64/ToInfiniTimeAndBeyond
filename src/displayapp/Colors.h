#pragma once

#include <lvgl/src/misc/lv_color.h>
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Applications {
    #define PINETIME_COLOR_WHITE LV_COLOR_MAKE(0xFF, 0xFF, 0xFF)
    #define PINETIME_COLOR_SILVER LV_COLOR_MAKE(0xC0, 0xC0, 0xC0)
    #define PINETIME_COLOR_GRAY LV_COLOR_MAKE(0x80, 0x80, 0x80)
    #define PINETIME_COLOR_BLACK LV_COLOR_MAKE(0x00, 0x00, 0x00)
    #define PINETIME_COLOR_RED LV_COLOR_MAKE(0xFF, 0x00, 0x00)
    #define PINETIME_COLOR_MAROON LV_COLOR_MAKE(0x80, 0x00, 0x00)
    #define PINETIME_COLOR_YELLOW LV_COLOR_MAKE(0xFF, 0xFF, 0x00)
    #define PINETIME_COLOR_OLIVE LV_COLOR_MAKE(0x80, 0x80, 0x00)
    #define PINETIME_COLOR_LIME LV_COLOR_MAKE(0x00, 0xFF, 0x00)
    #define PINETIME_COLOR_GREEN LV_COLOR_MAKE(0x00, 0x80, 0x00)
    #define PINETIME_COLOR_CYAN LV_COLOR_MAKE(0x00, 0xFF, 0xFF)
    #define PINETIME_COLOR_TEAL LV_COLOR_MAKE(0x00, 0x80, 0x80)
    #define PINETIME_COLOR_BLUE LV_COLOR_MAKE(0x00, 0x00, 0xFF)
    #define PINETIME_COLOR_NAVY LV_COLOR_MAKE(0x00, 0x00, 0x80)
    #define PINETIME_COLOR_MAGENTA LV_COLOR_MAKE(0xFF, 0x00, 0xFF)
    #define PINETIME_COLOR_PURPLE LV_COLOR_MAKE(0x80, 0x00, 0x80)
    #define PINETIME_COLOR_ORANGE LV_COLOR_MAKE(0xFF, 0xA5, 0x00)

    lv_color_t Convert(Controllers::Settings::Colors color);
  }
}