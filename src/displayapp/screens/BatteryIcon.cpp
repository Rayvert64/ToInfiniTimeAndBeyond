#include <cstdint>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/icons/battery/batteryicon.c"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/Colors.h"
#include "lvgl/src/core/lv_obj.h"
#include "lvgl/src/misc/lv_rb.h"

using namespace Pinetime::Applications::Screens;

BatteryIcon::BatteryIcon(bool colorOnLowBattery) : colorOnLowBattery {colorOnLowBattery} {};

void BatteryIcon::Create(lv_obj_t* parent) {
  batteryImg = lv_img_create(parent);
  lv_img_set_src(batteryImg, &batteryicon);
  lv_obj_set_style_bg_image_recolor(batteryImg, lv_color_black(), _LV_STYLE_STATE_CMP_SAME);

  batteryJuice = lv_obj_create(batteryImg);
  lv_obj_set_width(batteryJuice, 8);
  lv_obj_align(batteryJuice, LV_ALIGN_BOTTOM_RIGHT, -2, -2);
  lv_obj_set_style_radius(batteryJuice, 0, LV_STATE_DEFAULT);
}

lv_obj_t* BatteryIcon::GetObject() {
  return batteryImg;
}

void BatteryIcon::SetBatteryPercentage(uint8_t percentage) {
  lv_obj_set_height(batteryJuice, percentage * 14 / 100);

  if (colorOnLowBattery) {
    static constexpr int lowBatteryThreshold = 15;
    static constexpr int criticalBatteryThreshold = 5;
    if (percentage > lowBatteryThreshold) {
      SetColor(lv_color_white());
    } else if (percentage > criticalBatteryThreshold) {
      SetColor(PINETIME_COLOR_ORANGE);
    } else {
      SetColor(Colors::deepOrange);
    }
  }
}

void BatteryIcon::SetColor(lv_color_t color) {
  lv_obj_set_style_bg_image_recolor(batteryImg, color, LV_STATE_DEFAULT);
  lv_obj_set_style_bg_image_recolor_opa(batteryImg, LV_OPA_COVER, LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(batteryJuice, color, LV_PART_MAIN);
}

const char* BatteryIcon::GetPlugIcon(bool isCharging) {
  if (isCharging)
    return Symbols::plug;
  else
    return "";
}
