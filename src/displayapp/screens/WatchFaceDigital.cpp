#include "displayapp/screens/WatchFaceDigital.h"

#include <arduinoFFT/src/types.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <lv_conf.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include <lvgl/src/lv_core/lv_disp.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include <lvgl/src/lv_core/lv_obj_style_dec.h>
#include <lvgl/src/lv_draw/lv_draw_mask.h>
#include <lvgl/src/lv_misc/lv_area.h>
#include <lvgl/src/lv_misc/lv_color.h>
#include <lvgl/src/lv_misc/lv_math.h>
#include <lvgl/src/lv_widgets/lv_gauge.h>
#include <lvgl/src/lv_widgets/lv_img.h>
#include <lvgl/src/lv_widgets/lv_label.h>
#include <lvgl/src/lv_widgets/lv_line.h>
#include <lvgl/src/lv_widgets/lv_objmask.h>
#include <sys/_stdint.h>
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/icons/botw/weather/temp_c.c"
#include "displayapp/icons/botw/weather/gauge_needle.c"
#include "displayapp/icons/botw/weather/triangle.c"
#include "displayapp/icons/botw/hearts/heart_full.c"
#include "displayapp/icons/botw/connectivity/sheika_ble_active.c"
#include "displayapp/icons/botw/connectivity/sheika_ble_inactive.c"
#include "displayapp/icons/botw/connectivity/triforce_notification.c"
#include "displayapp/InfiniTimeTheme.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "components/ble/SimpleWeatherService.h"

using namespace Pinetime::Applications::Screens;

constexpr int32_t LOW_MOVEMENT_MAX = 600;
constexpr int32_t MEDIUM_MOVEMENT_MAX = 1200;
constexpr int32_t HIGH_MOVEMENT_MAX = 2400;

constexpr uint8_t CURVE_BEGINNING_AND_END_PADDING = 5;

static float loweredSin(int16_t val);

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
    bleController {bleController},
    statusIcons(batteryController, bleController) {

  statusIcons.Create();

  InitTemperatureMeter();

  InitWeatherRollerObjects();

  InitMotionMeter();

  // @TODO: Check size of width of image since its a u32
  for (uint16_t i = 0, padding = 0; i < MAX_HEARTS; i++, padding += heart_full.header.w) {
    heart_containers[i] = lv_img_create(lv_scr_act(), nullptr);
    lv_img_set_src(heart_containers[i], &heart_full);
    lv_obj_align(heart_containers[i], lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, padding, 0);
  }

  sheikaSensorBle = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(sheikaSensorBle, (bleState.Get() ? &sheika_ble_active : &sheika_ble_inactive));
  lv_obj_align(sheikaSensorBle, lv_scr_act(), LV_ALIGN_OUT_BOTTOM_RIGHT, 0, -205);

  notificationIcon = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(notificationIcon, &triforce_notification);
  lv_obj_align(notificationIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, -4, 6);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -15);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::ui_font_small_dark);

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &hylia_serif_42);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -50);

  label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -30, -85);

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::heart_red);
  lv_obj_align(heartbeatIcon, notificationIcon, LV_ALIGN_OUT_TOP_LEFT, 15, 0);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::heart_red);
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_value_align(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_TOP_LEFT);
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, heart_containers[MAX_HEARTS - 1], LV_ALIGN_IN_BOTTOM_RIGHT, 0, 10);

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

  UpdateNotificationTriforce();

  UpdateSheikaSensor();

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
      lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -50);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
      lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -50);
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
  }

  UpdateMotionMeter();
  UpdateTempRoller();

  for (uint8_t i = 0, padding = 0; i < MAX_HEARTS; i++, padding += heart_full.header.w) {
    lv_obj_align(heart_containers[i], lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, padding, 0);
    lv_obj_realign(heart_containers[i]);
  }
}

