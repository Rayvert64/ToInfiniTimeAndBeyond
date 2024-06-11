#include "PassKey.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

PassKey::PassKey(uint32_t key) {
  passkeyLabel = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(passkeyLabel, PINETIME_COLOR_YELLOW, LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(passkeyLabel, &jetbrains_mono_42, LV_STATE_DEFAULT);
  lv_label_set_text_fmt(passkeyLabel, "%06u", key);
  lv_obj_align(passkeyLabel, LV_ALIGN_CENTER, 0, -20);
}

PassKey::~PassKey() {
  lv_obj_clean(lv_scr_act());
}
