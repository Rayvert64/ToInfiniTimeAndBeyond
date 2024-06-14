#include "displayapp/screens/settings/SettingWakeUp.h"
#include <lvgl/lvgl.h>
#include <lvgl/src/core/lv_obj.h>
#include <lvgl/src/core/lv_obj_event.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Colors.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Styles.h"

using namespace Pinetime::Applications::Screens;

constexpr std::array<SettingWakeUp::Option, 5> SettingWakeUp::options;

namespace {
  void event_handler(lv_event_t* event) {
    auto* screen = static_cast<SettingWakeUp*>(event->user_data);
    screen->UpdateSelected(lv_event_get_target_obj(event));
  }
}

SettingWakeUp::SettingWakeUp(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {
  lv_obj_t* container1 = lv_obj_create(lv_screen_active());

  lv_obj_set_style_bg_opa(container1, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_style_pad_all(container1, 10, LV_STATE_DEFAULT);
  lv_obj_set_style_text_align(container1, LV_ALIGN_CENTER, LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(container1, 0, LV_PART_MAIN);

  lv_obj_set_pos(container1, 10, 35);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 20);
  lv_obj_set_style_text_align(container1, LV_ALIGN_TOP_LEFT, LV_STATE_ANY);

  lv_obj_t* title = lv_label_create(lv_screen_active());
  lv_label_set_text_static(title, "Wake Up");
  lv_obj_set_align(title, LV_ALIGN_CENTER);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(icon, PINETIME_COLOR_ORANGE, LV_STATE_DEFAULT);
  lv_label_set_text_static(icon, Symbols::eye);
  lv_obj_set_align(icon, LV_ALIGN_CENTER);
  lv_obj_align(icon, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  for (unsigned int i = 0; i < options.size(); i++) {
    cbOption[i] = lv_checkbox_create(container1);
    lv_checkbox_set_text(cbOption[i], options[i].name);
    if (settingsController.isWakeUpModeOn(static_cast<Controllers::Settings::WakeUpMode>(i))) {
      lv_obj_set_state(cbOption[i], LV_STATE_CHECKED, true);
    }
    cbOption[i]->user_data = this;
    lv_obj_add_event_cb(cbOption[i], event_handler, LV_EVENT_VALUE_CHANGED, this);
  }
}

SettingWakeUp::~SettingWakeUp() {
  lv_obj_clean(lv_screen_active());
  settingsController.SaveSettings();
}

void SettingWakeUp::UpdateSelected(lv_obj_t* object) {
  // Find the index of the checkbox that triggered the event
  for (size_t i = 0; i < options.size(); i++) {
    if (cbOption[i] == object) {
      bool currentState = settingsController.isWakeUpModeOn(options[i].wakeUpMode);
      settingsController.setWakeUpMode(options[i].wakeUpMode, !currentState);
      break;
    }
  }

  // Update checkbox according to current wakeup modes.
  // This is needed because we can have extra logic when setting or unsetting wakeup modes,
  // for example, when setting SingleTap, DoubleTap is unset and vice versa.
  auto modes = settingsController.getWakeUpModes();
  for (size_t i = 0; i < options.size(); ++i) {
    lv_obj_set_state(cbOption[i], LV_STATE_CHECKED, modes[i]);
  }
}
