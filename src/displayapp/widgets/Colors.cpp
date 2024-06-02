#include "displayapp/Colors.h"

using namespace Pinetime::Applications;
using namespace Pinetime::Controllers;

lv_color_t Pinetime::Applications::Convert(Pinetime::Controllers::Settings::Colors color) {
  switch (color) {
    case Pinetime::Controllers::Settings::Colors::White:
      return lv_color_white();
    case Pinetime::Controllers::Settings::Colors::Silver:
      return PINETIME_COLOR_SILVER;
    case Pinetime::Controllers::Settings::Colors::Gray:
      return PINETIME_COLOR_GRAY;
    case Pinetime::Controllers::Settings::Colors::Black:
      return lv_color_black();
    case Pinetime::Controllers::Settings::Colors::Red:
      return PINETIME_COLOR_RED;
    case Pinetime::Controllers::Settings::Colors::Maroon:
      return LV_COLOR_MAKE(0xb0, 0x0, 0x0);
    case Pinetime::Controllers::Settings::Colors::Yellow:
      return PINETIME_COLOR_YELLOW;
    case Pinetime::Controllers::Settings::Colors::Olive:
      return LV_COLOR_MAKE(0xb0, 0xb0, 0x0);
    case Pinetime::Controllers::Settings::Colors::Lime:
      return PINETIME_COLOR_LIME;
    case Pinetime::Controllers::Settings::Colors::Green:
      return LV_COLOR_MAKE(0x0, 0xb0, 0x0);
    case Pinetime::Controllers::Settings::Colors::Cyan:
      return PINETIME_COLOR_CYAN;
    case Pinetime::Controllers::Settings::Colors::Teal:
      return LV_COLOR_MAKE(0x0, 0xb0, 0xb0);
    case Pinetime::Controllers::Settings::Colors::Blue:
      return PINETIME_COLOR_BLUE;
    case Pinetime::Controllers::Settings::Colors::Navy:
      return LV_COLOR_MAKE(0x0, 0x0, 0xb0);
    case Pinetime::Controllers::Settings::Colors::Magenta:
      return PINETIME_COLOR_MAGENTA;
    case Pinetime::Controllers::Settings::Colors::Purple:
      return LV_COLOR_MAKE(0xb0, 0x0, 0xb0);
    case Pinetime::Controllers::Settings::Colors::Orange:
      return PINETIME_COLOR_ORANGE;
    case Pinetime::Controllers::Settings::Colors::Pink:
      return LV_COLOR_MAKE(0xFF, 0xAE, 0xC9);
    default:
      return lv_color_white();
  }
}
