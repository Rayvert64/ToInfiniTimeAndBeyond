#include "displayapp/screens/WatchFaceAnalog.h"
#include <cmath>
#include <lvgl/lvgl.h>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/NotificationIcon.h"
#include "components/settings/Settings.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t HourLength = 70;
  constexpr int16_t MinuteLength = 90;
  constexpr int16_t SecondLength = 110;

  // sin(90) = 1 so the value of _lv_trigo_sin(90) is the scaling factor
  const auto LV_TRIG_SCALE = lv_trigo_sin(90);

  int16_t Cosine(int16_t angle) {
    return lv_trigo_sin(angle + 90);
  }

  int16_t Sine(int16_t angle) {
    return lv_trigo_sin(angle);
  }

  int16_t CoordinateXRelocate(int16_t x) {
    return (x + LV_HOR_RES / 2);
  }

  int16_t CoordinateYRelocate(int16_t y) {
    return std::abs(y - LV_HOR_RES / 2);
  }

  lv_point_precise_t CoordinateRelocate(int16_t radius, int16_t angle) {
    return lv_point_precise_t {.x = CoordinateXRelocate(radius * static_cast<int32_t>(Sine(angle)) / LV_TRIG_SCALE),
                               .y = CoordinateYRelocate(radius * static_cast<int32_t>(Cosine(angle)) / LV_TRIG_SCALE)};
  }

}

