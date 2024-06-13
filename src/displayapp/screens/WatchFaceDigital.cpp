#include "displayapp/screens/WatchFaceDigital.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

WatchFaceDigital::WatchFaceDigital(Controllers::DateTime& dateTimeController,
                                   const Controllers::Battery& batteryController,
                                   const Controllers::Ble& bleController,
                                   Controllers::NotificationManager& notificationManager,
                                   Controllers::Settings& settingsController,
                                   Controllers::HeartRateController& heartRateController,
                                   Controllers::MotionController& motionController,
                                   Controllers::SimpleWeatherService& weatherService)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    weatherService {weatherService},
    statusIcons(batteryController, bleController) {

  statusIcons.Create();

  notificationIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(notificationIcon, PINETIME_COLOR_LIME, LV_STATE_DEFAULT);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, LV_ALIGN_TOP_LEFT, 0, 0);

  weatherIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(weatherIcon, lv_color_hex(0x999999), LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(weatherIcon, &fontawesome_weathericons, LV_STATE_DEFAULT);
  lv_label_set_text(weatherIcon, "");
  lv_obj_align(weatherIcon, LV_ALIGN_TOP_MID, -20, 50);
  lv_obj_set_auto_realign(weatherIcon, true);

  temperature = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(temperature, lv_color_hex(0x999999), LV_STATE_DEFAULT);
  lv_label_set_text(temperature, "");
  lv_obj_align(temperature, LV_ALIGN_TOP_MID, 20, 50);

  label_date = lv_label_create(lv_screen_active());
  lv_obj_align(label_date, LV_ALIGN_CENTER, 0, 60);
  lv_obj_set_style_text_color(label_date, lv_color_hex(0x999999), LV_STATE_DEFAULT);

  label_time = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(label_time, &jetbrains_mono_extrabold_compressed, LV_STATE_DEFAULT);

  lv_obj_align(label_time, LV_ALIGN_RIGHT_MID, 0, 0);

  label_time_ampm = lv_label_create(lv_screen_active());
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, LV_ALIGN_RIGHT_MID, -30, -55);

  heartbeatIcon = lv_label_create(lv_screen_active());
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_text_color(heartbeatIcon, lv_color_hex(0xCE1B1B), LV_STATE_DEFAULT);
  lv_obj_align(heartbeatIcon, LV_ALIGN_BOTTOM_LEFT, 0, 0);

  heartbeatValue = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(heartbeatValue, lv_color_hex(0xCE1B1B), LV_STATE_DEFAULT);
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(stepValue, lv_color_hex(0x00FFE7), LV_STATE_DEFAULT);
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

  stepIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(stepIcon, lv_color_hex(0x00FFE7), LV_STATE_DEFAULT);
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
  Refresh();
}

WatchFaceDigital::~WatchFaceDigital() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_screen_active());
}

void WatchFaceDigital::Refresh() {
  statusIcons.Update();

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());

  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      char ampmChar[3] = "AM";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampmChar[0] = 'P';
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar[0] = 'P';
      }
      lv_label_set_text(label_time_ampm, ampmChar);
      lv_label_set_text_fmt(label_time, "%2d:%02d", hour, minute);
      lv_obj_align(label_time, LV_ALIGN_RIGHT_MID, 0, 0);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
      lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 0);
    }

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      uint8_t day = dateTimeController.Day();
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        lv_label_set_text_fmt(label_date,
                              "%s %d %s %d",
                              dateTimeController.DayOfWeekShortToString(),
                              day,
                              dateTimeController.MonthShortToString(),
                              year);
      } else {
        lv_label_set_text_fmt(label_date,
                              "%s %s %d %d",
                              dateTimeController.DayOfWeekShortToString(),
                              dateTimeController.MonthShortToString(),
                              day,
                              year);
      }
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_text_color(heartbeatIcon, lv_color_hex(0xCE1B1B), LV_STATE_DEFAULT);
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_text_color(heartbeatIcon, lv_color_hex(0x1B1B1B), LV_STATE_DEFAULT);
      lv_label_set_text_static(heartbeatValue, "");
    }
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
  }

  currentWeather = weatherService.Current();
  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get();
    if (optCurrentWeather) {
      int16_t temp = optCurrentWeather->temperature;
      char tempUnit = 'C';
      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        temp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(temp);
        tempUnit = 'F';
      }
      temp = temp / 100 + (temp % 100 >= 50 ? 1 : 0);
      lv_label_set_text_fmt(temperature, "%d°%c", temp, tempUnit);
      lv_label_set_text(weatherIcon, Symbols::GetSymbol(optCurrentWeather->iconId));
    } else {
      lv_label_set_text_static(temperature, "");
      lv_label_set_text(weatherIcon, "");
    }
  }
}
