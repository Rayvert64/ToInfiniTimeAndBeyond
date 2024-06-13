#include "displayapp/screens/settings/SettingDisplay.h"
#include <lvgl/lvgl.h>
#include <lvgl/src/core/lv_obj_event.h>
#include <lvgl/src/misc/lv_event.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/Colors.h"
#include "displayapp/Messages.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_event_t* event) {
    auto* screen = static_cast<SettingDisplay*>(event->user_data);
    screen->UpdateSelected(event);
  }
}

constexpr std::array<uint16_t, 6> SettingDisplay::options;

SettingDisplay::SettingDisplay(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app}, settingsController {settingsController} {

  lv_obj_t* container1 = lv_obj_create(lv_screen_active());

  lv_obj_set_style_bg_opa(container1, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_style_pad_all(container1, 10, LV_STATE_DEFAULT);
  lv_obj_set_style_text_align(container1, LV_ALIGN_CENTER, LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(container1, 0, LV_PART_MAIN);

  lv_obj_set_pos(container1, 10, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 50);
  lv_obj_set_style_text_align(container1, LV_ALIGN_TOP_LEFT, LV_STATE_ANY);

  lv_obj_t* title = lv_label_create(lv_screen_active());
  lv_label_set_text_static(title, "Display timeout");
  lv_obj_set_align(title, LV_ALIGN_CENTER);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(icon, PINETIME_COLOR_ORANGE, LV_STATE_DEFAULT);
  lv_label_set_text_static(icon, Symbols::sun);
  lv_obj_set_align(icon, LV_ALIGN_CENTER);
  lv_obj_align(icon, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  char buffer[4];
  for (unsigned int i = 0; i < options.size(); i++) {
    cbOption[i] = lv_checkbox_create(container1);
    snprintf(buffer, sizeof(buffer), "%2" PRIu16 "s", options[i] / 1000);
    lv_checkbox_set_text(cbOption[i], buffer);
    cbOption[i]->user_data = this;
    lv_obj_add_event_cb(cbOption[i], event_handler, LV_EVENT_CLICKED, this);
    SetRadioButtonStyle(cbOption[i]);

    if (settingsController.GetScreenTimeOut() == options[i]) {
      lv_obj_set_state(cbOption[i], LV_STATE_CHECKED, true);
    }
  }
}

SettingDisplay::~SettingDisplay() {
  lv_obj_clean(lv_screen_active());
  settingsController.SaveSettings();
}

void SettingDisplay::UpdateSelected(lv_event_t* event) {
  auto* object = lv_event_get_target_obj(event);
  for (unsigned int i = 0; i < options.size(); i++) {
    if (object == cbOption[i]) {
      lv_obj_set_state(cbOption[i], LV_STATE_CHECKED, true);
      settingsController.SetScreenTimeOut(options[i]);
    } else {
      lv_obj_set_state(cbOption[i], LV_STATE_CHECKED, false);
    }
  }
}
