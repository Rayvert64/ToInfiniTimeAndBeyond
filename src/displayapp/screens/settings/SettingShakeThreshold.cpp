#include "displayapp/screens/settings/SettingShakeThreshold.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/Colors.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    SettingShakeThreshold* screen = static_cast<SettingShakeThreshold*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingShakeThreshold::SettingShakeThreshold(Controllers::Settings& settingsController,
                                             Controllers::MotionController& motionController,
                                             System::SystemTask& systemTask)
  : settingsController {settingsController}, motionController {motionController}, systemTask {systemTask} {

  lv_obj_t* title = lv_label_create(lv_screen_active());
  lv_label_set_text_static(title, "Wake Sensitivity");
  lv_obj_set_align(title, LV_ALIGN_CENTER);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

  positionArc = lv_arc_create(lv_screen_active());
  positionArc->user_data = this;

  lv_obj_add_event_cb(positionArc, event_handler);
  lv_arc_set_bg_angles(positionArc, 180, 360);
  lv_arc_set_range(positionArc, 0, 4095);
  lv_arc_set_adjustable(positionArc, true);
  lv_obj_set_width(positionArc, lv_obj_get_width(lv_screen_active()) - 10);
  lv_obj_set_height(positionArc, 240);
  lv_obj_align(positionArc, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  animArc = lv_arc_create(positionArc, positionArc);
  lv_arc_set_adjustable(animArc, false);
  lv_obj_set_width(animArc, lv_obj_get_width(positionArc));
  lv_obj_set_height(animArc, lv_obj_get_height(positionArc));
  lv_obj_align_mid(animArc, positionArc, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_line_opa(animArc, 0, LV_STATE_DEFAULT);
  ;
  lv_obj_set_style_line_opa(animArc, LV_OPA_70, LV_STATE_DEFAULT);
  ;
  lv_obj_set_style_line_opa(animArc, LV_OPA_0, LV_STATE_DEFAULT);
  ;
  lv_obj_set_style_line_color(animArc, PINETIME_COLOR_RED, LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(animArc, LV_COLOR_TRANSP, LV_ARC_PART_BG);

  animArc->user_data = this;
  lv_obj_remove_flag(animArc, LV_OBJ_FLAG_CLICKABLE);

  calButton = lv_button_create(lv_screen_active());
  calButton->user_data = this;
  lv_obj_add_event_cb(calButton, event_handler);
  lv_obj_set_height(calButton, 80);
  lv_obj_set_width(calButton, 200);
  lv_obj_align(calButton, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_btn_set_checkable(calButton, true);
  calLabel = lv_label_create(calButton);
  lv_label_set_text_static(calLabel, "Calibrate");

  lv_arc_set_value(positionArc, settingsController.GetShakeThreshold());

  vDecay = xTaskGetTickCount();
  calibrating = false;
  EnableForCal = false;
  if (!settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::Shake)) {
    EnableForCal = true;
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::Shake, true);
  }
  refreshTask = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

SettingShakeThreshold::~SettingShakeThreshold() {
  settingsController.SetShakeThreshold(lv_arc_get_value(positionArc));

  if (EnableForCal) {
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::Shake, false);
    EnableForCal = false;
  }
  lv_timer_del(refreshTask);
  settingsController.SaveSettings();
  lv_obj_clean(lv_screen_active());
}

void SettingShakeThreshold::Refresh() {

  if (calibrating == 1) {
    if (xTaskGetTickCount() - vCalTime > pdMS_TO_TICKS(2000)) {
      vCalTime = xTaskGetTickCount();
      calibrating = 2;
      lv_obj_set_style_bg_color(calButton, PINETIME_COLOR_RED, LV_PART_MAIN);
      lv_obj_set_style_bg_color(calButton, PINETIME_COLOR_RED, LV_PART_MAIN);
      lv_label_set_text_static(calLabel, "Shake!");
    }
  }
  if (calibrating == 2) {

    if ((motionController.CurrentShakeSpeed() - 300) > lv_arc_get_value(positionArc)) {
      lv_arc_set_value(positionArc, (int16_t) motionController.CurrentShakeSpeed() - 300);
    }
    if (xTaskGetTickCount() - vCalTime > pdMS_TO_TICKS(7500)) {
      lv_btn_set_state(calButton, LV_STATE_DEFAULT);
      lv_event_send(calButton, LV_EVENT_VALUE_CHANGED, nullptr);
    }
  }
  if (motionController.CurrentShakeSpeed() - 300 > lv_arc_get_value(animArc)) {
    lv_arc_set_value(animArc, (uint16_t) motionController.CurrentShakeSpeed() - 300);
    vDecay = xTaskGetTickCount();
  } else if ((xTaskGetTickCount() - vDecay) > pdMS_TO_TICKS(1500)) {
    lv_arc_set_value(animArc, lv_arc_get_value(animArc) - 25);
  }
}

void SettingShakeThreshold::UpdateSelected(lv_obj_t* object, lv_event_t event) {

  switch (event) {
    case LV_EVENT_VALUE_CHANGED: {
      if (object == calButton) {
        if (lv_btn_get_state(calButton) == LV_BTN_STATE_CHECKED_RELEASED && calibrating == 0) {
          lv_arc_set_value(positionArc, 0);
          calibrating = 1;
          vCalTime = xTaskGetTickCount();
          lv_label_set_text_static(calLabel, "Ready!");
          lv_obj_remove_flag(positionArc, LV_OBJ_FLAG_CLICKABLE);
          lv_obj_set_style_bg_color(calButton, Colors::highlight, LV_PART_MAIN);
        } else if (lv_btn_get_state(calButton) == LV_BTN_STATE_RELEASED) {
          calibrating = 0;
          lv_obj_add_flag(positionArc, LV_OBJ_FLAG_CLICKABLE);
          lv_label_set_text_static(calLabel, "Calibrate");
        }
        break;
      }
    }
  }
}
