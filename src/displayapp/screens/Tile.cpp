#include "displayapp/screens/Tile.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/Colors.h"
#include "components/ble/BleController.h"
#include "displayapp/InfiniTimeTheme.h"
#include <cstdint>
#include <lvgl/src/core/lv_obj.h>
#include <lvgl/src/core/lv_obj_style.h>
#include <lvgl/src/lv_api_map_v8.h>

using namespace Pinetime::Applications::Screens;

namespace {
  void lv_update_task(struct _lv_timer_t* task) {
    auto* user_data = static_cast<Tile*>(task->user_data);
    user_data->UpdateScreen();
  }

  void event_handler_value_changed(lv_event_t* event) {
    auto* screen = static_cast<Tile*>(event->user_data);
    auto clickedButton = lv_buttonmatrix_get_selected_button(lv_event_get_target_obj(event));
    screen->OnValueChangedEvent(clickedButton);
  }
}

Tile::Tile(uint8_t screenID,
           uint8_t numScreens,
           DisplayApp* app,
           Controllers::Settings& settingsController,
           const Controllers::Battery& batteryController,
           const Controllers::Ble& bleController,
           Controllers::DateTime& dateTimeController,
           std::array<Applications, 6>& applications)
  : app {app}, dateTimeController {dateTimeController}, pageIndicator(screenID, numScreens), statusIcons(batteryController, bleController) {

  settingsController.SetAppMenu(screenID);

  statusIcons.Create();
  lv_obj_align(statusIcons.GetObject(), LV_ALIGN_TOP_RIGHT, -8, 0);

  // Time
  label_time = lv_label_create(lv_screen_active());
  lv_obj_set_align(label_time, LV_ALIGN_CENTER);
  lv_obj_align(label_time, LV_ALIGN_TOP_LEFT, 0, 0);

  pageIndicator.Create();

  uint8_t btIndex = 0;
  for (uint8_t i = 0; i < 6; i++) {
    if (i == 3) {
      btnmMap[btIndex++] = "\n";
    }
    if (applications[i].application == Apps::None) {
      btnmMap[btIndex] = " ";
    } else {
      btnmMap[btIndex] = applications[i].icon;
    }
    btIndex++;
    apps[i] = applications[i].application;
  }
  btnmMap[btIndex] = "";

  btnm1 = lv_btnmatrix_create(lv_screen_active());
  lv_btnmatrix_set_map(btnm1, btnmMap);
  lv_obj_set_size(btnm1, LV_HOR_RES - 16, LV_VER_RES - 60);
  lv_obj_align(btnm1, LV_ALIGN_CENTER, 0, 10);

  lv_obj_set_style_radius(btnm1, 20, LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(btnm1, LV_OPA_50, LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(btnm1, PINETIME_COLOR_CYAN, LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(btnm1, LV_OPA_50, LV_STATE_PRESSED);
  lv_obj_set_style_bg_color(btnm1, Colors::bgDark, LV_STATE_PRESSED);
  lv_obj_set_style_pad_all(btnm1, 0, LV_STATE_DEFAULT);
  lv_obj_set_style_text_align(btnm1, LV_ALIGN_CENTER, LV_STATE_DEFAULT);

  for (uint8_t i = 0; i < 6; i++) {
    lv_btnmatrix_set_btn_ctrl(btnm1, i, LV_BTNMATRIX_CTRL_CLICK_TRIG);
    if (applications[i].application == Apps::None || !applications[i].enabled) {
      lv_btnmatrix_set_btn_ctrl(btnm1, i, LV_BTNMATRIX_CTRL_DISABLED);
    }
  }

  btnm1->user_data = this;
  lv_obj_add_event_cb(btnm1, event_handler_value_changed, LV_EVENT_VALUE_CHANGED, this);

  taskUpdate = lv_timer_create(lv_update_task, 5000, this);
  UpdateScreen();
}

Tile::~Tile() {
  lv_timer_del(taskUpdate);
  lv_obj_clean(lv_screen_active());
}

void Tile::UpdateScreen() {
  lv_label_set_text(label_time, dateTimeController.FormattedTime().c_str());
  statusIcons.Update();
}

void Tile::OnValueChangedEvent(uint32_t buttonId) {
  app->StartApp(apps[buttonId], DisplayApp::FullRefreshDirections::Up);
  running = false;
}
