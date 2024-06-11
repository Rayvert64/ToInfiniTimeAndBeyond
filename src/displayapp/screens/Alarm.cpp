/*  Copyright (C) 2021 mruss77, Florian

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <any>
#include "displayapp/screens/Alarm.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/Colors.h"
#include "components/settings/Settings.h"
#include "components/alarm/AlarmController.h"
#include "components/motor/MotorController.h"
#include "systemtask/SystemTask.h"
#include <lvgl/src/core/lv_obj.h>
#include <lvgl/src/misc/lv_event.h>

using namespace Pinetime::Applications::Screens;
using Pinetime::Controllers::AlarmController;

namespace {
  void AlarmHandler(lv_timer_t* timer) {
    auto* screen = static_cast<Alarm*>(timer->user_data);
    screen->OnValueChanged();
  }
}

void ValueChangedHandler(lv_event_t* timer) {
  auto* screen = static_cast<Alarm*>(timer->user_data);
  screen->OnValueChanged();
}

static void btnEventHandlerStop(lv_event_t* event) {
  auto* alarm = static_cast<Alarm*>(event->user_data);
  alarm->StopAlerting();
}

static void btnEventHandlerShowInfo(lv_event_t* event) {
  auto* alarm = static_cast<Alarm*>(event->user_data);
  alarm->ShowInfo();
}

static void btnEventHandlerDisableAlarm(lv_event_t* event) {
  auto* alarm = static_cast<Alarm*>(event->user_data);
  alarm->DisableAlarm();
  alarm->ToggleRecurrence();
}

static void StopAlarmTaskCallback(lv_event_t* task) {
  auto* screen = static_cast<Alarm*>(task->user_data);
  screen->StopAlerting();
}

static void switchValueChanged(lv_event_t* event) {
  auto* alarm = static_cast<Alarm*>(event->user_data);
  if (alarm->GetSwitchState()) {
    alarm->EnableAlarm();
  } else {
    alarm->DisableAlarm();
  }
}

Alarm::Alarm(Controllers::AlarmController& alarmController,
             Controllers::Settings::ClockType clockType,
             System::SystemTask& systemTask,
             Controllers::MotorController& motorController)
  : alarmController {alarmController}, systemTask {systemTask}, motorController {motorController} {

  hourCounter = std::make_shared<lv_obj_t>(lv_roller_create(lv_scr_act()));
  lv_obj_align(hourCounter.get(), LV_ALIGN_TOP_LEFT, 0, 0);
  if (clockType == Controllers::Settings::ClockType::H12) {
    lv_roller_set_options(hourCounter.get(), "01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12", LV_ROLLER_MODE_INFINITE);

    lblampm = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lblampm, &jetbrains_mono_bold_20, LV_STATE_DEFAULT);
    lv_label_set_text_static(lblampm, "AM");
    lv_obj_set_align(lblampm, LV_ALIGN_CENTER);
    lv_obj_align(lblampm, LV_ALIGN_CENTER, 0, 30);
  } else {
    lv_roller_set_options(hourCounter.get(),
                          "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23",
                          LV_ROLLER_MODE_INFINITE);
  }
  lv_roller_set_selected(hourCounter.get(), alarmController.Hours(), LV_ANIM_ON);
  lv_obj_add_event_cb(hourCounter.get(), ValueChangedHandler, LV_EVENT_RELEASED, this);

  minuteCounter = std::make_shared<lv_obj_t>(lv_roller_create(lv_scr_act()));
  lv_obj_align(minuteCounter.get(), LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_roller_set_selected(minuteCounter.get(), alarmController.Minutes(), LV_ANIM_ON);
  lv_obj_add_event_cb(minuteCounter.get(), ValueChangedHandler, LV_EVENT_RELEASED, this);

  lv_obj_t* colonLabel = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(colonLabel, &jetbrains_mono_76, LV_STATE_DEFAULT);
  lv_label_set_text_static(colonLabel, ":");
  lv_obj_align(colonLabel, LV_ALIGN_CENTER, 0, -29);

  btnStop = lv_button_create(lv_scr_act());
  btnStop->user_data = this;
  lv_obj_add_event_cb(btnStop, btnEventHandlerStop, LV_EVENT_SHORT_CLICKED, this);
  lv_obj_set_size(btnStop, 115, 50);
  lv_obj_align(btnStop, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_bg_color(btnStop, PINETIME_COLOR_RED, LV_PART_MAIN);
  txtStop = lv_label_create(btnStop);
  lv_label_set_text_static(txtStop, Symbols::stop);
  lv_obj_add_flag(btnStop, LV_OBJ_FLAG_HIDDEN);

  static constexpr lv_color_t bgColor = Colors::bgAlt;

  btnRecur = lv_button_create(lv_scr_act());
  btnRecur->user_data = this;
  lv_obj_add_event_cb(btnRecur, btnEventHandlerDisableAlarm, LV_EVENT_SHORT_CLICKED, this);
  lv_obj_set_size(btnRecur, 115, 50);
  lv_obj_align(btnRecur, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  txtRecur = lv_label_create(btnRecur);
  SetRecurButtonState();
  lv_obj_set_style_bg_color(btnRecur, bgColor, LV_PART_MAIN);

  btnInfo = lv_button_create(lv_scr_act());
  btnInfo->user_data = this;
  lv_obj_add_event_cb(btnInfo, btnEventHandlerShowInfo, LV_EVENT_SHORT_CLICKED, this);
  lv_obj_set_size(btnInfo, 50, 50);
  lv_obj_align(btnInfo, LV_ALIGN_TOP_MID, 0, -4);
  lv_obj_set_style_bg_color(btnInfo, bgColor, LV_PART_MAIN);
  lv_obj_set_style_border_width(btnInfo, 4, LV_PART_MAIN);
  lv_obj_set_style_border_color(btnInfo, lv_color_black(), LV_PART_MAIN);

  lv_obj_t* txtInfo = lv_label_create(btnInfo);
  lv_label_set_text_static(txtInfo, "i");

  enableSwitch = lv_switch_create(lv_scr_act());
  enableSwitch->user_data = this;
  lv_obj_add_event_cb(enableSwitch, switchValueChanged, LV_EVENT_VALUE_CHANGED, this);
  lv_obj_set_size(enableSwitch, 100, 50);
  // Align to the center of 115px from edge
  lv_obj_align(enableSwitch, LV_ALIGN_BOTTOM_LEFT, 7, 0);
  lv_obj_set_style_bg_color(enableSwitch, bgColor, LV_PART_MAIN);

  UpdateAlarmTime();

  if (alarmController.State() == Controllers::AlarmController::AlarmState::Alerting) {
    SetAlerting();
  } else {
    SetSwitchState(false);
  }
}

Alarm::~Alarm() {
  if (alarmController.State() == AlarmController::AlarmState::Alerting) {
    StopAlerting();
  }
  lv_obj_clean(lv_scr_act());
}

void Alarm::EnableAlarm() {
  if (alarmController.State() == AlarmController::AlarmState::Set) {
    alarmController.ScheduleAlarm();
    if (!lv_obj_has_flag(enableSwitch, LV_STATE_CHECKED)) {
      lv_obj_add_flag(enableSwitch, LV_STATE_CHECKED);
    }
  }
}

void Alarm::DisableAlarm() {
  if (alarmController.State() == AlarmController::AlarmState::Set) {
    alarmController.DisableAlarm();
    if (lv_obj_has_flag(enableSwitch, LV_STATE_CHECKED)) {
      lv_obj_remove_flag(enableSwitch, LV_STATE_CHECKED);
    }
  }
}

bool Alarm::GetSwitchState() const {
  return lv_obj_has_state(enableSwitch, LV_STATE_CHECKED);
}

bool Alarm::OnButtonPushed() {
  if (txtMessage != nullptr && btnMessage != nullptr) {
    HideInfo();
    return true;
  }
  if (alarmController.State() == AlarmController::AlarmState::Alerting) {
    StopAlerting();
    return true;
  }
  return false;
}

bool Alarm::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  // Don't allow closing the screen by swiping while the alarm is alerting
  return alarmController.State() == AlarmController::AlarmState::Alerting && event == TouchEvents::SwipeDown;
}

void Alarm::OnValueChanged() {
  DisableAlarm();
  UpdateAlarmTime();
}

void Alarm::UpdateAlarmTime() {
  if (lblampm != nullptr) {
    if (lv_roller_get_selected(hourCounter.get()) >= 12) {
      lv_label_set_text_static(lblampm, "PM");
    } else {
      lv_label_set_text_static(lblampm, "AM");
    }
  }
  alarmController.SetAlarmTime(lv_roller_get_selected(hourCounter.get()), lv_roller_get_selected(minuteCounter.get()));
}

void Alarm::SetAlerting() {
  lv_obj_add_flag(enableSwitch, LV_OBJ_FLAG_HIDDEN);
  if (lv_obj_has_flag(btnStop, LV_OBJ_FLAG_HIDDEN)) {
    lv_obj_remove_flag(btnStop, LV_OBJ_FLAG_HIDDEN);
  };
  taskStopAlarm = lv_timer_create(AlarmHandler, pdMS_TO_TICKS(60 * 1000), this);
  motorController.StartRinging();
  systemTask.PushMessage(System::Messages::DisableSleeping);
}

void Alarm::StopAlerting() {
  alarmController.StopAlerting();
  motorController.StopRinging();
  SetSwitchState(false);
  if (taskStopAlarm != nullptr) {
    lv_timer_del(taskStopAlarm);
    taskStopAlarm = nullptr;
  }
  systemTask.PushMessage(System::Messages::EnableSleeping);
  if (lv_obj_has_flag(enableSwitch, LV_OBJ_FLAG_HIDDEN)) {
    lv_obj_remove_flag(enableSwitch, LV_OBJ_FLAG_HIDDEN);
  }
  lv_obj_add_flag(btnStop, LV_OBJ_FLAG_HIDDEN);
}

void Alarm::SetSwitchState(bool state) {
  if (state) {
    lv_obj_add_flag(enableSwitch, LV_STATE_CHECKED);
  } else {
    lv_obj_remove_flag(enableSwitch, LV_STATE_CHECKED);
  }
}

void Alarm::ShowInfo() {
  if (btnMessage != nullptr) {
    return;
  }
  btnMessage = lv_button_create(lv_scr_act());
  btnMessage->user_data = this;
  lv_obj_add_event_cb(btnMessage, btnEventHandlerShowInfo, LV_EVENT_SHORT_CLICKED, this);
  lv_obj_set_height(btnMessage, 200);
  lv_obj_set_width(btnMessage, 150);
  lv_obj_align(btnMessage, LV_ALIGN_CENTER, 0, 0);
  txtMessage = lv_label_create(btnMessage);
  lv_obj_set_style_bg_color(btnMessage, PINETIME_COLOR_NAVY, LV_PART_MAIN);

  if (alarmController.State() == AlarmController::AlarmState::Set) {
    auto timeToAlarm = alarmController.SecondsToAlarm();

    auto daysToAlarm = timeToAlarm / 86400;
    auto hrsToAlarm = (timeToAlarm % 86400) / 3600;
    auto minToAlarm = (timeToAlarm % 3600) / 60;
    auto secToAlarm = timeToAlarm % 60;

    lv_label_set_text_fmt(txtMessage,
                          "Time to\nalarm:\n%2lu Days\n%2lu Hours\n%2lu Minutes\n%2lu Seconds",
                          static_cast<unsigned long>(daysToAlarm),
                          static_cast<unsigned long>(hrsToAlarm),
                          static_cast<unsigned long>(minToAlarm),
                          static_cast<unsigned long>(secToAlarm));
  } else {
    lv_label_set_text_static(txtMessage, "Alarm\nis not\nset.");
  }
}

void Alarm::HideInfo() {
  lv_obj_del(btnMessage);
  txtMessage = nullptr;
  btnMessage = nullptr;
}

void Alarm::SetRecurButtonState() {
  using Pinetime::Controllers::AlarmController;
  switch (alarmController.Recurrence()) {
    case AlarmController::RecurType::None:
      lv_label_set_text_static(txtRecur, "ONCE");
      break;
    case AlarmController::RecurType::Daily:
      lv_label_set_text_static(txtRecur, "DAILY");
      break;
    case AlarmController::RecurType::Weekdays:
      lv_label_set_text_static(txtRecur, "MON-FRI");
  }
}

void Alarm::ToggleRecurrence() {
  using Pinetime::Controllers::AlarmController;
  switch (alarmController.Recurrence()) {
    case AlarmController::RecurType::None:
      alarmController.SetRecurrence(AlarmController::RecurType::Daily);
      break;
    case AlarmController::RecurType::Daily:
      alarmController.SetRecurrence(AlarmController::RecurType::Weekdays);
      break;
    case AlarmController::RecurType::Weekdays:
      alarmController.SetRecurrence(AlarmController::RecurType::None);
  }
  SetRecurButtonState();
}