void WatchFaceDigital::UpdateTempGauge() {
  currentWeather = weatherSrvc.Current();
  int16_t temp = 15;
  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get();
    if (optCurrentWeather) {
      temp = optCurrentWeather->temperature;
      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        temp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(temp);
      }
      temp = temp / 100 + (temp % 100 >= 50 ? 1 : 0);
      if (temp * 100 != temperature)
        go_to_temp = true;
      target_temp = temp * 10;
    }
  }
  // Do a x10 to temp to have a more stable needle
  temp *= 10;

  // int32_t angle = 0;
  if (!go_to_temp) {
    // A little wiggle on the guage looks neat
    int16_t hysteresis = (int16_t) (((std::rand() % 5) + 1) * (std::rand() % 2 ? -1 : 1));
    temp = target_temp + hysteresis;
    //go_to_temp = true;
  } else {
    int32_t curr = lv_gauge_get_value(weatherMeter, 0);

    if (curr < target_temp)
      curr++;
    else if (curr > target_temp)
      curr--;

    if (curr == target_temp) {
      go_to_temp = false;
    }
    temp = (int16_t) curr;
  }
  temperature = temp;

  lv_gauge_set_value(weatherMeter, 0, temperature);
  // angle = lv_gauge_get_last_angle(weatherMeter);

  // lv_obj_align(weatherMeterBackground, lv_scr_act(), LV_ALIGN_OUT_BOTTOM_RIGHT, 0, -150);
  // lv_obj_align(weatherMeter, weatherMeterBackground, LV_ALIGN_CENTER, 0, 0);

  // lv_label_set_text_fmt(weatherMeterLabel, "%d", temperature);
  // lv_obj_align(weatherMeterLabel, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  lv_obj_realign(weatherMeterBackground);
  lv_obj_realign(weatherMeter);
  // lv_obj_realign(weatherMeterLabel);
}

void WatchFaceDigital::UpdateTempRoller() {
  forecastWeather = weatherSrvc.GetForecast();
  uint8_t hour = dateTimeController.Hours();
  uint8_t minute = dateTimeController.Minutes();
  // uint8_t second = dateTimeController.Seconds();

  lv_obj_align(weatherRoller, lv_scr_act(), LV_ALIGN_OUT_BOTTOM_RIGHT, 0, -37);

  auto optCurrentWeather = currentWeather.Get();
  if (optCurrentWeather.has_value()) {
    lv_label_set_text(weatherRollerTextSelected, Symbols::GetSymbol(optCurrentWeather->iconId));
  }

  auto optForecastWeather = forecastWeather.Get();
  if (optForecastWeather.has_value()) {
    lv_label_set_text(weatherRollerTextNext1, Symbols::GetSymbol(optForecastWeather->days[1].iconId));
    lv_label_set_text(weatherRollerTextNext2, Symbols::GetSymbol(optForecastWeather->days[2].iconId));
  }

  lv_obj_align(weatherRollerTextSelected, weatherRoller, LV_ALIGN_CENTER, -45 + (12 - hour), 0);
  lv_obj_align(weatherRollerTextNext1, weatherRoller, LV_ALIGN_CENTER, 0 + (12 - hour), 0);
  lv_obj_align(weatherRollerTextNext2, weatherRoller, LV_ALIGN_CENTER, 45 + (12 - hour), 0);
  lv_obj_set_style_local_text_opa(weatherRollerTextNext2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER - ((24 - hour) * 6));

  char ampmChar[3] = "AM";
  if (hour == 0) {
    hour = 12;
  } else if (hour == 12) {
    ampmChar[0] = 'P';
  } else if (hour > 12) {
    hour = hour - 12;
    ampmChar[0] = 'P';
  }

  // lv_label_set_text(weatherRollerTime, ampmChar);
  lv_label_set_text_fmt(weatherRollerTime, "%2d:%02d %c%c", hour, minute, ampmChar[0], ampmChar[1]);
  lv_obj_align(weatherRollerTime, weatherRoller, LV_ALIGN_CENTER, -27, -33);

  lv_obj_realign(weatherRoller);
  lv_obj_realign(weatherRollerTextSelected);
  lv_obj_realign(weatherRollerTextNext1);
  lv_obj_realign(weatherRollerTextNext2);
  lv_obj_realign(weatherRollerTime);
}

