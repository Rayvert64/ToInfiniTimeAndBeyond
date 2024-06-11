#include "displayapp/screens/Weather.h"
#include "displayapp/Colors.h"
#include <lvgl/lvgl.h>
#include "components/ble/SimpleWeatherService.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  lv_color_t TemperatureColor(int16_t temperature) {
    if (temperature <= 0) { // freezing
      return Colors::blue;
    } else if (temperature <= 400) { // ice
      return PINETIME_COLOR_CYAN;
    } else if (temperature >= 2700) { // hot
      return Colors::deepOrange;
    }
    return Colors::orange; // normal
  }

  uint8_t TemperatureStyle(int16_t temperature) {
    if (temperature <= 0) { // freezing
      return LV_TABLE_PART_CELL3;
    } else if (temperature <= 400) { // ice
      return LV_TABLE_PART_CELL4;
    } else if (temperature >= 2700) { // hot
      return LV_TABLE_PART_CELL6;
    }
    return LV_TABLE_PART_CELL5; // normal
  }

  int16_t RoundTemperature(int16_t temp) {
    return temp = temp / 100 + (temp % 100 >= 50 ? 1 : 0);
  }
}

Weather::Weather(Controllers::Settings& settingsController, Controllers::SimpleWeatherService& weatherService)
  : settingsController {settingsController}, weatherService {weatherService} {

  temperature = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(temperature, lv_color_white(), LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(temperature, &jetbrains_mono_42, LV_STATE_DEFAULT);
  lv_label_set_text(temperature, "---");
  lv_obj_align(temperature, LV_ALIGN_CENTER, 0, -30);
  lv_obj_set_auto_realign(temperature, true);

  minTemperature = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(minTemperature, Colors::bg, LV_STATE_DEFAULT);
  lv_label_set_text(minTemperature, "");
  lv_obj_align(minTemperature, LV_ALIGN_OUT_LEFT_MID, -10, 0);
  lv_obj_set_auto_realign(minTemperature, true);

  maxTemperature = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(maxTemperature, Colors::bg, LV_STATE_DEFAULT);
  lv_label_set_text(maxTemperature, "");
  lv_obj_align(maxTemperature, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  lv_obj_set_auto_realign(maxTemperature, true);

  condition = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(condition, Colors::lightGray, LV_STATE_DEFAULT);
  lv_label_set_text(condition, "");
  lv_obj_align(condition, LV_ALIGN_OUT_TOP_MID, 0, -10);
  lv_obj_set_auto_realign(condition, true);

  icon = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(icon, lv_color_white(), LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(icon, &fontawesome_weathericons, LV_STATE_DEFAULT);
  lv_label_set_text(icon, "");
  lv_obj_align(icon, LV_ALIGN_OUT_TOP_MID, 0, 0);
  lv_obj_set_auto_realign(icon, true);

  forecast = lv_table_create(lv_scr_act());
  lv_table_set_col_cnt(forecast, Controllers::SimpleWeatherService::MaxNbForecastDays);
  lv_table_set_row_cnt(forecast, 4);
  // LV_TABLE_PART_CELL1: Default table style
  lv_obj_set_style_border_color(forecast, lv_color_black(), LV_TABLE_PART_CELL1);
  lv_obj_set_style_text_color(forecast, Colors::lightGray, LV_STATE_DEFAULT);
  // LV_TABLE_PART_CELL2: Condition icon
  lv_obj_set_style_border_color(forecast, lv_color_black(), LV_TABLE_PART_CELL2);
  lv_obj_set_style_text_color(forecast, lv_color_white(), LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(forecast, &fontawesome_weathericons, LV_STATE_DEFAULT);
  // LV_TABLE_PART_CELL3: Freezing
  lv_obj_set_style_border_color(forecast, lv_color_black(), LV_TABLE_PART_CELL3);
  lv_obj_set_style_text_color(forecast, Colors::blue, LV_STATE_DEFAULT);
  // LV_TABLE_PART_CELL4: Ice
  lv_obj_set_style_border_color(forecast, lv_color_black(), LV_TABLE_PART_CELL4);
  lv_obj_set_style_text_color(forecast, PINETIME_COLOR_CYAN, LV_STATE_DEFAULT);
  // LV_TABLE_PART_CELL5: Normal
  lv_obj_set_style_border_color(forecast, lv_color_black(), LV_TABLE_PART_CELL5);
  lv_obj_set_style_text_color(forecast, Colors::orange, LV_STATE_DEFAULT);
  // LV_TABLE_PART_CELL6: Hot
  lv_obj_set_style_border_color(forecast, lv_color_black(), LV_TABLE_PART_CELL6);
  lv_obj_set_style_text_color(forecast, Colors::deepOrange, LV_STATE_DEFAULT);

  lv_obj_align(forecast, LV_ALIGN_BOTTOM_LEFT, 0, 0);

  for (int i = 0; i < Controllers::SimpleWeatherService::MaxNbForecastDays; i++) {
    lv_table_set_col_width(forecast, i, 48);
    lv_table_set_cell_type(forecast, 1, i, LV_TABLE_PART_CELL2);
    lv_table_set_cell_align(forecast, 0, i, LV_ALIGN_CENTER);
    lv_table_set_cell_align(forecast, 1, i, LV_ALIGN_CENTER);
    lv_table_set_cell_align(forecast, 2, i, LV_ALIGN_CENTER);
    lv_table_set_cell_align(forecast, 3, i, LV_ALIGN_CENTER);
  }

  taskRefresh = lv_timer_create(RefreshTaskCallback, 1000, this);
  Refresh();
}

Weather::~Weather() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Weather::Refresh() {
  currentWeather = weatherService.Current();
  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get();
    if (optCurrentWeather) {
      int16_t temp = optCurrentWeather->temperature;
      int16_t minTemp = optCurrentWeather->minTemperature;
      int16_t maxTemp = optCurrentWeather->maxTemperature;
      lv_obj_set_style_text_color(temperature, TemperatureColor(temp), LV_STATE_DEFAULT);
      char tempUnit = 'C';
      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        temp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(temp);
        minTemp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(minTemp);
        maxTemp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(maxTemp);
        tempUnit = 'F';
      }
      lv_label_set_text(icon, Symbols::GetSymbol(optCurrentWeather->iconId));
      lv_label_set_text(condition, Symbols::GetCondition(optCurrentWeather->iconId));
      lv_label_set_text_fmt(temperature, "%d°%c", RoundTemperature(temp), tempUnit);
      lv_label_set_text_fmt(minTemperature, "%d°", RoundTemperature(minTemp));
      lv_label_set_text_fmt(maxTemperature, "%d°", RoundTemperature(maxTemp));
    } else {
      lv_label_set_text(icon, "");
      lv_label_set_text(condition, "");
      lv_label_set_text(temperature, "---");
      lv_obj_set_style_text_color(temperature, lv_color_white(), LV_STATE_DEFAULT);
      lv_label_set_text(minTemperature, "");
      lv_label_set_text(maxTemperature, "");
    }
  }

  currentForecast = weatherService.GetForecast();
  if (currentForecast.IsUpdated()) {
    auto optCurrentForecast = currentForecast.Get();
    if (optCurrentForecast) {
      std::tm localTime = *std::localtime(reinterpret_cast<const time_t*>(&optCurrentForecast->timestamp));

      for (int i = 0; i < Controllers::SimpleWeatherService::MaxNbForecastDays; i++) {
        int16_t maxTemp = optCurrentForecast->days[i].maxTemperature;
        int16_t minTemp = optCurrentForecast->days[i].minTemperature;
        lv_table_set_cell_type(forecast, 2, i, TemperatureStyle(maxTemp));
        lv_table_set_cell_type(forecast, 3, i, TemperatureStyle(minTemp));
        if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
          maxTemp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(maxTemp);
          minTemp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(minTemp);
        }
        uint8_t wday = localTime.tm_wday + i + 1;
        if (wday > 7) {
          wday -= 7;
        }
        maxTemp = RoundTemperature(maxTemp);
        minTemp = RoundTemperature(minTemp);
        const char* dayOfWeek = Controllers::DateTime::DayOfWeekShortToStringLow(static_cast<Controllers::DateTime::Days>(wday));
        lv_table_set_cell_value(forecast, 0, i, dayOfWeek);
        lv_table_set_cell_value(forecast, 1, i, Symbols::GetSymbol(optCurrentForecast->days[i].iconId));
        // Pad cells based on the largest number of digits on each column
        char maxPadding[3] = "  ";
        char minPadding[3] = "  ";
        int diff = snprintf(nullptr, 0, "%d", maxTemp) - snprintf(nullptr, 0, "%d", minTemp);
        if (diff <= 0) {
          maxPadding[-diff] = '\0';
          minPadding[0] = '\0';
        } else {
          maxPadding[0] = '\0';
          minPadding[diff] = '\0';
        }
        lv_table_set_cell_value_fmt(forecast, 2, i, "%s%d", maxPadding, maxTemp);
        lv_table_set_cell_value_fmt(forecast, 3, i, "%s%d", minPadding, minTemp);
      }
    } else {
      for (int i = 0; i < Controllers::SimpleWeatherService::MaxNbForecastDays; i++) {
        lv_table_set_cell_value(forecast, 0, i, "");
        lv_table_set_cell_value(forecast, 1, i, "");
        lv_table_set_cell_value(forecast, 2, i, "");
        lv_table_set_cell_value(forecast, 3, i, "");
        lv_table_set_cell_type(forecast, 2, i, LV_TABLE_PART_CELL1);
        lv_table_set_cell_type(forecast, 3, i, LV_TABLE_PART_CELL1);
      }
    }
  }
}
