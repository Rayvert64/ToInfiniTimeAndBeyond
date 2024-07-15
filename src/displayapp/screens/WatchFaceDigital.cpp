#include "displayapp/screens/WatchFaceDigital.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <lvgl/lvgl.h>
#include <cstdio>
#include <lvgl/src/lv_core/lv_disp.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include <lvgl/src/lv_core/lv_obj_style_dec.h>
#include <lvgl/src/lv_misc/lv_area.h>
#include <lvgl/src/lv_misc/lv_color.h>
#include <lvgl/src/lv_widgets/lv_gauge.h>
#include <lvgl/src/lv_widgets/lv_img.h>
#include <sys/_stdint.h>
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/icons/botw/weather/temp_c.c"
#include "displayapp/icons/botw/weather/gauge_needle.c"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

LV_IMG_DECLARE(guage_bg_bar);

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
    weatherSrvc {weatherService},
    statusIcons(batteryController, bleController) {

  statusIcons.Create();

  weatherMeterBackground = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(weatherMeterBackground, &temp_c);
  lv_obj_align(weatherMeterBackground, lv_scr_act(), LV_ALIGN_OUT_BOTTOM_RIGHT, 0, -50);

  weatherMeter = lv_gauge_create(lv_scr_act(), nullptr);
  lv_obj_align(weatherMeter, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 100);
  // lv_obj_clean_style_list(weatherMeter, LV_GAUGE_PART_MAIN);
  // lv_obj_clean_style_list(weatherMeter, LV_GAUGE_PART_MAJOR);
  // lv_obj_clean_style_list(weatherMeter, LV_GAUGE_PART_NEEDLE);
  lv_obj_set_style_local_image_recolor_opa(weatherMeter, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_text_opa(weatherMeter, LV_GAUGE_PART_MAJOR, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_gauge_set_needle_count(weatherMeter, 1, needleColor);
  lv_obj_set_size(weatherMeter, 40, 40);
  lv_gauge_set_needle_img(weatherMeter, &gauge_needle, 3, 4);
  lv_gauge_set_value(weatherMeter, 0, 30u);
  temperature = lv_gauge_get_value(weatherMeter, 0);

  weatherMeterLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(weatherMeterLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_label_set_text_fmt(weatherMeterLabel, "%d", temperature);
  lv_obj_align(weatherMeterLabel, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_CENTER, 0, -15);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x3CD3FC));

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &hylia_serif_42);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, -50);

  label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -30, -85);

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 25, 0);

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, 0, 0);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceDigital::~WatchFaceDigital() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceDigital::Refresh() {
  statusIcons.Update();

  UpdateTempGauge();

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
      lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, -50);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
      lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, -50);
    }

    currentDate = std::chrono::time_point_cast<days>(currentDateTime.Get());
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
      lv_obj_realign(label_date);
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1B1B1B));
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_realign(heartbeatIcon);
    lv_obj_realign(heartbeatValue);
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepValue);
    lv_obj_realign(stepIcon);
  }
}

void WatchFaceDigital::UpdateTempGauge() {
  currentWeather = weatherSrvc.Current();
  int16_t temp = 10;
  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get();
    if (optCurrentWeather) {
      temp = optCurrentWeather->temperature;
      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        temp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(temp);
      }
      temp = temp / 100 + (temp % 100 >= 50 ? 1 : 0);
    }
  }

  // A little wiggle on the guage looks neat
  int16_t hysteresis = (int16_t) (((std::rand() % 5) + 1) * (std::rand() % 2 ? -1 : 1));
  temp += hysteresis;

  temperature = temp;

  lv_gauge_set_value(weatherMeter, 0, temperature);
  temperature = lv_gauge_get_value(weatherMeter, 0);

  lv_obj_align(weatherMeterBackground, lv_scr_act(), LV_ALIGN_OUT_BOTTOM_RIGHT, 0, -50);
  lv_obj_align(weatherMeter, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 100);

  lv_label_set_text_fmt(weatherMeterLabel, "%d", temperature);
  lv_obj_align(weatherMeterLabel, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  lv_obj_realign(weatherMeterBackground);
  lv_obj_realign(weatherMeter);
  lv_obj_realign(weatherMeterLabel);
}