void WatchFaceDigital::InitWeatherRollerObjects() {
  weatherRoller = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(weatherRoller, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_bg_opa(weatherRoller, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 175);
  lv_obj_set_size(weatherRoller, 150, 35);
  lv_obj_set_style_local_radius(weatherRoller, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 35);
  lv_obj_align(weatherRoller, lv_scr_act(), LV_ALIGN_OUT_BOTTOM_RIGHT, 0, -37);

  weatherRollerTriangle = lv_img_create(weatherRoller, nullptr);
  lv_img_set_src(weatherRollerTriangle, &triangle);
  lv_obj_align(weatherRollerTriangle, weatherRoller, LV_ALIGN_IN_TOP_LEFT, 15, -8);

  weatherRollerTextSelected = lv_label_create(weatherRoller, nullptr);
  lv_obj_align(weatherRollerTextSelected, weatherRoller, LV_ALIGN_CENTER, -45, 0);
  lv_obj_set_style_local_text_color(weatherRollerTextSelected, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::ui_sheika_blue_light);
  lv_obj_set_style_local_text_font(weatherRollerTextSelected, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_label_set_text(weatherRollerTextSelected, Symbols::ban);

  weatherRollerTextNext1 = lv_label_create(weatherRoller, nullptr);
  lv_obj_align(weatherRollerTextNext1, weatherRoller, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_text_color(weatherRollerTextNext1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::ui_sheika_blue_dark);
  lv_obj_set_style_local_text_font(weatherRollerTextNext1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_obj_set_style_local_text_opa(weatherRollerTextNext1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_label_set_text(weatherRollerTextNext1, Symbols::cloud);

  weatherRollerTextNext2 = lv_label_create(weatherRoller, nullptr);
  lv_obj_align(weatherRollerTextNext2, weatherRoller, LV_ALIGN_CENTER, 45, 0);
  lv_obj_set_style_local_text_color(weatherRollerTextNext2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::ui_sheika_blue_dark);
  lv_obj_set_style_local_text_font(weatherRollerTextNext2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_obj_set_style_local_text_opa(weatherRollerTextNext2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_text_opa(weatherRollerTextNext2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_label_set_text(weatherRollerTextNext2, Symbols::cloudSunRain);

  weatherRollerTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(weatherRollerTime, weatherRoller, LV_ALIGN_CENTER, -27, -33);
  lv_obj_set_style_local_text_font(weatherRollerTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &roboto_italics_20);
  lv_obj_set_style_local_text_color(weatherRollerTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::ui_sheika_blue_light);
}

void WatchFaceDigital::InitTemperatureMeter() {
  weatherMeterBackground = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(weatherMeterBackground, &temp_c);
  lv_obj_align(weatherMeterBackground, lv_scr_act(), LV_ALIGN_OUT_BOTTOM_RIGHT, 0, -150);

  target_temp = 150;

  weatherMeter = lv_gauge_create(lv_scr_act(), nullptr);
  lv_obj_align(weatherMeter, weatherMeterBackground, LV_ALIGN_CENTER, 0, 0);
  lv_obj_clean_style_list(weatherMeter, LV_GAUGE_PART_MAIN);
  lv_obj_clean_style_list(weatherMeter, LV_GAUGE_PART_MAJOR);
  lv_obj_set_style_local_image_recolor_opa(weatherMeter, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_text_opa(weatherMeter, LV_GAUGE_PART_MAJOR, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_gauge_set_needle_count(weatherMeter, 1, needleColor);
  lv_obj_set_size(weatherMeter, 50, 50);
  lv_gauge_set_range(weatherMeter, -100, 350);
  lv_gauge_set_needle_img(weatherMeter, &gauge_needle, 3, 4);
  lv_gauge_set_value(weatherMeter, 0, 0u);
  temperature = lv_gauge_get_value(weatherMeter, 0);
}

void WatchFaceDigital::InitMotionMeter() {
  motionMeterBackgrnd = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(motionMeterBackgrnd, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_bg_opa(motionMeterBackgrnd, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 175);
  lv_obj_set_size(motionMeterBackgrnd, 50, 50);
  lv_obj_set_style_local_radius(motionMeterBackgrnd, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 25);
  lv_obj_align(motionMeterBackgrnd, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, -45);

  // motionMeterTestLabel = lv_label_create(lv_scr_act(), nullptr);
  // lv_obj_align(motionMeterTestLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  // lv_obj_set_style_local_text_font(motionMeterTestLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &roboto_italics_20);
  // lv_label_set_text(motionMeterTestLabel, "fq");

  for (uint8_t i = 0; i < NUM_POINTS_MOTION_METER; i++) {
    motionMeterPoints[i] = {i, 25};
  }

  motionMeterLine = lv_line_create(motionMeterBackgrnd, nullptr);
  lv_line_set_points(motionMeterLine, motionMeterPoints, NUM_POINTS_MOTION_METER);
  lv_obj_align(motionMeterLine, motionMeterBackgrnd, LV_ALIGN_IN_TOP_MID, 0, 0);
  lv_obj_set_style_local_line_color(motionMeterLine, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Colors::ui_sheika_purple);
  lv_obj_set_style_local_line_width(motionMeterLine, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
}

void WatchFaceDigital::UpdateMotionMeter() {
  static uint8_t counter = 0;
  static int32_t motion = 0;
  // static int32_t maxMotion = 0;
  counter++;

  if (counter % 3 != 0) {
    for (int16_t i = CURVE_BEGINNING_AND_END_PADDING; i < (NUM_POINTS_MOTION_METER - CURVE_BEGINNING_AND_END_PADDING); i++) {
      if (motionMeterPoints[i].y > motionMeterPointsTargets[i]) {
        motionMeterPoints[i].y--;
      } else if (motionMeterPoints[i].y < motionMeterPointsTargets[i]) {
        motionMeterPoints[i].y++;
      }
    }
  } else {

    int32_t motionLive = GetMotionLevel();
    uint32_t motionDelta = std::abs(motion - motionLive);

    if (motionDelta > MEDIUM_MOVEMENT_MAX) {
      // Even more complicated
      ApplyAgressiveMotionToLine(motionDelta);
    } else if (motionDelta > LOW_MOVEMENT_MAX) {
      // A more complicated sum of two sins
      ApplyComplexMotionToLine(motionDelta);
    } else {
      // A simple large sin wave should be enough
      ApplyAgressiveMotionToLine(motionDelta);
    }

    // lv_label_set_text_fmt(motionMeterTestLabel, "%04d\nMax: %04d", motion - motionLive, maxMotion);

    // Save last motion
    motion = motionLive;
  }

  lv_line_set_points(motionMeterLine, motionMeterPoints, NUM_POINTS_MOTION_METER);
  lv_obj_realign(motionMeterLine);
  // lv_obj_realign(motionMeterTestLabel);
}

// When there is not a lot of movement, a simple large sin function
// should be enough to convey that we are sneaky
// @NOTE: UNUSED, as since we can not draw actual curves, a large wave looks bad when we
//        draw it using our technique. (A collection of 50 strait lines)
[[maybe_unused]] void WatchFaceDigital::ApplySimpleMotionToLine(uint32_t motionDiff) {
  static uint8_t curveMotion = 0; // This will give a nice feeling of lateral motion to the curve
  curveMotion += 2;
  for (int16_t i = CURVE_BEGINNING_AND_END_PADDING; i < (NUM_POINTS_MOTION_METER - CURVE_BEGINNING_AND_END_PADDING); i++) {
    motionMeterPointsTargets[i] = (lv_coord_t) ((motionDiff >> 1) * loweredSin((i + curveMotion) * 28) + 25);
    if (motionMeterPoints[i].y > motionMeterPointsTargets[i]) {
      motionMeterPoints[i].y--;
    } else if (motionMeterPoints[i].y < motionMeterPointsTargets[i]) {
      motionMeterPoints[i].y++;
    }
  }
}

// When there is a moderate amount of movement we put a more intense sin function
// and simply multiply the value by a sin function that only goes up to pie.
// This will make a nice "gaussian" sin wave.
void WatchFaceDigital::ApplyComplexMotionToLine(uint32_t motionDiff) {
  static uint8_t curveMotion = 0; // This will give a nice feeling of lateral motion to the curve
  curveMotion++;
  for (int16_t i = CURVE_BEGINNING_AND_END_PADDING; i < (NUM_POINTS_MOTION_METER - CURVE_BEGINNING_AND_END_PADDING); i++) {
    motionMeterPointsTargets[i] = (lv_coord_t) (((motionDiff >> 2) * loweredSin(i * 3)) * loweredSin((i + curveMotion) * 56)) + 25;
    if (motionMeterPoints[i].y > motionMeterPointsTargets[i]) {
      motionMeterPoints[i].y--;
    } else if (motionMeterPoints[i].y < motionMeterPointsTargets[i]) {
      motionMeterPoints[i].y++;
    }
  }
}

// When there is a lot of movement. Here we want many peaks that seem dissorganized
// so we add more peaks and multiply by the absolute value of a sin that goes up to 2pie.
// This is not too much more complicated calculation wise, but to a puny human this seems nuts!
void WatchFaceDigital::ApplyAgressiveMotionToLine(uint32_t motionDiff) {
  static uint8_t curveMotion = 0; // This will give a nice feeling of lateral motion to the curve
  curveMotion++;
  for (int16_t i = CURVE_BEGINNING_AND_END_PADDING; i < (NUM_POINTS_MOTION_METER - CURVE_BEGINNING_AND_END_PADDING); i++) {
    motionMeterPointsTargets[i] =
      (lv_coord_t) (((motionDiff >> 3) * std::abs(loweredSin(i * 7.5))) * loweredSin((i + curveMotion) * 56)) + 25;
    if (motionMeterPoints[i].y > motionMeterPointsTargets[i]) {
      motionMeterPoints[i].y--;
    } else if (motionMeterPoints[i].y < motionMeterPointsTargets[i]) {
      motionMeterPoints[i].y++;
    }
  }
}

int32_t WatchFaceDigital::GetMotionLevel() {
  // Get current motion
  int16_t xLive = motionController.X();
  int16_t yLive = motionController.Y();
  int16_t zLive = motionController.Z();

  return xLive + yLive + zLive;
}

// This returns a value between -1 and 1
static float loweredSin(int16_t val) {
  // Value between -1 and 1
  return ((float) _lv_trigo_sin(val) / (float) INT16_MAX);
}

void WatchFaceDigital::UpdateNotificationTriforce() {
  static int16_t throbber = 0;
  static bool animDir = true;
  notificationState = notificationManager.AreNewNotificationsAvailable();
  if ((bool) notificationManager.NbNotifications()) {
    //    This should do a throbbing effect- I mean PULSATING... pulsating... is... what...I......meant...
    if (animDir) {
      throbber += 5;
      if (throbber == MAX_U08) {
        animDir = false;
      }
    } else {
      throbber -= 5;
      if (throbber == 0) {
        animDir = true;
      }
    }
    if (throbber % 15 == 0) {
      lv_obj_set_style_local_image_opa(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, throbber);
      lv_obj_invalidate(notificationIcon);
    }
  } else {
    lv_obj_set_style_local_image_opa(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  }
  lv_obj_realign(notificationIcon);
}

void WatchFaceDigital::UpdateSheikaSensor() {
  lv_img_set_src(sheikaSensorBle, (bleController.IsConnected() ? &sheika_ble_active : &sheika_ble_inactive));
  lv_obj_invalidate(sheikaSensorBle);
  lv_obj_realign(sheikaSensorBle);
}