#include "displayapp/Colors.h"

using namespace Pinetime::Applications;
using namespace Pinetime::Controllers;

lv_color_t Pinetime::Applications::Convert(Pinetime::Controllers::Settings::Colors color) {
  switch (color) {
    case Pinetime::Controllers::Settings::Colors::White:
      return lv_color_white();
    case Pinetime::Controllers::Settings::Colors::Silver:
      return LV_COLOR_MAKE(0xC0, 0xC0, 0xC0);
    case Pinetime::Controllers::Settings::Colors::Gray:
      return LV_COLOR_MAKE(0x80, 0x80, 0x80);
    case Pinetime::Controllers::Settings::Colors::Black:
      return lv_color_black();
    case Pinetime::Controllers::Settings::Colors::Red:
      return LV_COLOR_MAKE(0xFF, 0x00, 0x00);
    case Pinetime::Controllers::Settings::Colors::Maroon:
      return LV_COLOR_MAKE(0xb0, 0x0, 0x0);
    case Pinetime::Controllers::Settings::Colors::Yellow:
      return LV_COLOR_MAKE(0xFF, 0xFF, 0x00);
    case Pinetime::Controllers::Settings::Colors::Olive:
      return LV_COLOR_MAKE(0xb0, 0xb0, 0x0);
    case Pinetime::Controllers::Settings::Colors::Lime:
      return LV_COLOR_MAKE(0x00, 0xFF, 0x00);
    case Pinetime::Controllers::Settings::Colors::Green:
      return LV_COLOR_MAKE(0x0, 0xb0, 0x0);
    case Pinetime::Controllers::Settings::Colors::Cyan:
      return LV_COLOR_MAKE(0x00, 0xFF, 0xFF);
    case Pinetime::Controllers::Settings::Colors::Teal:
      return LV_COLOR_MAKE(0x0, 0xb0, 0xb0);
    case Pinetime::Controllers::Settings::Colors::Blue:
      return LV_COLOR_MAKE(0x00, 0x00, 0xFF);
    case Pinetime::Controllers::Settings::Colors::Navy:
      return LV_COLOR_MAKE(0x0, 0x0, 0xb0);
    case Pinetime::Controllers::Settings::Colors::Magenta:
      return LV_COLOR_MAKE(0xFF, 0x00, 0xFF);
    case Pinetime::Controllers::Settings::Colors::Purple:
      return LV_COLOR_MAKE(0xb0, 0x0, 0xb0);
    case Pinetime::Controllers::Settings::Colors::Orange:
      return LV_COLOR_MAKE(0xFF, 0xA5, 0x00);
    case Pinetime::Controllers::Settings::Colors::Pink:
      return LV_COLOR_MAKE(0xFF, 0xAE, 0xC9);
    default:
      return lv_color_white();
  }
}
