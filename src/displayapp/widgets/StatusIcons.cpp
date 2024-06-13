#include "displayapp/widgets/StatusIcons.h"
#include "displayapp/screens/Symbols.h"
#include <lvgl/src/core/lv_obj.h>

using namespace Pinetime::Applications::Widgets;

StatusIcons::StatusIcons(const Controllers::Battery& batteryController, const Controllers::Ble& bleController)
  : batteryIcon(true), batteryController {batteryController}, bleController {bleController} {
}

void StatusIcons::Create() {
  container = lv_obj_create(lv_screen_active());
  lv_obj_set_style_text_align(container, LV_ALIGN_TOP_LEFT, LV_STATE_ANY);
  lv_obj_set_style_text_align(container, LV_ALIGN_CENTER, LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_STATE_DEFAULT);

  bleIcon = lv_label_create(container);
  lv_label_set_text_static(bleIcon, Screens::Symbols::bluetooth);

  batteryPlug = lv_label_create(container);
  lv_label_set_text_static(batteryPlug, Screens::Symbols::plug);

  batteryIcon.Create(container);

  lv_obj_align(container, LV_ALIGN_TOP_RIGHT, 0, 0);
}

void StatusIcons::Update() {
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    if (powerPresent.Get()) {
      lv_obj_add_flag(batteryPlug, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_remove_flag(batteryPlug, LV_OBJ_FLAG_HIDDEN);
    }
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    batteryIcon.SetBatteryPercentage(batteryPercent);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    if (bleState.Get()) {
      lv_obj_add_flag(bleIcon, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_remove_flag(bleIcon, LV_OBJ_FLAG_HIDDEN);
    }
  }
}
