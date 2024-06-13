#include "displayapp/screens/settings/SettingSetTime.h"
#include <lvgl/lvgl.h>
#include <nrf_log.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/Colors.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t POS_Y_TEXT = -7;

  void SetTimeEventHandler(lv_event_t* event) {
    auto* screen = static_cast<SettingSetTime*>(event->user_data);
    screen->SetTime();
  }

  void ValueChangedHandler(void* userData) {
    auto* screen = static_cast<SettingSetTime*>(userData);
    screen->UpdateScreen();
  }
}

SettingSetTime::SettingSetTime(Pinetime::Controllers::DateTime& dateTimeController,
                               Pinetime::Controllers::Settings& settingsController,
                               Pinetime::Applications::Screens::SettingSetDateTime& settingSetDateTime)
  : dateTimeController {dateTimeController}, settingsController {settingsController}, settingSetDateTime {settingSetDateTime} {

  lv_obj_t* title = lv_label_create(lv_screen_active());
  lv_label_set_text_static(title, "Set current time");
  lv_obj_set_align(title, LV_ALIGN_CENTER);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(icon, PINETIME_COLOR_ORANGE, LV_STATE_DEFAULT);
  lv_label_set_text_static(icon, Symbols::clock);
  lv_obj_set_align(icon, LV_ALIGN_CENTER);
  lv_obj_align(icon, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  lv_obj_t* staticLabel = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(staticLabel, &jetbrains_mono_42, LV_STATE_DEFAULT);
  lv_label_set_text_static(staticLabel, "00:00:00");
  lv_obj_align(staticLabel, LV_ALIGN_CENTER, 0, POS_Y_TEXT);

  hourCounter.Create();
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    hourCounter.EnableTwelveHourMode();
  }
  lv_roller_set_selected(hourCounter.GetObject(), dateTimeController.Hours(), LV_ANIM_ON);
  lv_obj_align(hourCounter.GetObject(), LV_ALIGN_CENTER, -75, POS_Y_TEXT);
  hourCounter.SetValueChangedEventCallback(this, ValueChangedHandler);

  minuteCounter.Create();
  lv_roller_set_selected(minuteCounter.GetObject(), dateTimeController.Minutes(), LV_ANIM_ON);
  lv_obj_align(minuteCounter.GetObject(), LV_ALIGN_CENTER, 0, POS_Y_TEXT);
  minuteCounter.SetValueChangedEventCallback(this, ValueChangedHandler);

  lblampm = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(lblampm, &jetbrains_mono_bold_20, LV_STATE_DEFAULT);
  lv_label_set_text_static(lblampm, "  ");
  lv_obj_align(lblampm, LV_ALIGN_CENTER, 75, -50);

  btnSetTime = lv_button_create(lv_screen_active());
  btnSetTime->user_data = this;
  lv_obj_set_size(btnSetTime, 120, 50);
  lv_obj_align(btnSetTime, LV_ALIGN_BOTTOM_MID, 0, 0);
  lblSetTime = lv_label_create(btnSetTime);
  lv_label_set_text_static(lblSetTime, "Set");
  lv_obj_set_style_bg_color(btnSetTime, Colors::bgAlt, LV_PART_MAIN);
  lv_obj_set_style_text_color(lblSetTime, PINETIME_COLOR_GRAY, LV_STATE_DISABLED);
  lv_obj_add_event_cb(btnSetTime, SetTimeEventHandler, LV_EVENT_VALUE_CHANGED, this);

  UpdateScreen();
}

SettingSetTime::~SettingSetTime() {
  lv_obj_clean(lv_screen_active());
}

void SettingSetTime::UpdateScreen() {
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    if (lv_roller_get_selected(hourCounter.GetObject()) >= 12) {
      lv_label_set_text_static(lblampm, "PM");
    } else {
      lv_label_set_text_static(lblampm, "AM");
    }
  }
}

void SettingSetTime::SetTime() {
  const int hoursValue = lv_roller_get_selected(hourCounter.GetObject());
  const int minutesValue = lv_roller_get_selected(minuteCounter.GetObject());
  NRF_LOG_INFO("Setting time (manually) to %02d:%02d:00", hoursValue, minutesValue);
  dateTimeController.SetTime(dateTimeController.Year(),
                             static_cast<uint8_t>(dateTimeController.Month()),
                             dateTimeController.Day(),
                             static_cast<uint8_t>(hoursValue),
                             static_cast<uint8_t>(minutesValue),
                             0);
  settingSetDateTime.Quit();
}