WatchFaceAnalog::WatchFaceAnalog(Controllers::DateTime& dateTimeController,
                                 const Controllers::Battery& batteryController,
                                 const Controllers::Ble& bleController,
                                 Controllers::NotificationManager& notificationManager,
                                 Controllers::Settings& settingsController)
  : currentDateTime {{}},
    batteryIcon(true),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController} {

  sHour = 99;
  sMinute = 99;
  sSecond = 99;

  minor_scales = lv_scale_create(lv_screen_active());
  lv_scale_set_range(minor_scales, 0, 59);
  lv_obj_set_size(minor_scales, 240, 240);
  lv_obj_align(minor_scales, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_opa(minor_scales, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_style_local_scale_width(minor_scales, LV_PART_INDICATOR, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_line_width(minor_scales, LV_PART_INDICATOR, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_scale_end_color(minor_scales, LV_PART_INDICATOR, LV_STATE_DEFAULT, PINETIME_COLOR_GRAY);

  major_scales = lv_scale_create(lv_screen_active());
  lv_scale_set_range(major_scales, 0, 59);
  lv_obj_set_size(major_scales, 240, 240);
  lv_obj_align(major_scales, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_opa(major_scales, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_style_local_scale_width(major_scales, LV_PART_INDICATOR, LV_STATE_DEFAULT, 6);
  lv_obj_set_style_local_scale_end_line_width(major_scales, LV_PART_INDICATOR, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_color(major_scales, LV_PART_INDICATOR, LV_STATE_DEFAULT, lv_color_white());

  large_scales = lv_scale_create(lv_screen_active());
  lv_scale_set_scale(large_scales, 180, 3);
  lv_scale_set_angle_offset(large_scales, 180);
  lv_obj_set_size(large_scales, 240, 240);
  lv_obj_align(large_scales, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_opa(large_scales, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_style_local_scale_width(large_scales, LV_PART_INDICATOR, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_scale_end_line_width(large_scales, LV_PART_INDICATOR, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_color(large_scales, LV_PART_INDICATOR, LV_STATE_DEFAULT, PINETIME_COLOR_CYAN);

  twelve = lv_label_create(lv_screen_active());
  lv_obj_set_align(twelve, LV_ALIGN_CENTER);
  lv_label_set_text_static(twelve, "12");
  lv_obj_set_pos(twelve, 110, 10);
  lv_obj_set_style_text_color(twelve, PINETIME_COLOR_CYAN, LV_STATE_DEFAULT);

  batteryIcon.Create(lv_screen_active());
  lv_obj_align(batteryIcon.GetObject(), LV_ALIGN_TOP_RIGHT, 0, 0);

  plugIcon = lv_label_create(lv_screen_active());
  lv_label_set_text_static(plugIcon, Symbols::plug);
  lv_obj_align(plugIcon, LV_ALIGN_TOP_RIGHT, 0, 0);

  bleIcon = lv_label_create(lv_screen_active());
  lv_label_set_text_static(bleIcon, "");
  lv_obj_align(bleIcon, LV_ALIGN_TOP_RIGHT, -30, 0);

  notificationIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(notificationIcon, PINETIME_COLOR_LIME, LV_STATE_DEFAULT);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, LV_ALIGN_TOP_LEFT, 0, 0);

  // Date - Day / Week day

  label_date_day = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(label_date_day, Colors::orange, LV_STATE_DEFAULT);
  lv_label_set_text_fmt(label_date_day, "%s\n%02i", dateTimeController.DayOfWeekShortToString(), dateTimeController.Day());
  lv_obj_set_align(label_date_day, LV_ALIGN_CENTER);
  lv_obj_align(label_date_day, LV_ALIGN_CENTER, 50, 0);

  minute_body = lv_line_create(lv_screen_active());
  minute_body_trace = lv_line_create(lv_screen_active());
  hour_body = lv_line_create(lv_screen_active());
  hour_body_trace = lv_line_create(lv_screen_active());
  second_body = lv_line_create(lv_screen_active());

  lv_style_init(&second_line_style);
  lv_style_set_line_width(&second_line_style, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&second_line_style, LV_STATE_DEFAULT, PINETIME_COLOR_RED);
  lv_style_set_line_rounded(&second_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(second_body, &second_line_style, LV_LINE_PART_MAIN);

  lv_style_init(&minute_line_style);
  lv_style_set_line_width(&minute_line_style, LV_STATE_DEFAULT, 7);
  lv_style_set_line_color(&minute_line_style, LV_STATE_DEFAULT, lv_color_white());
  lv_style_set_line_rounded(&minute_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(minute_body, &minute_line_style, LV_LINE_PART_MAIN);

  lv_style_init(&minute_line_style_trace);
  lv_style_set_line_width(&minute_line_style_trace, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&minute_line_style_trace, LV_STATE_DEFAULT, lv_color_white());
  lv_style_set_line_rounded(&minute_line_style_trace, LV_STATE_DEFAULT, false);
  lv_obj_add_style(minute_body_trace, &minute_line_style_trace, LV_LINE_PART_MAIN);

  lv_style_init(&hour_line_style);
  lv_style_set_line_width(&hour_line_style, LV_STATE_DEFAULT, 7);
  lv_style_set_line_color(&hour_line_style, LV_STATE_DEFAULT, lv_color_white());
  lv_style_set_line_rounded(&hour_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(hour_body, &hour_line_style, LV_LINE_PART_MAIN);

  lv_style_init(&hour_line_style_trace);
  lv_style_set_line_width(&hour_line_style_trace, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&hour_line_style_trace, LV_STATE_DEFAULT, lv_color_white());
  lv_style_set_line_rounded(&hour_line_style_trace, LV_STATE_DEFAULT, false);
  lv_obj_add_style(hour_body_trace, &hour_line_style_trace, LV_LINE_PART_MAIN);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
  Refresh();
}

WatchFaceAnalog::~WatchFaceAnalog() {
  lv_timer_del(taskRefresh);

  lv_style_reset(&hour_line_style);
  lv_style_reset(&hour_line_style_trace);
  lv_style_reset(&minute_line_style);
  lv_style_reset(&minute_line_style_trace);
  lv_style_reset(&second_line_style);

  lv_obj_clean(lv_screen_active());
}

void WatchFaceAnalog::UpdateClock() {
  uint8_t hour = dateTimeController.Hours();
  uint8_t minute = dateTimeController.Minutes();
  uint8_t second = dateTimeController.Seconds();

  if (sMinute != minute) {
    auto const angle = minute * 6;
    minute_point[0] = CoordinateRelocate(30, angle);
    minute_point[1] = CoordinateRelocate(MinuteLength, angle);

    minute_point_trace[0] = CoordinateRelocate(5, angle);
    minute_point_trace[1] = CoordinateRelocate(31, angle);

    lv_line_set_points(minute_body, minute_point, 2);
    lv_line_set_points(minute_body_trace, minute_point_trace, 2);
  }

  if (sHour != hour || sMinute != minute) {
    sHour = hour;
    sMinute = minute;
    auto const angle = (hour * 30 + minute / 2);

    hour_point[0] = CoordinateRelocate(30, angle);
    hour_point[1] = CoordinateRelocate(HourLength, angle);

    hour_point_trace[0] = CoordinateRelocate(5, angle);
    hour_point_trace[1] = CoordinateRelocate(31, angle);

    lv_line_set_points(hour_body, hour_point, 2);
    lv_line_set_points(hour_body_trace, hour_point_trace, 2);
  }

  if (sSecond != second) {
    sSecond = second;
    auto const angle = second * 6;

    second_point[0] = CoordinateRelocate(-20, angle);
    second_point[1] = CoordinateRelocate(SecondLength, angle);
    lv_line_set_points(second_body, second_point, 2);
  }
}

void WatchFaceAnalog::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  batteryIcon.SetBatteryPercentage(batteryPercent);
}

void WatchFaceAnalog::Refresh() {
  isCharging = batteryController.IsCharging();
  if (isCharging.IsUpdated()) {
    if (isCharging.Get()) {
      lv_obj_add_flag(batteryIcon.GetObject(), LV_OBJ_FLAG_HIDDEN);
      if (lv_obj_has_flag(plugIcon, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_remove_flag(plugIcon, LV_OBJ_FLAG_HIDDEN);
      }
    } else {
      if (lv_obj_has_flag(batteryIcon.GetObject(), LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_remove_flag(batteryIcon.GetObject(), LV_OBJ_FLAG_HIDDEN);
      }
      lv_obj_add_flag(plugIcon, LV_OBJ_FLAG_HIDDEN);
      SetBatteryIcon();
    }
  }
  if (!isCharging.Get()) {
    batteryPercentRemaining = batteryController.PercentRemaining();
    if (batteryPercentRemaining.IsUpdated()) {
      SetBatteryIcon();
    }
  }

  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    if (bleState.Get()) {
      lv_label_set_text_static(bleIcon, Symbols::bluetooth);
    } else {
      lv_label_set_text_static(bleIcon, "");
    }
  }

  notificationState = notificationManager.AreNewNotificationsAvailable();

  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = dateTimeController.CurrentDateTime();
  if (currentDateTime.IsUpdated()) {
    UpdateClock();

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      lv_label_set_text_fmt(label_date_day, "%s\n%02i", dateTimeController.DayOfWeekShortToString(), dateTimeController.Day());
    }
  }
}
