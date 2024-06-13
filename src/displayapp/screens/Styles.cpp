#include "Styles.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/src/core/lv_obj.h>

void Pinetime::Applications::Screens::SetRadioButtonStyle(lv_obj_t* checkbox) {
  lv_obj_set_style_radius(checkbox, LV_RADIUS_CIRCLE, LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(checkbox, 9, LV_PART_KNOB);
  lv_obj_set_style_border_color(checkbox, Colors::highlight, LV_PART_KNOB);
  lv_obj_set_style_bg_color(checkbox, lv_color_white(), LV_PART_KNOB);
}
